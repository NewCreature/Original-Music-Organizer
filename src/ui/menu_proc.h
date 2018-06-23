#ifndef OMO_UI_MENU_PROC_H
#define OMO_UI_MENU_PROC_H

#include "../t3f/t3f.h"

int omo_menu_file_play_files(int id, void * data);
int omo_menu_file_queue_files(int id, void * data);
int omo_menu_file_play_folder(int id, void * data);
int omo_menu_file_queue_folder(int id, void * data);
int omo_menu_file_save_playlist(int id, void * data);
int omo_menu_file_get_tagger_key(int id, void * data);
int omo_menu_file_load_theme(int id, void * data);
int omo_menu_file_exit(int id, void * data);

int omo_menu_edit_copy_tags(int id, void * data);
int omo_menu_edit_paste_tags(int id, void * data);

int omo_menu_playback_previous_track(int id, void * data);
int omo_menu_playback_play(int id, void * data);
int omo_menu_playback_pause(int id, void * data);
int omo_menu_playback_stop(int id, void * data);
int omo_menu_playback_next_track(int id, void * data);
int omo_menu_playback_shuffle(int id, void * data);
int omo_menu_playback_edit_tags(int id, void * data);
int omo_menu_playback_split_track(int id, void * data);
int omo_menu_playback_find_track(int id, void * data);

int omo_menu_library_select_profile(int id, void * data);
int omo_menu_library_add_profile(int id, void * data);
int omo_menu_library_remove_profile(int id, void * data);
int omo_menu_library_add_folder(int id, void * data);
int omo_menu_library_clear_folders(int id, void * data);
int omo_menu_library_rescan_folders(int id, void * data);
int omo_menu_library_edit_filter(int id, void * data);
int omo_menu_library_edit_tags(int id, void * data);
int omo_menu_library_split_track(int id, void * data);
int omo_menu_library_submit_tags(int id, void * data);
int omo_menu_library_retrieve_tags(int id, void * data);

int omo_menu_view_basic(int id, void * data);
int omo_menu_view_library(int id, void * data);

int omo_menu_help_about(int id, void * data);

#endif
