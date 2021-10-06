#ifndef OMO_UI_DIALOG_PROC_H
#define OMO_UI_DIALOG_PROC_H

char * ui_queue_list_proc(int index, int *list_size, bool * multi, void * data);
char * ui_artist_list_proc(int index, int *list_size, bool * multi, void * data);
char * ui_album_list_proc(int index, int *list_size, bool * multi, void * data);
char * ui_song_list_proc(int index, int *list_size, bool * multi, void * data);
int ui_player_button_proc(T3GUI_ELEMENT * ep, void * dp3);
int ui_tags_button_proc(T3GUI_ELEMENT * ep, void * dp3);

#endif
