#include "../t3f/t3f.h"
#include "../t3gui/t3gui.h"
#include "../t3gui/resource.h"
#include "../instance.h"
#include "ui.h"
#include "dialog_proc.h"
#include "../constants.h"
#include "../profile.h"

#define OMO_BEZEL_TOP    1
#define OMO_BEZEL_BOTTOM 2
#define OMO_BEZEL_LEFT   4
#define OMO_BEZEL_RIGHT  8

static const int bezel = 8;
static const int slider_size = 16;
static const int button_size = 32;

/* adjust the passed rectangle for flags */
static void setup_module_box(int * x, int * y, int * w, int * h, int flags)
{
	if(x)
	{
		*x += bezel;
	}
	if(y)
	{
		*y += bezel;
	}
	if(w)
	{
		*w -= bezel * 2;
	}
	if(h)
	{
		*h -= bezel * 2;
	}

	if(flags & OMO_BEZEL_LEFT)
	{
		if(w)
		{
			*w += bezel / 2;
		}
		if(x)
		{
			*x -= bezel / 2;
		}
	}
	if(flags & OMO_BEZEL_RIGHT)
	{
		if(w)
		{
			*w += bezel / 2;
		}
	}
	if(flags & OMO_BEZEL_TOP)
	{
		if(h)
		{
			*h += bezel / 2;
		}
		if(y)
		{
			*y -= bezel / 2;
		}
	}
	if(flags & OMO_BEZEL_BOTTOM)
	{
		if(h)
		{
			*h += bezel / 2;
		}
	}
}

/* place player module elements inside rectangle defined by parameters */
static void setup_player_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	int button_y, button_width, button_height;
	int i;
	int pos_y;
	int font_height;

	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);

	/* adjust bezels so adjacent items are only 'bezel' pixels away */
	setup_module_box(&x, &y, &w, &h, flags);
	pos_y = y;

	uip->ui_song_info_box_element->x = x;
	uip->ui_song_info_box_element->y = pos_y;
	uip->ui_song_info_box_element->w = w - slider_size - bezel;
	uip->ui_song_info_box_element->h = font_height * 2 + 4;
	uip->ui_volume_control_element->x = x + (w - slider_size);
	uip->ui_volume_control_element->y = pos_y;
	uip->ui_volume_control_element->w = slider_size;
	uip->ui_volume_control_element->h = font_height * 2 + 4;
	pos_y += 2;
	uip->ui_song_info_1_element->x = x + 4;
	uip->ui_song_info_1_element->y = pos_y;
	uip->ui_song_info_1_element->w = w - slider_size - bezel - 5;
	uip->ui_song_info_1_element->h = font_height * 2 + 4;
	pos_y += font_height;
	uip->ui_song_info_2_element->x = x + 4;
	uip->ui_song_info_2_element->y = pos_y;
	uip->ui_song_info_2_element->w = w - slider_size - bezel - 5;
	uip->ui_song_info_2_element->h = font_height * 2 + 4;
	pos_y += font_height + 4 + bezel / 2;

	uip->ui_seek_control_element->x = x;
	uip->ui_seek_control_element->y = pos_y;
	uip->ui_seek_control_element->w = w;
	uip->ui_seek_control_element->h = slider_size;
	pos_y += slider_size + bezel;

	button_height = button_size;
	button_y = pos_y;

	button_width = w / 6;
	for(i = 0; i < 6; i++)
	{
		uip->ui_button_element[i]->x = x + button_width * i;
		uip->ui_button_element[i]->y = button_y;
		uip->ui_button_element[i]->w = button_width;
		uip->ui_button_element[i]->h = button_height;
	}
}

static void setup_queue_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(&x, &y, &w, &h, flags);
	uip->ui_queue_list_element->x = x;
	uip->ui_queue_list_element->y = y;
	uip->ui_queue_list_element->w = w;
	uip->ui_queue_list_element->h = h;
}

static void setup_library_artist_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(&x, &y, &w, &h, flags);
	uip->ui_artist_list_element->x = x;
	uip->ui_artist_list_element->y = y;
	uip->ui_artist_list_element->w = w;
	uip->ui_artist_list_element->h = h;
}

static void setup_library_album_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(&x, &y, &w, &h, flags);
	uip->ui_album_list_element->x = x;
	uip->ui_album_list_element->y = y;
	uip->ui_album_list_element->w = w;
	uip->ui_album_list_element->h = h;
}

static void setup_library_song_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(&x, &y, &w, &h, flags);
	uip->ui_song_list_element->x = x;
	uip->ui_song_list_element->y = y;
	uip->ui_song_list_element->w = w;
	uip->ui_song_list_element->h = h;
}

