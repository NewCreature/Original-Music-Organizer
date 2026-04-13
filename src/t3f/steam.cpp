#include "t3f.h"
#ifdef T3F_ENABLE_STEAM_INTEGRATION
  #include "steam/steam_api_flat.h"
#endif

#ifdef T3F_ENABLE_STEAM_INTEGRATION
  static bool _t3f_steam_integration_enabled = false;
  static bool _t3f_steam_stats_ready = false;
  static bool _t3f_steam_want_stats_reset = false;
  static double _t3f_steam_store_time_interval = 7.0;
  static double _t3f_steam_store_time = -_t3f_steam_store_time_interval;
  static T3F_ACHIEVEMENTS_LIST * _t3f_achievements_list = NULL;
  static bool _t3f_steam_overlay_active = false;
#endif

bool t3f_init_steam_integration(T3F_ACHIEVEMENTS_LIST * achievements_list)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    const char * val;

    if(!SteamAPI_Init())
    {
      goto fail;
    }
    SteamAPI_ManualDispatch_Init();
    _t3f_achievements_list = achievements_list;
    _t3f_steam_integration_enabled = true;
    SteamAPI_ISteamUserStats_RequestUserStats(SteamUserStats(), SteamAPI_ISteamUser_GetSteamID(SteamAPI_SteamUser()));
    val = al_get_config_value(t3f_config, "Setting", "Steam Notification Interval");
    if(val)
    {
      _t3f_steam_store_time_interval = atof(val);
    }
    return true;

    fail:
    {
      t3f_shutdown_steam_integration();
      return false;
    }
  #else
    return false;
  #endif
}

void t3f_shutdown_steam_integration(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_Shutdown();
      _t3f_steam_integration_enabled = false;
    }
  #endif
}

bool t3f_steam_integration_enabled(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    return _t3f_steam_integration_enabled;
  #endif
  return false;
}

bool t3f_restart_through_steam(uint32_t app_id)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(al_filename_exists("data/steam.dat"))
    {
      return SteamAPI_RestartAppIfNecessary(app_id);
    }
  #endif
  return false;
}

bool t3f_steam_deck_mode(void)
{
  const char * val;

  /* detect user config */
  val = al_get_config_value(t3f_config, "T3F", "force_deck");
  if(val)
  {
    if(!strcasecmp(val, "false"))
    {
      return false;
    }
    else if(strcasecmp(val, "true"))
    {
      return true;
    }
  }

  /* check through API if no user config */
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(SteamUtils()))
    {
      if(SteamAPI_ISteamUtils_IsSteamInBigPictureMode(SteamUtils()))
      {
        return true;
      }
    }
  #endif

  return false;
}

bool t3f_show_steam_text_input(int x, int y, int width, int height)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput(SteamUtils(), k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
    }
  #endif
  return true;
}

const char * t3f_get_steam_user_display_name(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      return SteamAPI_ISteamFriends_GetPersonaName(SteamFriends());
    }
  #endif
  return NULL;
}

#ifdef T3F_ENABLE_STEAM_INTEGRATION

  static void _t3f_run_steam_callbacks(void)
  {
    HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe();
    SteamAPI_ManualDispatch_RunFrame(hSteamPipe);
    CallbackMsg_t callback;
    while(SteamAPI_ManualDispatch_GetNextCallback(hSteamPipe, &callback))
    {
      // Check for dispatching API call results
      if(callback.m_iCallback == SteamAPICallCompleted_t::k_iCallback)
      {
        SteamAPICallCompleted_t *pCallCompleted = (SteamAPICallCompleted_t *)&callback;
        void * pTmpCallResult = malloc(pCallCompleted->m_cubParam);
        bool bFailed;
        if ( SteamAPI_ManualDispatch_GetAPICallResult( hSteamPipe, pCallCompleted->m_hAsyncCall, pTmpCallResult, pCallCompleted->m_cubParam, pCallCompleted->m_iCallback, &bFailed ) )
        {
          // Dispatch the call result to the registered handler(s) for the
          // call identified by pCallCompleted->m_hAsyncCall
        }
        free(pTmpCallResult);
      }
      else
      {
        // Look at callback.m_iCallback to see what kind of callback it is,
        // and dispatch to appropriate handler(s)
        if(callback.m_iCallback == UserStatsReceived_t::k_iCallback)
        {
          _t3f_steam_stats_ready = true;
        }
        else if(callback.m_iCallback == UserStatsStored_t::k_iCallback)
        {
          if(_t3f_achievements_list)
          {
            if(_t3f_achievements_list->store_entry >= 0)
            {
              _t3f_achievements_list->entry[_t3f_achievements_list->store_entry].store_state = T3F_ACHIEVEMENTS_STATE_STORED;
              _t3f_steam_store_time = al_get_time();
            }
          }
        }
        else if(callback.m_iCallback == GameOverlayActivated_t::k_iCallback)
        {
          uint8_t * overlay_state = (uint8_t *)callback.m_pubParam;
          _t3f_steam_overlay_active = *overlay_state;
        }
      }
      SteamAPI_ManualDispatch_FreeLastCallback( hSteamPipe );
    }
  }

  static bool _t3f_synchronize_achievements_with_steam(T3F_ACHIEVEMENTS_LIST * achievements_list)
  {
    int i;

    if(_t3f_steam_integration_enabled && _t3f_achievements_list && _t3f_achievements_list->updated && _t3f_steam_stats_ready && al_get_time() - _t3f_steam_store_time >= _t3f_steam_store_time_interval)
    {
      for(i = 0; i < achievements_list->entries; i++)
      {
        if(achievements_list->entry[i].step >= achievements_list->entry[i].steps && achievements_list->entry[i].store_state != T3F_ACHIEVEMENTS_STATE_STORED)
        {
          SteamAPI_ISteamUserStats_SetAchievement(SteamUserStats(), achievements_list->entry[i].steam_id);
          achievements_list->entry[i].store_state = T3F_ACHIEVEMENTS_STATE_STORING;
          achievements_list->store_entry = i;
          break;
        }
      }
      if(i >= achievements_list->entries)
      {
        _t3f_achievements_list->updated = false;
      }
      else
      {
        SteamAPI_ISteamUserStats_StoreStats(SteamUserStats());
        _t3f_steam_store_time = al_get_time();
      }
      return true;
    }
    return false;
  }

  static bool _t3f_really_reset_steam_stats(void)
  {
    if(_t3f_steam_integration_enabled && _t3f_steam_stats_ready)
    {
      return SteamAPI_ISteamUserStats_ResetAllStats(SteamUserStats(), true);
    }
    return false;
  }

#endif

void t3f_reset_steam_stats(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    _t3f_steam_want_stats_reset = true;
  #endif
}

bool t3f_steam_overlay_active(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    return _t3f_steam_overlay_active;
  #else
    return false;
  #endif
}

void t3f_steam_integration_logic(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(_t3f_steam_integration_enabled)
    {
      if(_t3f_steam_want_stats_reset)
      {
        if(_t3f_really_reset_steam_stats())
        {
          _t3f_steam_want_stats_reset = false;
        }
      }
      _t3f_synchronize_achievements_with_steam(_t3f_achievements_list);
      _t3f_run_steam_callbacks();
    }
  #endif
}
