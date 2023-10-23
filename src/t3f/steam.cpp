#include "t3f.h"
#ifdef T3F_ENABLE_STEAM_INTEGRATION
  #include "steam/steam_api_flat.h"
#endif

#define T3F_STEAM_STORE_STATE_NONE        0
#define T3F_STEAM_STORE_STATE_IN_PROGRESS 1
#define T3F_STEAM_STORE_STATE_DONE        2
#define T3F_STEAM_STORE_STATE_ERROR       3

#define T3F_STEAM_STATS_STORE_INTERVAL 10.0

#ifdef T3F_ENABLE_STEAM_INTEGRATION
  static bool _t3f_steam_integration_enabled = false;
  static bool _t3f_steam_stats_ready = false;
  static bool _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_NONE;
  static double _t3f_steam_store_time = -T3F_STEAM_STATS_STORE_INTERVAL;
  static T3F_ACHIEVEMENTS_LIST * _t3f_achievements_list = NULL;
#endif

bool t3f_init_steam_integration(T3F_ACHIEVEMENTS_LIST * achievements_list)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    if(!SteamAPI_Init())
    {
      goto fail;
    }
    SteamAPI_ManualDispatch_Init();
    _t3f_achievements_list = achievements_list;
    _t3f_steam_integration_enabled = true;
    SteamAPI_ISteamUserStats_RequestCurrentStats(SteamUserStats());
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
          _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_DONE;
        }
      }
      SteamAPI_ManualDispatch_FreeLastCallback( hSteamPipe );
    }
  }

  static bool _t3f_synchronize_achievements_with_steam(T3F_ACHIEVEMENTS_LIST * achievements_list)
  {
    int i;

    if(_t3f_steam_integration_enabled && _t3f_achievements_list && _t3f_achievements_list->modified && _t3f_steam_stats_ready)
    {
      for(i = 0; i < achievements_list->entries; i++)
      {
        if(achievements_list->entry[i].step >= achievements_list->entry[i].steps)
        {
          SteamAPI_ISteamUserStats_SetAchievement(SteamUserStats(), achievements_list->entry[i].steam_id);
        }
      }
      SteamAPI_ISteamUserStats_StoreStats(SteamUserStats());
      _t3f_achievements_list->modified = false;
      _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_IN_PROGRESS;
      al_set_config_value(t3f_user_data, "Achievements", "Stored", "false");
      return true;
    }
    return false;
  }

#endif


void t3f_steam_integration_logic(void)
{
  #ifdef T3F_ENABLE_STEAM_INTEGRATION
    const char * val;

    if(_t3f_steam_integration_enabled)
    {
      _t3f_synchronize_achievements_with_steam(_t3f_achievements_list);
      _t3f_run_steam_callbacks();

      /* attempt to store current achievements if it is needed and store isn't currently processing */
      if(_t3f_steam_stats_ready && _t3f_steam_stats_store_state != T3F_STEAM_STORE_STATE_IN_PROGRESS)
      {
        val = al_get_config_value(t3f_user_data, "Achievements", "Stored");
        if(val && !strcmp(val, "false"))
        {
          if(al_get_time() - _t3f_steam_store_time >= T3F_STEAM_STATS_STORE_INTERVAL)
          {
            SteamAPI_ISteamUserStats_StoreStats(SteamUserStats());
            _t3f_steam_store_time = al_get_time();
            _t3f_steam_stats_store_state = T3F_STEAM_STORE_STATE_IN_PROGRESS;
          }
        }
      }
    }
  #endif
}