static void setup_library_status_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(&x, &y, &w, NULL, flags);
	uip->ui_status_bar_element->x = x;
	uip->ui_status_bar_element->y = y;
	uip->ui_status_bar_element->w = w;
	uip->ui_status_bar_element->h = h;
}

static void resize_dialogs(OMO_UI * uip, int mode, int width, int height)
{
	int player_x, player_y, player_width, player_height;
	int queue_list_x, queue_list_y, queue_list_width, queue_list_height;
	int artist_list_x, artist_list_y, artist_list_width, artist_list_height;
	int album_list_x, album_list_y, album_list_width, album_list_height;
	int song_list_x, song_list_y, song_list_width, song_list_height;
	int status_bar_x, status_bar_y, status_bar_width, status_bar_height;
	int pane_width;
	int font_height;
/*	int queue_width, queue_height;
	int pane_width;
	int button_y, button_width, button_height;
	int bezel;
	int i; */

	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);

	uip->ui_queue_list_box_element->w = width;
	uip->ui_queue_list_box_element->h = height;
	if(mode == 0)
	{
		player_width = width;
		player_height = bezel + button_size + bezel / 2 + slider_size + bezel / 2 + font_height * 2 + 4 + bezel / 2 + bezel;
		player_x = 0;
		player_y = height - player_height;
		queue_list_x = 0;
		queue_list_y = 0;
		queue_list_width = width;
		queue_list_height = height - player_height;

		setup_player_module(uip, player_x, player_y, player_width, player_height, OMO_BEZEL_TOP);
		setup_queue_list_module(uip, queue_list_x, queue_list_y, queue_list_width, queue_list_height, OMO_BEZEL_BOTTOM);
	}
	else
	{
		pane_width = width / 4;

		player_width = pane_width;
		player_height = bezel + button_size + bezel / 2 + slider_size + bezel / 2 + font_height * 2 + 4 + bezel / 2 + bezel;
		player_x = pane_width * 3;
		player_y = height - player_height;

		artist_list_x = pane_width * 0;
		artist_list_y = 0;
		artist_list_width = pane_width;
		artist_list_height = height - font_height - 4;

		album_list_x = pane_width * 1;
		album_list_y = 0;
		album_list_width = pane_width;
		album_list_height = height - font_height - 4;

		song_list_x = pane_width * 2;
		song_list_y = 0;
		song_list_width = pane_width;
		song_list_height = height - font_height - 4;

		queue_list_x = pane_width * 3;
		queue_list_y = 0;
		queue_list_width = pane_width;
		queue_list_height = height - player_height;

		status_bar_x = 0;
		status_bar_y = artist_list_y + artist_list_height - 8;
		status_bar_width = pane_width * 3;
		status_bar_height = font_height + 4;

		setup_library_artist_list_module(uip, artist_list_x, artist_list_y, artist_list_width, artist_list_height, OMO_BEZEL_RIGHT);
		setup_library_album_list_module(uip, album_list_x, album_list_y, album_list_width, album_list_height, OMO_BEZEL_LEFT | OMO_BEZEL_RIGHT);
		setup_library_song_list_module(uip, song_list_x, song_list_y, song_list_width, song_list_height, OMO_BEZEL_LEFT | OMO_BEZEL_RIGHT);
		setup_queue_list_module(uip, queue_list_x, queue_list_y, queue_list_width, queue_list_height, OMO_BEZEL_LEFT | OMO_BEZEL_BOTTOM);
		setup_player_module(uip, player_x, player_y, player_width, player_height, OMO_BEZEL_LEFT | OMO_BEZEL_TOP);
		setup_library_status_module(uip, status_bar_x, status_bar_y, status_bar_width, status_bar_height, OMO_BEZEL_TOP);
	}
}

static bool load_ui_data(OMO_UI * uip)
{
	const char * val;
	int font_size = 0;

	val = al_get_config_value(t3f_config, "Settings", "font_size_override");
	if(val)
	{
		font_size = atoi(val);
	}
	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->main_theme = omo_load_theme(val, 0, font_size);
	if(!uip->main_theme)
	{
		al_remove_config_key(t3f_config, "Settings", "theme");
		uip->main_theme = omo_load_theme("data/themes/basic/omo_theme.ini", 0, font_size);
		if(!uip->main_theme)
		{
			return false;
		}
	}

	return true;
}

static void free_ui_data(OMO_UI * uip)
{
	omo_destroy_theme(uip->main_theme);
}

