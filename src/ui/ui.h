#ifndef OMO_UI_H
#define OMO_UI_H

#include "theme.h"

#define OMO_UI_MAX_TAGS           16
#define OMO_UI_MAX_TAG_LENGTH   1024
#define OMO_UI_SEEK_RESOLUTION  2000

typedef struct
{

	ALLEGRO_DISPLAY * display;
	OMO_THEME * theme;
	T3GUI_DIALOG * dialog;

} OMO_UI_POPUP_DIALOG;

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
	T3GUI_ELEMENT * ui_seek_control_element;
	bool ui_seeked;
	int selected_song;
	int mode;

	/* tags dialog */
	OMO_UI_POPUP_DIALOG * tags_popup_dialog;
	char original_tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	char tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	const char * tags_entry;
	T3GUI_ELEMENT * tags_ok_button_element;
	int tags_queue_entry;

	/* split track dialog */
	OMO_UI_POPUP_DIALOG * split_track_popup_dialog;
	char original_split_track_text[OMO_UI_MAX_TAG_LENGTH];
	char split_track_text[OMO_UI_MAX_TAG_LENGTH];
	const char * split_track_fn;
	const char * split_track_entry;
	T3GUI_ELEMENT * split_track_ok_button_element;
	int split_track_queue_entry;

	/* theme data */
	OMO_THEME * main_theme;
	char ui_button_text[6][8];

} OMO_UI;

OMO_UI * omo_create_ui(void);
void omo_destroy_ui(OMO_UI * uip);
void omo_resize_ui(OMO_UI * uip, int mode, int width, int height);
bool omo_create_main_dialog(OMO_UI * uip, int mode, int width, int height, void * data);

OMO_UI_POPUP_DIALOG * omo_create_popup_dialog(const char * theme_file, int w, int h, void * data);
void omo_close_popup_dialog(OMO_UI_POPUP_DIALOG * dp);

bool omo_open_tags_dialog(OMO_UI * uip, void * data);
void omo_close_tags_dialog(OMO_UI * uip, void * data);

bool omo_open_split_track_dialog(OMO_UI * uip, void * data);
void omo_close_split_track_dialog(OMO_UI * uip, void * data);

#endif
