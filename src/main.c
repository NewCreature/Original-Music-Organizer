#include "t3f/t3f.h"
#include "t3f/file_utils.h"

#include "instance.h"
#include "defines.h"
#include "constants.h"
#include "library.h"
#include "player.h"
#include "file_chooser.h"
#include "ui/menu_init.h"
#include "ui/menu_proc.h"
#include "ui/dialog_proc.h"
#include "ui/ui.h"
#include "init.h"
#include "file_helpers.h"
#include "ui/queue_list.h"
#include "ui/library.h"
#include "ui/tags_dialog.h"
#include "ui/shortcut.h"

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
			omo_file_chooser_logic(data);
			omo_library_pre_gui_logic(data);
			t3gui_logic();
			if(!app->ui->tags_display)
			{
				if(app->library_view)
				{
					omo_library_logic(data);
				}
				omo_queue_list_logic(app);
				omo_shortcut_logic(app);
				if(app->library_view)
				{
					app->ui->ui_artist_list_element->id2 = app->ui->ui_artist_list_element->d1;
					app->ui->ui_album_list_element->id2 = app->ui->ui_album_list_element->d1;
				}
			}
			else
			{
				omo_tags_dialog_logic(data);
			}
			omo_player_logic(app->player, app->archive_handler_registry, app->codec_handler_registry);
			if(app->player->state == OMO_PLAYER_STATE_PLAYING || app->player->state == OMO_PLAYER_STATE_PAUSED)
			{
				app->ui->ui_queue_list_element->id2 = app->player->queue_pos;
			}
			else
			{
				app->ui->ui_queue_list_element->id2 = -1;
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
			t3gui_render();
			if(app->ui->tags_display)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->tags_display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			break;
		}
	}
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;

	if(omo_initialize(&app, argc, argv))
	{
		t3f_run();
		if(app.player)
		{
			omo_stop_player(app.player);
		}
		if(app.library)
		{
			omo_save_library(app.library);
		}
		omo_exit(&app);
		t3gui_exit();
	}
	else
	{
		printf("Error: could not initialize T3F!\n");
	}
	return 0;
}
