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

static const int default_bezel = 8;
static const int default_slider_size = 16;
static const int default_button_size = 32;

/* adjust the passed rectangle for flags */
static void setup_module_box(OMO_THEME * tp, int * x, int * y, int * w, int * h, int flags)
{
	const char * val;
	int bezel = default_bezel;

	val = al_get_config_value(tp->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}
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
	int time_width;
	int time_height;
	const char * val;
	int bezel = default_bezel;
	int slider_size = default_slider_size;
	int button_size = default_button_size;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}
	val = al_get_config_value(uip->main_theme->config, "Settings", "slider_size");
	if(val)
	{
		slider_size = atoi(val);
	}
	val = al_get_config_value(uip->main_theme->config, "Settings", "button_size");
	if(val)
	{
		button_size = atoi(val);
	}
	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);
	time_width = al_get_text_width(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO]->state[0].font[0], "00:00:00");
	time_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO]->state[0].font[0]);

	/* adjust bezels so adjacent items are only 'bezel' pixels away */
	setup_module_box(uip->main_theme, &x, &y, &w, &h, flags);
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
	uip->ui_song_info_1_element->h = font_height + 4;
	pos_y += font_height;
	uip->ui_song_info_2_element->x = x + 4;
	uip->ui_song_info_2_element->y = pos_y;
	uip->ui_song_info_2_element->w = w - slider_size - bezel - 5;
	uip->ui_song_info_2_element->h = font_height + 4;
	pos_y += font_height + 4 + bezel - 2;

	uip->ui_seek_control_element->x = x;
	uip->ui_seek_control_element->y = pos_y;
	uip->ui_seek_control_element->w = w - time_width - bezel;
	uip->ui_seek_control_element->h = slider_size;
	uip->ui_current_time_element->x = x + w - time_width + bezel;
	uip->ui_current_time_element->y = pos_y - 2;
	uip->ui_current_time_element->w = time_width - bezel;
	uip->ui_current_time_element->h = time_height;
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
	int font_height;
	const char * val;
	int bezel = default_bezel;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}
	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);
	setup_module_box(uip->main_theme, &x, &y, &w, &h, flags);
	uip->ui_queue_list_element->x = x;
	uip->ui_queue_list_element->y = y;
	uip->ui_queue_list_element->w = w;
	uip->ui_queue_list_element->h = h - bezel - font_height - 4;
	uip->ui_queue_info_box_element->x = x;
	uip->ui_queue_info_box_element->y = y + uip->ui_queue_list_element->h + bezel;
	uip->ui_queue_info_box_element->w = w;
	uip->ui_queue_info_box_element->h = font_height + 4;
	uip->ui_queue_info_element->x = x + 4;
	uip->ui_queue_info_element->y = y + uip->ui_queue_list_element->h + bezel + 2;
	uip->ui_queue_info_element->w = w - bezel;
	uip->ui_queue_info_element->h = font_height + 4;
}

static void setup_library_artist_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	int pos_y;
	int font_height;
	const char * val;
	int bezel = default_bezel;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}

	setup_module_box(uip->main_theme, &x, &y, &w, &h, flags);
	pos_y = y;
	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);
	uip->ui_artist_search_element->x = x;
	uip->ui_artist_search_element->y = pos_y;
	uip->ui_artist_search_element->w = w;
	uip->ui_artist_search_element->h = font_height + 4;
	pos_y += font_height + 4 + bezel;

	uip->ui_artist_list_element->x = x;
	uip->ui_artist_list_element->y = pos_y;
	uip->ui_artist_list_element->w = w;
	uip->ui_artist_list_element->h = h - (font_height + bezel);
}

