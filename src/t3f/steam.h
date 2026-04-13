#ifndef T3F_STEAM_H
#define T3F_STEAM_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "achievements.h"

bool t3f_init_steam_integration(T3F_ACHIEVEMENTS_LIST * achievements_list);
void t3f_shutdown_steam_integration(void);
bool t3f_steam_integration_enabled(void);
bool t3f_restart_through_steam(uint32_t app_id);
bool t3f_steam_deck_mode(void);
bool t3f_show_steam_text_input(int x, int y, int width, int height);
void t3f_reset_steam_stats(void);
const char * t3f_get_steam_user_display_name(void);
bool t3f_steam_overlay_active(void);
void t3f_steam_integration_logic(void);

#ifdef __cplusplus
   }
#endif

#endif