#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3net/t3net.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../profile.h"
#include "menu_init.h"
#include "dialog_proc.h"

typedef struct
{

	char * type[256];
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

static int sort_types(const void *e1, const void *e2)
{
	char * s1 = *(char **)e1;
	char * s2 = *(char **)e2;
	return strcmp(s1, s2);
}

bool omo_open_filter_dialog(OMO_UI * uip, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	static TYPE_LIST types;
	char section_buffer[1024];
	const char * val;
	char * filter = NULL;
	int y = 8;
	int rows = 0;
	int row_exp = 0;
	int row = 0;
	int old_column = 0;
	int column = 0;
	int columns = 6;
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
	qsort(types.type, types.types, sizeof(char *), sort_types);
	omo_get_profile_section(app->library_config, omo_get_profile(omo_get_current_profile()), section_buffer);
	val = al_get_config_value(t3f_config, section_buffer, "filter");
	if(val)
	{
		filter = strdup(val);
	}

	columns = ceil(sqrt(types.types));
	for(i = 0; i < types.types; i++)
	{
		rows++;
	}
	if(rows % columns)
	{
		row_exp = 1;
	}
	w = al_get_text_width(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0], "FORMAT") + 32;
	h = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font[0]) * 2 + 4;
	h *= rows / columns + row_exp;
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
		t3gui_dialog_add_element(uip->filter_popup_dialog->dialog, uip->filter_popup_dialog->theme->gui_theme[OMO_THEME_GUI_THEME_WINDOW_BOX], t3gui_box_proc, 0, 0, w * columns, h, 0, 0, 0, 0, NULL, NULL, NULL);
		for(i = 0; i < types.types; i++)
		{
			old_column = column;
			column = row / (rows / columns + row_exp);
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

void omo_filter_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char section_buffer[1024];
	char filter_buffer[1024];
	bool first = true;
	bool changed = false;
	bool clear = false;
	int i;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_filter_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		omo_get_profile_section(app->library_config, omo_get_profile(omo_get_current_profile()), section_buffer);
		for(i = 0; i < app->ui->filter_types; i++)
		{
			if(!(app->ui->filter_type_element[i]->flags & D_SELECTED))
			{
				break;
			}
		}
		if(i >= app->ui->filter_types)
		{
			al_remove_config_key(t3f_config, section_buffer, "filter");
			clear = true;
		}
		strcpy(filter_buffer, "");
		for(i = 0; i < app->ui->filter_types; i++)
		{
			if(app->ui->filter_type_selected[i] && !(app->ui->filter_type_element[i]->flags & D_SELECTED))
			{
				changed = true;
			}
			else if(!app->ui->filter_type_selected[i] && (app->ui->filter_type_element[i]->flags & D_SELECTED))
			{
				changed = true;
			}
			if(app->ui->filter_type_element[i]->flags & D_SELECTED)
			{
				if(!first)
				{
					strcat(filter_buffer, ";");
				}
				strcat(filter_buffer, app->ui->filter_type_element[i]->dp);
				first = false;
			}
		}
		if(changed)
		{
			if(!clear)
			{
				al_set_config_value(t3f_config, section_buffer, "filter", filter_buffer);
			}
			omo_cancel_library_setup(app);
			omo_clear_library_cache();
			app->spawn_library_thread = true;
		}
		omo_close_filter_dialog(app->ui, app);
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_filter_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
