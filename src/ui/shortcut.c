#include "../t3f/t3f.h"
#include "../instance.h"
#include "../file_chooser.h"
#include "../init.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "menu_proc.h"

static void omo_toggle_library_view(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	if(app->library_view)
	{
		omo_menu_view_basic(app);
	}
	else
	{
		omo_menu_view_library(app);
	}
}

void omo_shortcut_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(t3f_key[ALLEGRO_KEY_L])
	{
		omo_toggle_library_view(app);
		t3f_key[ALLEGRO_KEY_L] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_T] && app->library)
	{
		app->ui->tags_queue_entry = -1;
		if(app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_playback_edit_tags(app);
		}
		else if(app->ui->ui_song_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_library_edit_tags(app);
		}
		t3f_key[ALLEGRO_KEY_T] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_Z])
	{
		app->button_pressed = 0;
		t3f_key[ALLEGRO_KEY_Z] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_X])
	{
		if(app->player->queue_pos != app->ui->ui_queue_list_element->d1)
		{
			omo_stop_player(app->player);
			app->player->queue_pos = app->ui->ui_queue_list_element->d1;
			omo_start_player(app->player);
		}
	}
	if(t3f_key[ALLEGRO_KEY_C])
	{
		if(app->player->state == OMO_PLAYER_STATE_PLAYING)
		{
			omo_pause_player(app->player);
		}
		else if(app->player->state == OMO_PLAYER_STATE_PAUSED)
		{
			omo_resume_player(app->player);
		}
		t3f_key[ALLEGRO_KEY_C] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_V])
	{
		omo_stop_player(app->player);
		t3f_key[ALLEGRO_KEY_V] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_B])
	{
		app->button_pressed = 3;
		t3f_key[ALLEGRO_KEY_B] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_DELETE])
	{
		if(app->player->queue && app->ui->ui_queue_list_element->d1 < app->player->queue->entry_count)
		{
			if(app->ui->ui_queue_list_element->d1 == app->player->queue_pos)
			{
				omo_stop_player(app->player);
			}
			omo_delete_queue_item(app->player->queue, app->ui->ui_queue_list_element->d1);
			if(app->player->queue_pos > app->ui->ui_queue_list_element->d1)
			{
				app->player->queue_pos--;
			}
			if(app->player->queue->entry_count > 0)
			{
				omo_get_queue_tags(app->player->queue, app->library, app);
				omo_start_player(app->player);
			}
			else
			{
				omo_destroy_queue(app->player->queue);
				app->player->queue = NULL;
			}
		}
		t3f_key[ALLEGRO_KEY_DELETE] = 0;
	}
	if(t3f_key[ALLEGRO_KEY_S])
	{
		omo_menu_playback_shuffle(app);
		t3f_key[ALLEGRO_KEY_S] = 0;
	}
}