static bool omo_add_player_elements(OMO_UI * uip, void * data)
{
	int buttons[6] = {OMO_THEME_BITMAP_PREVIOUS_TRACK, OMO_THEME_BITMAP_PLAY, OMO_THEME_BITMAP_STOP, OMO_THEME_BITMAP_NEXT_TRACK, OMO_THEME_BITMAP_OPEN, OMO_THEME_BITMAP_ADD};
	int width = 16;
	int height = 16;
	int i;

	uip->ui_queue_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_queue_list_proc, NULL, data);
	uip->ui_song_info_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_box_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, NULL, NULL, data);
	uip->ui_song_info_1_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, uip->song_info_text[0], NULL, data);
	uip->ui_song_info_2_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, uip->song_info_text[1], NULL, data);
	uip->ui_volume_control_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_SLIDER], t3gui_slider_proc, 0, 0, width, height, 0, 0, OMO_UI_VOLUME_RESOLUTION, 0, NULL, NULL, NULL);
	uip->ui_seek_control_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_SLIDER], t3gui_slider_proc, 0, 0, width, height, 0, 0, OMO_UI_SEEK_RESOLUTION, 0, NULL, NULL, NULL);
	for(i = 0; i < 6; i++)
	{
		uip->ui_button_element[i] = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, uip->ui_button_text[i], ui_player_button_proc, uip->main_theme->bitmap[buttons[i]]);
	}

	return true;
}

bool omo_create_main_dialog(OMO_UI * uip, int mode, int width, int height, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(uip->ui_dialog)
	{
		t3gui_destroy_dialog(uip->ui_dialog);
	}
	uip->ui_dialog = t3gui_create_dialog();
	if(!uip->ui_dialog)
	{
		return false;
	}
	uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);
	if(mode == 0)
	{
		omo_add_player_elements(uip, data);
		resize_dialogs(uip, mode, width, height);
		return true;
	}
	else
	{
		uip->ui_dialog = t3gui_create_dialog();
		if(!uip->ui_dialog)
		{
			return false;
		}

		uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);

		uip->ui_artist_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_artist_list_proc, NULL, data);

		uip->ui_album_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_album_list_proc, NULL, data);

		uip->ui_song_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_song_list_proc, NULL, data);

		uip->ui_status_bar_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8, 8, 32, al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]), 0, 0, 0, 0, (void *)app->status_bar_text, NULL, NULL);

		/* create queue list and controls */
		omo_add_player_elements(uip, data);
		resize_dialogs(uip, mode, width, height);
		return true;
	}
	return false;
}

OMO_UI * omo_create_ui(void)
{
	OMO_UI * uip;

	uip = malloc(sizeof(OMO_UI));
	if(uip)
	{
		memset(uip, 0, sizeof(OMO_UI));
		load_ui_data(uip);
	}
	return uip;
}

void omo_destroy_ui(OMO_UI * uip)
{
	free_ui_data(uip);
	t3gui_destroy_dialog(uip->ui_dialog);
	free(uip);
}

void omo_resize_ui(OMO_UI * uip, int mode, int width, int height)
{
	resize_dialogs(uip, mode, width, height);
}

static void get_center(ALLEGRO_DISPLAY * dp, int w, int h, int * x, int * y)
{
	int dx, dy;

	al_get_window_position(dp, &dx, &dy);
	if(x)
	{
		*x = dx + al_get_display_width(dp) / 2 - w / 2;
	}
	if(y)
	{
		*y = dy + al_get_display_height(dp) / 2 - h / 2;
	}
}

OMO_UI_POPUP_DIALOG * omo_create_popup_dialog(const char * theme_file, int w, int h, void * data)
{
	OMO_UI_POPUP_DIALOG * popup_dialog;
	const char * val;
	int font_size = 0;
	int x, y;

	popup_dialog = malloc(sizeof(OMO_UI_POPUP_DIALOG));
	if(popup_dialog)
	{
		memset(popup_dialog, 0, sizeof(OMO_UI_POPUP_DIALOG));
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		get_center(t3f_display, w, h, &x, &y);
		al_set_new_window_position(x, y);
		popup_dialog->display = al_create_display(w, h);
		if(!popup_dialog->display)
		{
			goto fail;
		}
		al_register_event_source(t3f_queue, al_get_display_event_source(popup_dialog->display));
		al_set_target_bitmap(al_get_backbuffer(popup_dialog->display));
		val = al_get_config_value(t3f_config, "Settings", "font_size_override");
		if(val)
		{
			font_size = atoi(val);
		}
		if(theme_file)
		{
			popup_dialog->theme = omo_load_theme(theme_file, 1, font_size);
			if(!popup_dialog->theme)
			{
				goto fail;
			}
		}
		popup_dialog->dialog = t3gui_create_dialog();
		if(!popup_dialog->dialog)
		{
			goto fail;
		}
	}
	return popup_dialog;

	fail:
	{
		if(popup_dialog)
		{
			if(popup_dialog->dialog)
			{
				t3gui_destroy_dialog(popup_dialog->dialog);
			}
			if(popup_dialog->theme)
			{
				omo_destroy_theme(popup_dialog->theme);
			}
			if(popup_dialog->display)
			{
				al_destroy_display(popup_dialog->display);
			}
			free(popup_dialog);
		}
	}
	return NULL;
}

