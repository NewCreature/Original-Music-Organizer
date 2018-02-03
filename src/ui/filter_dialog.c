#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3net/t3net.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../profile.h"
#include "menu_init.h"

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
