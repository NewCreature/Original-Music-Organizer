#include "../t3f/t3f.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../cloud.h"
#include "dialog_proc.h"
#include "menu_proc.h"

bool omo_open_about_dialog(OMO_UI * uip, void * data)
{
	ALLEGRO_FONT * font;
	const char * val;
	int x, y;
	int font_height;
	char * text_line_1 = T3F_APP_TITLE " v" T3F_APP_VERSION;
	char * text_line_2 = T3F_APP_COPYRIGHT ".";

	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);

	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->about_popup_dialog = omo_create_popup_dialog(val, 320, 32 + 16 + font_height * 2 + 8, data);
	if(uip->about_popup_dialog)
	{
		font = uip->about_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0];
		y = 8;
		t3gui_dialog_add_element(uip->about_popup_dialog->dialog, uip->about_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_WINDOW_BOX], t3gui_box_proc, 0, 0, 320, 240, 0, 0, 0, 0, NULL, NULL, NULL);
		x = 160 - al_get_text_width(font, text_line_1) / 2;
		t3gui_dialog_add_element(uip->about_popup_dialog->dialog, uip->about_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, x, y, 320, font_height, 0, 0, 0, 0, text_line_1, NULL, NULL);
		y += font_height;
		x = 160 - al_get_text_width(font, text_line_2) / 2;
		t3gui_dialog_add_element(uip->about_popup_dialog->dialog, uip->about_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, x, y, 320, font_height, 0, 0, 0, 0, text_line_2, NULL, NULL);
		y += font_height;
		y += 8;
		t3gui_dialog_add_element(uip->about_popup_dialog->dialog, uip->about_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, y, 320 - 16, 32, 0, 0, 0, 1, "Okay", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->about_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		return true;
	}
	return false;
}

void omo_close_about_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->about_popup_dialog);
	uip->about_popup_dialog = NULL;
}

void omo_about_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_about_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed >= 0)
	{
		omo_close_about_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
