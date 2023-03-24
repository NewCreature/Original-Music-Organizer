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
		omo_menu_view_basic(0, app);
	}
	else
	{
		omo_menu_view_library(0, app);
	}
}

static bool selected(T3GUI_ELEMENT * ep, int entry)
{
	char * dp2 = ep->dp2;

	if(dp2)
	{
		return dp2[entry];
	}
	else
	{
		if(ep->d1 == entry)
		{
			return true;
		}
	}
	return false;
}

void omo_shortcut_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool restart_player = false;
	int i;

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
			omo_menu_playback_edit_tags(0, app);
		}
		else if(app->library_view && app->ui->ui_song_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_library_edit_tags(0, app);
		}
		else if(app->library_view && app->ui->ui_album_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_library_edit_album_tags(0, app);
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
			for(i = app->player->queue->entry_count - 1; i >= 0; i--)
			{
				if(selected(app->ui->ui_queue_list_element, i))
				{
					if(i == app->player->queue_pos)
					{
						if(app->player->state == OMO_PLAYER_STATE_PLAYING)
						{
							restart_player = true;
						}
						omo_stop_player(app->player);
					}
					omo_delete_queue_item(app->player->queue, i);
					if(app->player->queue_pos > i)
					{
						app->player->queue_pos--;
					}
				}
			}
			free(app->ui->ui_queue_list_element->dp2);
			app->ui->ui_queue_list_element->dp2 = NULL;
			if(app->player->queue->entry_count > 0)
			{
				app->spawn_queue_thread = true;
				if(restart_player)
				{
					omo_start_player(app->player);
				}
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
		if(app->ui->ui_artist_list_element->flags & D_GOTFOCUS || app->ui->ui_album_list_element->flags & D_GOTFOCUS || app->ui->ui_song_list_element->flags & D_GOTFOCUS)
		{
			app->ui->ui_song_list_element->d1 = 0;
			app->ui->ui_song_list_element->id1 = 0;
		}
		else if(app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_playback_shuffle(0, app);
		}
		t3f_key[ALLEGRO_KEY_S] = 0;
	}
}
