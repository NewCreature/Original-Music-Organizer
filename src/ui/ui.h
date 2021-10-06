#ifndef OMO_UI_H
#define OMO_UI_H

#include "theme.h"

#define OMO_UI_MAX_TAGS            16
#define OMO_UI_MAX_TAG_LENGTH    1024
#define OMO_UI_SEEK_RESOLUTION   2000
#define OMO_UI_VOLUME_RESOLUTION  100

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
	T3GUI_ELEMENT * ui_queue_info_box_element;
	T3GUI_ELEMENT * ui_queue_info_element;
	T3GUI_ELEMENT * ui_button_element[6];
	T3GUI_ELEMENT * ui_artist_search_element;
	T3GUI_ELEMENT * ui_artist_list_element;
	T3GUI_ELEMENT * ui_album_search_element;
	T3GUI_ELEMENT * ui_album_list_element;
	T3GUI_ELEMENT * ui_song_search_element;
	T3GUI_ELEMENT * ui_song_list_element;
	T3GUI_ELEMENT * ui_song_info_box_element;
	T3GUI_ELEMENT * ui_song_info_1_element;
	T3GUI_ELEMENT * ui_song_info_2_element;
	T3GUI_ELEMENT * ui_seek_control_element;
	T3GUI_ELEMENT * ui_current_time_element;
	T3GUI_ELEMENT * ui_volume_control_element;
	T3GUI_ELEMENT * ui_status_bar_element;
	bool ui_seeked;
	bool ui_volume_changed;
	int selected_song;
	int mode;
	char queue_info_text[1024];
	char current_time_text[32];
	char song_info_text[2][1024];
	char artist_search_text[256];
	char album_search_text[256];
	char song_search_text[256];
	bool apply_artist_search_filter;
	bool apply_album_search_filter;
	bool apply_song_search_filter;

	/* tags dialog */
	OMO_UI_POPUP_DIALOG * tags_popup_dialog;
	OMO_UI_POPUP_DIALOG * multi_tags_popup_dialog;
	OMO_UI_POPUP_DIALOG * album_tags_popup_dialog;
	char original_tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	char tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
	const char * tags_entry;
	T3GUI_ELEMENT * tags_ok_button_element;
	int tags_queue_entry;
	bool tag_enabled[OMO_MAX_TAG_TYPES];

	/* split track dialog */
	OMO_UI_POPUP_DIALOG * split_track_popup_dialog;
	char original_split_track_text[OMO_UI_MAX_TAG_LENGTH];
	char split_track_text[OMO_UI_MAX_TAG_LENGTH];
	const char * split_track_fn;
	const char * split_track_entry;
	T3GUI_ELEMENT * split_track_ok_button_element;
	int split_track_queue_entry;

	/* tagger key dialog */
	OMO_UI_POPUP_DIALOG * tagger_key_popup_dialog;
	char original_tagger_key_text[OMO_UI_MAX_TAG_LENGTH];
	char tagger_key_text[OMO_UI_MAX_TAG_LENGTH];
	T3GUI_ELEMENT * tagger_key_ok_button_element;

	/* new profile dialog */
	OMO_UI_POPUP_DIALOG * new_profile_popup_dialog;
	char new_profile_text[OMO_UI_MAX_TAG_LENGTH];
	T3GUI_ELEMENT * new_profile_ok_button_element;

	/* filter dialog */
	OMO_UI_POPUP_DIALOG * filter_popup_dialog;
	T3GUI_ELEMENT * filter_ok_button_element;
	T3GUI_ELEMENT * filter_type_element[256];
	bool filter_type_selected[256];
	int filter_types;

	/* about dialog */
	OMO_UI_POPUP_DIALOG * about_popup_dialog;

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

#endif
