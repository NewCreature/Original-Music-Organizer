#ifndef OMO_UI_MENU_PROC_H
#define OMO_UI_MENU_PROC_H

#include "../t3f/t3f.h"

int omo_menu_file_play_files(void * data);
int omo_menu_file_queue_files(void * data);
int omo_menu_file_play_folder(void * data);
int omo_menu_file_queue_folder(void * data);
int omo_menu_file_get_tagger_key(void * data);
int omo_menu_file_exit(void * data);

int omo_menu_playback_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_playback_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_playback_previous_track(void * data);
int omo_menu_playback_play(void * data);
int omo_menu_playback_pause(void * data);
int omo_menu_playback_stop(void * data);
int omo_menu_playback_next_track(void * data);
int omo_menu_playback_shuffle(void * data);
int omo_menu_playback_edit_tags(void * data);
int omo_menu_playback_split_track(void * data);

int omo_menu_library_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_library_add_folder(void * data);
int omo_menu_library_clear_folders(void * data);
int omo_menu_library_edit_tags(void * data);
int omo_menu_library_split_track(void * data);

int omo_menu_view_basic_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_view_library_update_proc(ALLEGRO_MENU * mp, int item, void * data);
int omo_menu_view_basic(void * data);
int omo_menu_view_library(void * data);

#endif
