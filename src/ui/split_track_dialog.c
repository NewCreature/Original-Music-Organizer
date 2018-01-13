#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../cloud.h"

void omo_split_track_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

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
			omo_set_database_value(app->library->entry_database, app->ui->split_track_entry, "Submitted", "false");
			if(omo_submit_track_tags(app->library, app->ui->split_track_entry, "http://www.t3-i.com/omo/tag_track.php", app->archive_handler_registry, app->codec_handler_registry, app->cloud_temp_path))
			{
				omo_set_database_value(app->library->entry_database, app->ui->split_track_entry, "Submitted", "true");
			}
			omo_save_library(app->library);
			omo_clear_library_cache();
			app->spawn_library_thread = true;
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
