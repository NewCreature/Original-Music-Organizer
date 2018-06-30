#ifndef OMO_MENU_UPDATE_PROC_H
#define OMO_MENU_UPDATE_PROC_H

int omo_menu_base_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_playlist_update_proc(ALLEGRO_MENU * mp, int item, void * data);

int omo_menu_playback_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_playback_find_track_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_playback_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data);

int omo_menu_library_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_library_profile_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_library_profile_delete_update_proc(ALLEGRO_MENU * mp, int item, void * data);

int omo_menu_cloud_update_proc(ALLEGRO_MENU * mp, int item, void * data);

int omo_menu_view_basic_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_view_library_update_proc(ALLEGRO_MENU * mp, int item, void * data);

#endif
