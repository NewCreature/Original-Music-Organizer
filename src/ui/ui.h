#ifndef OMO_UI_H
#define OMO_UI_H

#include "theme.h"

#define OMO_UI_MAX_TAGS         16
#define OMO_UI_MAX_TAG_LENGTH 1024

typedef struct
{
	/* main dialog */
	T3GUI_DIALOG * ui_dialog;
	T3GUI_ELEMENT * ui_queue_list_box_element;
	T3GUI_ELEMENT * ui_queue_list_element;
	T3GUI_ELEMENT * ui_button_element[6];
	T3GUI_ELEMENT * ui_artist_list_element;
	T3GUI_ELEMENT * ui_album_list_element;
	T3GUI_ELEMENT * ui_song_list_element;
	int selected_song;
	int mode;

	/* tags dialog */
	ALLEGRO_DISPLAY * tags_display;
	T3GUI_DIALOG * tags_dialog;
	char original_tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	char tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	const char * tags_entry;
	T3GUI_ELEMENT * tags_ok_button_element;
	int tags_queue_entry;

	/* theme data */
	OMO_THEME * main_theme;
	OMO_THEME * popup_theme;
	char ui_button_text[6][8];

} OMO_UI;

OMO_UI * omo_create_ui(void);
void omo_destroy_ui(OMO_UI * uip);
void omo_resize_ui(OMO_UI * uip, int mode, int width, int height);
bool omo_create_main_dialog(OMO_UI * uip, int mode, int width, int height, void * data);

bool omo_open_tags_dialog(OMO_UI * uip, void * data);
void omo_close_tags_dialog(OMO_UI * uip, void * data);

#endif
