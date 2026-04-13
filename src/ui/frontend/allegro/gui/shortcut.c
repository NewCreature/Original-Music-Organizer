#include "t3f/t3f.h"
#include "instance.h"
#include "file_chooser.h"
#include "init.h"
#include "constants.h"
#include "queue_helpers.h"
#include "menu_proc.h"
#include "ui.h"

static void omo_toggle_library_view(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	if(uip->app->library_view)
	{
		omo_menu_view_basic(0, uip);
	}
	else
	{
		omo_menu_view_library(0, uip);
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
	OMO_UI * uip = (OMO_UI *)data;
	bool restart_player = false;
	int i;

	if(t3f_key_pressed(ALLEGRO_KEY_L))
	{
		omo_toggle_library_view(uip->app);
		t3f_use_key_press(ALLEGRO_KEY_L);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_T) && uip->app->library)
	{
		uip->tags_queue_entry = -1;
		if(uip->app->player->queue && uip->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_playback_edit_tags(0, uip);
		}
		else if(uip->app->library_view && uip->ui_song_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_library_edit_tags(0, uip);
		}
		else if(uip->app->library_view && uip->ui_album_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_library_edit_album_tags(0, uip);
		}
		t3f_use_key_press(ALLEGRO_KEY_T);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_Z))
	{
		uip->app->button_pressed = 0;
		t3f_use_key_press(ALLEGRO_KEY_Z);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_X))
	{
		if(uip->app->player->queue_pos != uip->ui_queue_list_element->d1)
		{
			omo_stop_player(uip->app->player);
			uip->app->player->queue_pos = uip->ui_queue_list_element->d1;
			omo_start_player(uip->app->player);
		}
	}
	if(t3f_key_pressed(ALLEGRO_KEY_C))
	{
		if(uip->app->player->state == OMO_PLAYER_STATE_PLAYING)
		{
			omo_pause_player(uip->app->player);
		}
		else if(uip->app->player->state == OMO_PLAYER_STATE_PAUSED)
		{
			omo_resume_player(uip->app->player);
		}
		t3f_use_key_press(ALLEGRO_KEY_C);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_V))
	{
		omo_stop_player(uip->app->player);
		t3f_use_key_press(ALLEGRO_KEY_V);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_B))
	{
		uip->app->button_pressed = 3;
		t3f_use_key_press(ALLEGRO_KEY_B);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_DELETE))
	{
		if(uip->app->player->queue && uip->ui_queue_list_element->d1 < uip->app->player->queue->entry_count)
		{
			for(i = uip->app->player->queue->entry_count - 1; i >= 0; i--)
			{
				if(selected(uip->ui_queue_list_element, i))
				{
					if(i == uip->app->player->queue_pos)
					{
						if(uip->app->player->state == OMO_PLAYER_STATE_PLAYING)
						{
							restart_player = true;
						}
						omo_stop_player(uip->app->player);
					}
					omo_delete_queue_item(uip->app->player->queue, i);
					if(uip->app->player->queue_pos > i)
					{
						uip->app->player->queue_pos--;
					}
				}
			}
			free(uip->ui_queue_list_element->dp2);
			uip->ui_queue_list_element->dp2 = NULL;
			if(uip->app->player->queue->entry_count > 0)
			{
				uip->app->spawn_queue_thread = true;
				if(restart_player)
				{
					omo_start_player(uip->app->player);
				}
			}
			else
			{
				omo_destroy_queue(uip->app->player->queue);
				uip->app->player->queue = NULL;
			}
		}
		t3f_use_key_press(ALLEGRO_KEY_DELETE);
	}
	if(t3f_key_pressed(ALLEGRO_KEY_S))
	{
		if(uip->ui_artist_list_element->flags & D_GOTFOCUS || uip->ui_album_list_element->flags & D_GOTFOCUS || uip->ui_song_list_element->flags & D_GOTFOCUS)
		{
			uip->ui_song_list_element->d1 = 0;
			uip->ui_song_list_element->id1 = 0;
		}
		else if(uip->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			omo_menu_playback_shuffle(0, uip);
		}
		t3f_use_key_press(ALLEGRO_KEY_S);
	}
}