void omo_close_popup_dialog(OMO_UI_POPUP_DIALOG * dp)
{
	t3gui_close_dialog(dp->dialog);
	t3gui_destroy_dialog(dp->dialog);
	t3gui_unload_resources(dp->display, true);
	omo_destroy_theme(dp->theme);
	al_destroy_display(dp->display);
	free(dp);
}

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
	h *= rows / 2;
	h += 8;

	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->tags_popup_dialog = omo_create_popup_dialog(val, 640, h, data);
	if(uip->tags_popup_dialog)
	{
		t3gui_dialog_add_element(uip->tags_popup_dialog->dialog, uip->tags_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, 640, h, 0, 0, 0, 0, NULL, NULL, NULL);
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(omo_tag_type[i])
			{
				row++;
				if(row > rows / 2)
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
		t3gui_dialog_add_element(uip->split_track_popup_dialog->dialog, uip->split_track_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, 320, h, 0, 0, 0, 0, NULL, NULL, NULL);
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

bool omo_open_tagger_key_dialog(OMO_UI * uip, void * data)
{
	int pos_y = 8;
	int h;
	const char * val;

	h = 8 + (al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8) * 2 + 8 + 32 + 12;
	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->tagger_key_popup_dialog = omo_create_popup_dialog(val, 320, h, data);
	if(uip->tagger_key_popup_dialog)
	{
		val = al_get_config_value(t3f_config, "Settings", "tagger_name");
		if(val)
		{
			strcpy(uip->tagger_key_text, val);
		}
		else
		{
			strcpy(uip->tagger_key_text, "");
		}
		t3gui_dialog_add_element(uip->tagger_key_popup_dialog->dialog, uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, 320, h, 0, 0, 0, 0, NULL, NULL, NULL);
		t3gui_dialog_add_element(uip->tagger_key_popup_dialog->dialog, uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]), 0, 0, 0, 0, (void *)"Name", NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		strcpy(uip->original_tagger_key_text, uip->tagger_key_text);
		t3gui_dialog_add_element(uip->tagger_key_popup_dialog->dialog, uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, D_SETFOCUS, 256, 0, uip->tagger_key_text, NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		pos_y += 12;
		uip->tagger_key_ok_button_element = t3gui_dialog_add_element(uip->tagger_key_popup_dialog->dialog, uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, pos_y, 320 / 2 - 12, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
		t3gui_dialog_add_element(uip->tagger_key_popup_dialog->dialog, uip->tagger_key_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 320 / 2 + 4, pos_y, 320 / 2 - 12, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->tagger_key_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		return true;
	}
	return false;
}

void omo_close_tagger_key_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->tagger_key_popup_dialog);
	uip->tagger_key_popup_dialog = NULL;
}

bool omo_open_new_profile_dialog(OMO_UI * uip, void * data)
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
	uip->new_profile_popup_dialog = omo_create_popup_dialog(val, 320, h, data);
	if(uip->new_profile_popup_dialog)
	{
		strcpy(uip->new_profile_text, "");
		t3gui_dialog_add_element(uip->new_profile_popup_dialog->dialog, uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, 320, h, 0, 0, 0, 0, NULL, NULL, NULL);
		t3gui_dialog_add_element(uip->new_profile_popup_dialog->dialog, uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]), 0, 0, 0, 0, (void *)"Name", NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		t3gui_dialog_add_element(uip->new_profile_popup_dialog->dialog, uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, pos_y, 320 - 16, al_get_font_line_height(uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, D_SETFOCUS, 256, 0, uip->new_profile_text, NULL, NULL);
		pos_y += al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 8;
		pos_y += 12;
		uip->new_profile_ok_button_element = t3gui_dialog_add_element(uip->new_profile_popup_dialog->dialog, uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, pos_y, 320 / 2 - 12, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
		t3gui_dialog_add_element(uip->new_profile_popup_dialog->dialog, uip->new_profile_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 320 / 2 + 4, pos_y, 320 / 2 - 12, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->new_profile_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		return true;
	}
	return false;
}

void omo_close_new_profile_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->new_profile_popup_dialog);
	uip->new_profile_popup_dialog = NULL;
}

typedef struct
{

	const char * type[256];
	int types;

} TYPE_LIST;

static bool find_type(const char * check_type, TYPE_LIST * lp)
{
	int i;

	for(i = 0; i < lp->types; i++)
	{
		if(!strcmp(lp->type[i], check_type))
		{
			return true;
		}
	}
	return false;
}

static void clear_tokens(char * filter, int length)
{
	int i;

	for(i = 0; i < length; i++)
	{
		if(filter[i] == 0)
		{
			filter[i] = ';';
		}
	}
}

static bool check_filter(const char * type, char * filter)
{
	char * token;
	int l;

	if(filter)
	{
		l = strlen(filter);
		token = strtok(filter, "; ");
		while(token)
		{
			if(!strcasecmp(token, type))
			{
				clear_tokens(filter, l);
				return true;
			}
			token = strtok(NULL, "; ");
		}
		clear_tokens(filter, l);
		return false;
	}
	return true;
}

bool omo_open_filter_dialog(OMO_UI * uip, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	TYPE_LIST types;
	char section_buffer[1024];
	const char * val;
	char * filter = NULL;
	int y = 8;
	int rows = 0;
	int row = 0;
	int old_column = 0;
	int column = 0;
	int columns = 4;
	int i, j;
	int w, h;
	int flags;

	/* get type list */
	types.types = 0;
	for(i = 0; i < app->codec_handler_registry->codec_handlers; i++)
	{
		for(j = 0; j < app->codec_handler_registry->codec_handler[i].types; j++)
		{
			if(!find_type(app->codec_handler_registry->codec_handler[i].type[j], &types))
			{
				types.type[types.types] = app->codec_handler_registry->codec_handler[i].type[j];
				types.types++;
			}
		}
	}
	omo_get_profile_section(app->library_config, omo_get_profile(omo_get_current_profile()), section_buffer);
	val = al_get_config_value(t3f_config, section_buffer, "filter");
	if(val)
	{
		filter = strdup(val);
	}

	for(i = 0; i < types.types; i++)
	{
		rows++;
	}
	if(rows % columns)
	{
		rows += rows % columns;
	}
	w = al_get_text_width(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0], "FORMAT") + 32;
	h = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) * 2 + 4;
	h *= rows / columns;
	h += 8;
	h += 32;

	val = al_get_config_value(t3f_config, "Settings", "theme");
	if(!val)
	{
		val = "data/themes/basic/omo_theme.ini";
	}
	uip->filter_popup_dialog = omo_create_popup_dialog(val, w * columns, h, data);
	if(uip->filter_popup_dialog)
	{
		uip->filter_types = 0;
		t3gui_dialog_add_element(uip->filter_popup_dialog->dialog, uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, w * columns, h, 0, 0, 0, 0, NULL, NULL, NULL);
		for(i = 0; i < types.types; i++)
		{
			old_column = column;
			column = row / (rows / columns);
			if(column != old_column)
			{
				y = 8;
			}
			row++;
			if(check_filter(&(types.type[i][1]), filter))
			{
				uip->filter_type_selected[i] = true;
				flags = D_SELECTED;
			}
			else
			{
				uip->filter_type_selected[i] = false;
				flags = 0;
			}
			uip->filter_type_element[i] = t3gui_dialog_add_element(uip->filter_popup_dialog->dialog, uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_CHECK_BOX], t3gui_check_proc, 8 + w * column, y, w - 16, al_get_font_line_height(uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_CHECK_BOX]->state[0].font[0]), 0, flags, 0, 0, (void *)&(types.type[i][1]), NULL, NULL);
			uip->filter_types++;
			y += al_get_font_line_height(uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 2;
			y += al_get_font_line_height(uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 2;
		}
		y = h - 32 - 8;
		uip->filter_ok_button_element = t3gui_dialog_add_element(uip->filter_popup_dialog->dialog, uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, y, (w * columns) / 2 - 8 - 4, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
		t3gui_dialog_add_element(uip->filter_popup_dialog->dialog, uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, (w * columns) / 2 + 4, y, (w * columns) / 2 - 8 - 4, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
		t3gui_show_dialog(uip->filter_popup_dialog->dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
		if(filter)
		{
			free(filter);
		}
		return true;
	}
	if(filter)
	{
		free(filter);
	}
	return false;
}

void omo_close_filter_dialog(OMO_UI * uip, void * data)
{
	omo_close_popup_dialog(uip->filter_popup_dialog);
	uip->filter_popup_dialog = NULL;
}