static void setup_library_album_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	int pos_y;
	int font_height;
	const char * val;
	int bezel = default_bezel;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}

	setup_module_box(uip->main_theme, &x, &y, &w, &h, flags);
	pos_y = y;
	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);
	uip->ui_album_search_element->x = x;
	uip->ui_album_search_element->y = pos_y;
	uip->ui_album_search_element->w = w;
	uip->ui_album_search_element->h = font_height + 4;
	pos_y += font_height + 4 + bezel;

	uip->ui_album_list_element->x = x;
	uip->ui_album_list_element->y = pos_y;
	uip->ui_album_list_element->w = w;
	uip->ui_album_list_element->h = h - (font_height + bezel);
}

static void setup_library_song_list_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	int pos_y;
	int font_height;
	const char * val;
	int bezel = default_bezel;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}

	setup_module_box(uip->main_theme, &x, &y, &w, &h, flags);
	pos_y = y;
	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);
	uip->ui_song_search_element->x = x;
	uip->ui_song_search_element->y = pos_y;
	uip->ui_song_search_element->w = w;
	uip->ui_song_search_element->h = font_height + 4;
	pos_y += font_height + 4 + bezel;

	uip->ui_song_list_element->x = x;
	uip->ui_song_list_element->y = pos_y;
	uip->ui_song_list_element->w = w;
	uip->ui_song_list_element->h = h - (font_height + bezel);
}

static void setup_library_status_module(OMO_UI * uip, int x, int y, int w, int h, int flags)
{
	setup_module_box(uip->main_theme, &x, &y, &w, NULL, flags);
	uip->ui_status_bar_element->x = x;
	uip->ui_status_bar_element->y = y;
	uip->ui_status_bar_element->w = w;
	uip->ui_status_bar_element->h = h - 4;
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
	const char * val;
	int bezel = default_bezel;
	int slider_size = default_slider_size;
	int button_size = default_button_size;

	val = al_get_config_value(uip->main_theme->config, "Settings", "bezel");
	if(val)
	{
		bezel = atoi(val);
	}
	val = al_get_config_value(uip->main_theme->config, "Settings", "slider_size");
	if(val)
	{
		slider_size = atoi(val);
	}
	val = al_get_config_value(uip->main_theme->config, "Settings", "button_size");
	if(val)
	{
		button_size = atoi(val);
	}

	font_height = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]);

	uip->ui_queue_list_box_element->w = width;
	uip->ui_queue_list_box_element->h = height;
	if(mode == 0)
	{
		player_width = width;
		player_height = bezel + button_size + bezel + slider_size + bezel + font_height * 2 + 4 + bezel / 2;
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
	uip->ui_queue_info_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_box_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, NULL, NULL, data);
	uip->ui_queue_info_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, uip->queue_info_text, NULL, data);
	uip->ui_song_info_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_box_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, NULL, NULL, data);
	uip->ui_song_info_1_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, uip->song_info_text[0], NULL, data);
	uip->ui_song_info_2_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, uip->song_info_text[1], NULL, data);
	uip->ui_volume_control_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_SLIDER], t3gui_slider_proc, 0, 0, width, height, 0, 0, OMO_UI_VOLUME_RESOLUTION, 0, NULL, NULL, NULL);
	uip->ui_seek_control_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_SLIDER], t3gui_slider_proc, 0, 0, width, height, 0, 0, OMO_UI_SEEK_RESOLUTION, 0, NULL, NULL, NULL);
	uip->ui_current_time_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_INFO], t3gui_text_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, uip->current_time_text, NULL, data);
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

		uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_WINDOW_BOX], t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);

		uip->ui_artist_search_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, 0, 320 - 16, al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, 0, 256, 0, uip->artist_search_text, NULL, NULL);
		uip->ui_artist_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_artist_list_proc, NULL, data);

		uip->ui_album_search_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, 0, 320 - 16, al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, 0, 256, 0, uip->album_search_text, NULL, NULL);
		uip->ui_album_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_album_list_proc, NULL, data);

		uip->ui_song_search_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8, 0, 320 - 16, al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) + 4, 0, 0, 256, 0, uip->song_search_text, NULL, NULL);
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
