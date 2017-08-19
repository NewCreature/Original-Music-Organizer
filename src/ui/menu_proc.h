#ifndef OMO_UI_MENU_PROC_H
#define OMO_UI_MENU_PROC_H

#include "../t3f/t3f.h"

int omo_menu_file_play_files(void * data);
int omo_menu_file_queue_files(void * data);
int omo_menu_file_play_folder(void * data);
int omo_menu_file_queue_folder(void * data);
int omo_menu_file_add_library_folder(void * data);
int omo_menu_file_clear_library_folders(void * data);
int omo_menu_file_exit(void * data);
int omo_menu_playback_play(void * data);
int omo_menu_playback_pause(void * data);
int omo_menu_playback_shuffle(void * data);

#endif
