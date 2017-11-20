#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"

void omo_split_track_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char base_id[1024];
	char base_size[256];
	char id[1024];
	char fn[1024];
	char buf[256];
	char file_database_fn[1024];
	char entry_database_fn[1024];
	char * token;
	bool done = false;
	int current_track = 0;
	const char * title_val;
	char title_buf[1024];

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_split_track_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		if(strcmp(app->ui->original_split_track_text, app->ui->split_track_text))
		{
			/* copy track info string to entry database first, before breaking up the
			   track list to put into the file database */
	   		strcpy(base_id, app->ui->split_track_entry);
	   		base_id[32] = 0;
	   		sprintf(base_size, "%lu", t3f_file_size(app->ui->split_track_fn));
	   		strcat(base_id, base_size);
			al_set_config_value(app->library->entry_database, base_id, "Split Track Info", app->ui->split_track_text);
			title_val = al_get_config_value(app->library->entry_database, base_id, "Title");

			token = strtok(app->ui->split_track_text, ", ");
			while(!done)
			{
				if(!token)
				{
					if(current_track > 0)
					{
						sprintf(buf, "%d", current_track);
						al_set_config_value(app->library->file_database, app->ui->split_track_fn, "tracks", buf);
					}
					else
					{
						al_set_config_value(app->library->file_database, app->ui->split_track_fn, "tracks", "1");
						al_remove_config_key(app->library->file_database, app->ui->split_track_fn, "track_0");
					}
					break;
				}
				else
				{
					sprintf(fn, "%s:%s", app->ui->split_track_fn, token);
					sprintf(id, "%s%s", base_id, token);
					al_set_config_value(app->library->file_database, fn, "id", id);
					sprintf(buf, "track_%d", current_track);
					al_set_config_value(app->library->file_database, app->ui->split_track_fn, buf, token);
					if(title_val)
					{
						sprintf(title_buf, "%s: Track %d", title_val, current_track + 1);
						al_set_config_value(app->library->entry_database, id, "Title", title_buf);
						al_set_config_value(app->library->entry_database, id, "scanned", "1");
					}
					current_track++;
				}
				token = strtok(NULL, ", ");
			}
			omo_save_library(app->library);
			omo_clear_library_cache();
			strcpy(file_database_fn, app->library->file_database_fn);
			strcpy(entry_database_fn, app->library->entry_database_fn);
			omo_destroy_library(app->library);
			app->library = NULL;
			omo_setup_library(app, file_database_fn, entry_database_fn, NULL);
		}
		omo_close_split_track_dialog(app->ui, app);
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_split_track_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
