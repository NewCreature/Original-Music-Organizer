#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"

void omo_split_track_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char file_database_fn[1024];
	char entry_database_fn[1024];

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_split_track_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		if(strcmp(app->ui->original_split_track_text, app->ui->split_track_text))
		{
			omo_split_track(app->library, app->ui->split_track_fn, app->ui->split_track_text);
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
