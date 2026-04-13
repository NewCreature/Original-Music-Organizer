#include "t3f/t3f.h"
#include "instance.h"
#include "constants.h"
#include "queue_helpers.h"
#include "library_helpers.h"
#include "cloud.h"
#include "threads.h"
#include "dialog_proc.h"
#include "menu_proc.h"
#include "ui.h"

bool omo_open_tags_dialog(OMO_UI * uip, void * data)
{
	const char * val;
	int y = 8;
	int h = 8;
	int row = 0;
	int rows = 0;
	int column = 0;
	int i;
	int edit_flags = D_SETFOCUS;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(omo_tag_type[i])
		{
			rows++;
		}
	}
	if(rows % 2)
	{
		rows++;
	}
	h = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) * 3 + 4;
	h *= (rows / 2 + 1);
	h += 8;

	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->tags_popup_dialog = omo_create_popup_dialog(val, 640, h, data);
	if(uip->tags_popup_dialog)
	{
		t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_WINDOW_BOX], t3gui_box_proc, 0, 0, 640, h, 0, 0, 0, 0, NULL, NULL, NULL);
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(omo_tag_type[i])
			{
				row++;
				if(row > rows / 2 + rows % 2)
				{
					row = 0;
					column = 1;
					y = 8;
				}
				t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8 + 320 * column, y, 320 - 16, al_get_font_line_height(uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]), 0, 0, 0, 0, (void *)omo_tag_type[i], NULL, NULL);
				y += al_get_font_line_height(uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 2;
				strcpy(uip->original_tags_text[i], uip->tags_text[i]);
				t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8 + 320 * column, y, 320 - 16, al_get_font_line_height(uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, edit_flags, 256, 0, uip->tags_text[i], NULL, NULL);
				edit_flags = 0;
				y += al_get_font_line_height(uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) * 2 + 2;
			}
		}
		y += 12;
		uip->tags_ok_button_element = t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8 + 320 * column, y, 320 / 2 - 8 - 4, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
		t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 320 * column + 320 / 2 + 4, y, 320 / 2 - 8 - 4, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->tags_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		return true;
	}
	return false;
}

void omo_close_tags_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->tags_popup_dialog);
	uip->tags_popup_dialog = NULL;
}

void omo_tags_dialog_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	bool update_artists = false;
	bool update_albums = false;
	bool update_songs = false;
	bool update_tags = false;
	const char * val;
	int i;

	if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE))
	{
		omo_close_tags_dialog(uip, uip->app);
		t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
	}
	if((t3f_key_held(ALLEGRO_KEY_LCTRL) || t3f_key_held(ALLEGRO_KEY_RCTRL) || t3f_key_held(ALLEGRO_KEY_COMMAND)) && t3f_key_pressed(ALLEGRO_KEY_C))
	{
		omo_menu_edit_copy_tags(0, data);
		t3f_use_key_press(ALLEGRO_KEY_C);
	}
	else if((t3f_key_held(ALLEGRO_KEY_LCTRL) || t3f_key_held(ALLEGRO_KEY_RCTRL) || t3f_key_held(ALLEGRO_KEY_COMMAND)) && t3f_key_pressed(ALLEGRO_KEY_V))
	{
		omo_menu_edit_paste_tags(0, data);
		t3f_use_key_press(ALLEGRO_KEY_V);
	}

	if(uip->app->button_pressed == 0)
	{
		if(uip->app->library_view)
		{
			val = ui_artist_list_proc(uip->ui_artist_list_element->d1, NULL, NULL, uip->app);
			if(val)
			{
				strcpy(uip->app->edit_artist, val);
			}
			val = ui_album_list_proc(uip->ui_album_list_element->d1, NULL, NULL, uip->app);
			if(val)
			{
				strcpy(uip->app->edit_album, val);
			}
		}
		strcpy(uip->app->edit_song_id, uip->tags_entry);
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(omo_tag_type[i] && strcmp(uip->tags_text[i], uip->original_tags_text[i]))
			{
				if(!strcmp(omo_tag_type[i], "Artist"))
				{
					update_artists = true;
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Album Artist"))
				{
					update_artists = true;
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Album"))
				{
					update_albums = true;
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Title"))
				{
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Disambiguation"))
				{
					update_albums = true;
					update_songs = true;
				}
				if(strlen(uip->tags_text[i]) == 0)
				{
					omo_remove_database_key(uip->app->library->entry_database, uip->tags_entry, omo_tag_type[i]);
				}
				else
				{
					omo_set_database_value(uip->app->library->entry_database, uip->tags_entry, omo_tag_type[i], uip->tags_text[i]);
				}
				update_tags = true;
			}
		}
		if(update_tags)
		{
			omo_set_database_value(uip->app->library->entry_database, uip->tags_entry, "Submitted", "false");
			omo_spawn_cloud_thread(uip->app);
		}
		omo_discard_entry_backup(uip->app->library);
		omo_close_tags_dialog(uip, uip->app);
		if(uip->tags_queue_entry >= 0)
		{
			omo_get_queue_entry_tags(uip->app->player->queue, uip->tags_queue_entry, uip->app->library);
		}
		else
		{
			uip->app->spawn_queue_thread = true;
		}
		if(update_artists || update_albums || update_songs)
		{
			uip->app->spawn_library_lists_thread = true;
			uip->app->destroy_library_lists_cache = true;
		}
		uip->app->button_pressed = -1;
		t3f_use_key_press(ALLEGRO_KEY_ENTER);
	}
	else if(uip->app->button_pressed == 1)
	{
		omo_restore_entry_tags(uip->app->library);
		omo_close_tags_dialog(uip, uip->app);
		uip->app->button_pressed = -1;
	}
}
