#include "t3f/t3f.h"
#include "t3f/file_utils.h"

#include "instance.h"
#include "defines.h"
#include "constants.h"
#include "library.h"
#include "player.h"
#include "ui/frontend/allegro/gui/file_chooser.h"
#include "init.h"
#include "file_helpers.h"
#include "test.h"
#include "queue_helpers.h"
#include "library_helpers.h"
#include "threads.h"
#include "ui/frontend/allegro/gui/library.h"

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	t3f_refresh_menus();
	if(app->test_mode >= 0)
	{
		if(!omo_test_logic(app))
		{
			printf("Testing ended prematurely!\n");
			t3f_exit();
		}
	}
	switch(app->state)
	{
		default:
		{
			omo_player_logic(app->player, app->library, app->archive_handler_registry, app->codec_handler_registry, app->player_temp_path);
			if(app->player->new_tags)
			{
				omo_spawn_cloud_thread(app);
				app->player->new_tags = false;
			}
			app->frontend->logic(app->frontend->data, 0);

			omo_threads_logic(app);
			if(app->library && !app->library_thread)
			{
				strcpy(app->status_bar_text, "Library ready.");
			}
			if(app->library && app->library_thread && !app->cloud_thread)
			{
				strcpy(app->status_bar_text, "Generating library lists...");
			}
		}
	}
}

/* main rendering routine */
void omo_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
			app->frontend->render(app->frontend->data, 0);
			break;
		}
	}
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;
	char buf[256];

	if(omo_initialize(&app, argc, argv))
	{
		t3f_run();
		al_remove_config_key(t3f_config, "Settings", "queue_track_position");
		al_remove_config_key(t3f_config, "Settings", "player_state");
		if(app.player)
		{
			if(app.player->state == OMO_PLAYER_STATE_PLAYING || app.player->state == OMO_PLAYER_STATE_PAUSED)
			{
				if(app.player->track && app.player->track->codec_handler->get_position)
				{
					sprintf(buf, "%f", app.player->track->codec_handler->get_position(app.player->track->codec_data));
					al_set_config_value(t3f_config, "Settings", "queue_track_position", buf);
					sprintf(buf, "%d", app.player->state);
					al_set_config_value(t3f_config, "Settings", "player_state", buf);
				}
			}
			omo_stop_player(app.player);
		}
		if(app.library)
		{
			omo_save_library(app.library);
		}
		omo_exit(&app);
		t3gui_exit();
		t3f_finish();
	}
	return 0;
}
