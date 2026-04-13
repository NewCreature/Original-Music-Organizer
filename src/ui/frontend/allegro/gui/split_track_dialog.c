#include "t3f/t3f.h"
#include "t3f/file.h"
#include "instance.h"
#include "constants.h"
#include "queue_helpers.h"
#include "library_helpers.h"
#include "cloud.h"
#include "threads.h"
#include "dialog_proc.h"
#include "ui.h"

bool omo_open_split_track_dialog(OMO_UI * uip, void * data)
{
	const char * val;
	int pos_y = 8;
	int h;

	h = 8 + (al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8) * 2 + 8 + 32 + 12;
	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->split_track_popup_dialog = omo_create_popup_dialog(val, 320, h, data);
	if(uip->split_track_popup_dialog)
	{
		t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_WINDOW_BOX], t3gui_box_proc, 0, 0, 320, h, 0, 0, 0, 0, NULL, NULL, NULL);
		t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]), 0, 0, 0, 0, (void *)"Track List (e.g. 0,20,40)", NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		strcpy(uip->original_split_track_text, uip->split_track_text);
		t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, D_SETFOCUS, 256, 0, uip->split_track_text, NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		pos_y += 12;
		uip->split_track_ok_button_element = t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, pos_y, 320 / 2 - 12, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
		t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 320 / 2 + 4, pos_y, 320 / 2 - 12, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->split_track_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		return true;
	}
	return false;
}

void omo_close_split_track_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->split_track_popup_dialog);
	uip->split_track_popup_dialog = NULL;
}

void omo_split_track_dialog_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE))
	{
		omo_close_split_track_dialog(uip, uip->app);
		t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
	}
	if(uip->app->button_pressed == 0)
	{
		if(strcmp(uip->original_split_track_text, uip->split_track_text))
		{
			omo_split_track(uip->app->library, uip->split_track_fn, uip->split_track_text);
			omo_set_database_value(uip->app->library->entry_database, uip->split_track_entry, "Submitted", "false");
			omo_spawn_cloud_thread(uip->app);
			omo_save_library(uip->app->library);
			uip->app->spawn_library_thread = true;
			uip->app->destroy_library_lists_cache = true;
		}
		omo_discard_entry_backup(uip->app->library);
		omo_close_split_track_dialog(uip, uip->app);
		uip->app->button_pressed = -1;
		t3f_use_key_press(ALLEGRO_KEY_ENTER);
	}
	else if(uip->app->button_pressed == 1)
	{
		omo_restore_entry_tags(uip->app->library);
		omo_close_split_track_dialog(uip, uip->app);
		uip->app->button_pressed = -1;
	}
}
