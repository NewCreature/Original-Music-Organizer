#ifndef OMO_UI_H
#define OMO_UI_H

typedef struct
{
    T3GUI_DIALOG * ui_dialog;
	T3GUI_ELEMENT * ui_queue_list_box_element;
	T3GUI_ELEMENT * ui_queue_list_element;
	T3GUI_ELEMENT * ui_button_element[6];
	T3GUI_THEME * ui_box_theme;
	T3GUI_THEME * ui_list_box_theme;
	T3GUI_THEME * ui_button_theme;
	char ui_button_text[6][8];
    int mode;

} OMO_UI;

OMO_UI * omo_create_ui(int mode, int width, int height, void * data);
void omo_destroy_ui(OMO_UI * uip);
void omo_resize_ui(OMO_UI * uip, int width, int height);

#endif
