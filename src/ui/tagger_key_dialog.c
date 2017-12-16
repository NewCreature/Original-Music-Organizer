#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3net/t3net.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"

void omo_tagger_key_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	T3NET_ARGUMENTS * key_arguments;
	T3NET_DATA * key_data;
	const char * key_val;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_tagger_key_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		if(strlen(app->ui->tagger_key_text))
		{
			if(strcmp(app->ui->original_tagger_key_text, app->ui->tagger_key_text))
			{
				key_arguments = t3net_create_arguments();
				if(key_arguments)
				{
					/* copy track info string to entry database first, before breaking up the
					   track list to put into the file database */
					t3net_add_argument(key_arguments, "name", app->ui->tagger_key_text);
					key_data = t3net_get_data("http://www.t3-i.com/omo/get_tagger_key.php", key_arguments);
					if(key_data)
					{
						key_val = t3net_get_data_entry_field(key_data, 0, "tagger_key");
						if(key_val)
						{
							al_set_config_value(t3f_config, "Settings", "Tagger Name", app->ui->tagger_key_text);
							al_set_config_value(t3f_config, "Settings", "Tagger ID", key_val);
							t3f_save_config();
						}
						t3net_destroy_data(key_data);
					}
					t3net_destroy_arguments(key_arguments);
				}
			}
		}
		omo_close_tagger_key_dialog(app->ui, app);
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_tagger_key_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
