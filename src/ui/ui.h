#ifndef OMO_UI_H
#define OMO_UI_H

#define OMO_UI_MAX_TAGS         16
#define OMO_UI_MAX_TAG_LENGTH 1024

typedef struct
{
    /* main dialog */
    T3GUI_DIALOG * ui_dialog;
	T3GUI_ELEMENT * ui_queue_list_box_element;
	T3GUI_ELEMENT * ui_queue_list_element;
	T3GUI_ELEMENT * ui_button_element[6];
    int mode;

    /* tags dialog */
    ALLEGRO_DISPLAY * tags_display;
    T3GUI_DIALOG * tags_dialog;
    char tags_text[OMO_UI_MAX_TAGS][OMO_UI_MAX_TAG_LENGTH];
    const char * tags_entry;

    /* theme data */
    T3GUI_THEME * ui_box_theme;
	T3GUI_THEME * ui_list_box_theme;
	T3GUI_THEME * ui_button_theme;
    T3GUI_THEME * tags_box_theme;
    T3GUI_THEME * tags_list_box_theme;
    T3GUI_THEME * tags_button_theme;
	char ui_button_text[6][8];

} OMO_UI;

OMO_UI * omo_create_ui(int mode, int width, int height, void * data);
void omo_destroy_ui(OMO_UI * uip);
void omo_resize_ui(OMO_UI * uip, int width, int height);

bool omo_open_tags_dialog(OMO_UI * uip, void * data);

#endif
