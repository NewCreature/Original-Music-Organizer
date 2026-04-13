#include "instance.h"
#include "queue_helpers.h"
#include "dialog_proc.h"
#include "menu_proc.h"
#include "library_helpers.h"
#include "ui.h"

static int old_artist_d1 = -1;
static int old_album_d1 = -1;
static int old_artist_filter_length = -1;
static int old_album_filter_length = -1;
static int old_song_filter_length = -1;

void omo_library_pre_gui_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	if(uip->app->library_view)
	{
		old_artist_d1 = uip->ui_artist_list_element->d1;
		old_album_d1 = uip->ui_album_list_element->d1;
		old_artist_filter_length = strlen(uip->ui_artist_search_element->dp);
		old_album_filter_length = strlen(uip->ui_album_search_element->dp);
		old_song_filter_length = strlen(uip->ui_song_search_element->dp);
		if(uip->app->library && uip->app->library->loaded)
		{
			uip->ui_artist_search_element->flags &= ~D_DISABLED;
			uip->ui_album_search_element->flags &= ~D_DISABLED;
			uip->ui_song_search_element->flags &= ~D_DISABLED;
		}
		else
		{
			uip->ui_artist_search_element->flags |= D_DISABLED;
			uip->ui_album_search_element->flags |= D_DISABLED;
			uip->ui_song_search_element->flags |= D_DISABLED;
		}
	}
}

static void maybe_stop_player(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	if(!t3f_key_held(ALLEGRO_KEY_LSHIFT) && !t3f_key_held(ALLEGRO_KEY_RSHIFT))
	{
		switch(uip->app->player->state)
		{
			case OMO_PLAYER_STATE_PLAYING:
			case OMO_PLAYER_STATE_PAUSED:
			{
				omo_stop_player(uip->app->player);
				break;
			}
		}
	}
}

static void maybe_start_player(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	if(!t3f_key_held(ALLEGRO_KEY_LSHIFT) && !t3f_key_held(ALLEGRO_KEY_RSHIFT))
	{
		uip->app->player->queue_pos = 0;
		omo_start_player(uip->app->player);
	}
}

static bool prepare_queue(void * data, int count)
{
	OMO_UI * uip = (OMO_UI *)data;
	int final_count = count;

	if(t3f_key_held(ALLEGRO_KEY_LSHIFT) || t3f_key_held(ALLEGRO_KEY_RSHIFT))
	{
		if(uip->app->player->queue)
		{
			final_count += uip->app->player->queue->entry_count;
		}
	}
	if(uip->app->player->queue)
	{
		if(final_count == count)
		{
			omo_destroy_queue(uip->app->player->queue);
			uip->app->player->queue = NULL;
		}
		else if(!omo_resize_queue(&uip->app->player->queue, final_count))
		{
			return false;
		}
	}
	if(!uip->app->player->queue)
	{
		uip->app->player->queue = omo_create_queue(final_count);
	}
	if(uip->app->player->queue)
	{
		return true;
	}
	return false;
}

static void queue_song_list(void * data, OMO_LIBRARY * lp)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i;

	al_stop_timer(t3f_timer);
	maybe_stop_player(uip);
	if(prepare_queue(uip, lp->filtered_song_entry_count))
	{
		for(i = 0; i < lp->filtered_song_entry_count; i++)
		{
			omo_add_file_to_queue(uip->app->player->queue, lp->entry[lp->filtered_song_entry[i]]->filename, lp->entry[lp->filtered_song_entry[i]]->sub_filename, lp->entry[lp->filtered_song_entry[i]]->track, true);
		}
		maybe_start_player(uip);
		uip->app->spawn_queue_thread = true;
	}
	al_start_timer(t3f_timer);
}

void omo_library_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * val;
	const char * val2;
	const char * val3;
	int i;

	/* update library filter if user has typed into the search field */
	if(strlen(uip->ui_artist_search_element->dp) != old_artist_filter_length)
	{
		uip->apply_artist_search_filter = true;
		uip->ui_artist_list_element->d1 = 0;
		uip->ui_artist_list_element->d2 = 0;
		uip->ui_album_list_element->d1 = 0;
		uip->ui_album_list_element->d2 = 0;
		uip->ui_song_list_element->d1 = 0;
		uip->ui_song_list_element->d2 = 0;
	}
	if(strlen(uip->ui_album_search_element->dp) != old_album_filter_length)
	{
		uip->apply_album_search_filter = true;
		uip->ui_album_list_element->d1 = 0;
		uip->ui_album_list_element->d2 = 0;
		uip->ui_song_list_element->d1 = 0;
		uip->ui_song_list_element->d2 = 0;
	}
	if(strlen(uip->ui_song_search_element->dp) != old_song_filter_length)
	{
		uip->apply_song_search_filter = true;
		uip->ui_song_list_element->d1 = 0;
		uip->ui_song_list_element->d2 = 0;
	}

	uip->selected_song = uip->ui_song_list_element->d1 - 1;
	if(uip->ui_artist_list_element->d1 != old_artist_d1)
	{
		uip->ui_album_list_element->d1 = 0;
		uip->ui_album_list_element->d2 = 0;
		uip->ui_song_list_element->d1 = 0;
		uip->ui_song_list_element->d2 = 0;
		val2 = ui_artist_list_proc(uip->ui_artist_list_element->d1, NULL, NULL, uip->app);
		al_stop_timer(t3f_timer);
		omo_get_library_album_list(uip->app->library, val2);
		omo_get_library_song_list(uip->app->library, val2, "All Albums", NULL);
		uip->apply_album_search_filter = true;
		uip->apply_song_search_filter = true;
		al_start_timer(t3f_timer);
	}
	else if(uip->ui_album_list_element->d1 != old_album_d1)
	{
		uip->ui_song_list_element->d1 = 0;
		uip->ui_song_list_element->d2 = 0;
		val = ui_artist_list_proc(uip->ui_artist_list_element->d1, NULL, NULL, uip->app);
		if(uip->ui_album_list_element->d1 < 2)
		{
			val2 = ui_album_list_proc(uip->ui_album_list_element->d1, NULL, NULL, uip->app);
			val3 = NULL;
		}
		else
		{
			val2 = uip->app->library->album_entry[uip->ui_album_list_element->d1].name;
			val3 = uip->app->library->album_entry[uip->ui_album_list_element->d1].disambiguation;
		}
		al_stop_timer(t3f_timer);
		omo_get_library_song_list(uip->app->library, val, val2, val3);
		uip->apply_song_search_filter = true;
		al_start_timer(t3f_timer);
	}
	if(uip->ui_artist_list_element->id1 >= 0)
	{
		queue_song_list(uip, uip->app->library);
		uip->ui_artist_list_element->id1 = -1;
		if(!t3f_key_held(ALLEGRO_KEY_LSHIFT) && !t3f_key_held(ALLEGRO_KEY_RSHIFT))
		{
			uip->ui_queue_list_element->d1 = 0;
			uip->ui_queue_list_element->d2 = 0;
		}
	}
	else if(uip->ui_album_list_element->id1 >= 0)
	{
		queue_song_list(uip, uip->app->library);
		uip->ui_album_list_element->id1 = -1;
		if(!t3f_key_held(ALLEGRO_KEY_LSHIFT) && !t3f_key_held(ALLEGRO_KEY_RSHIFT))
		{
			uip->ui_queue_list_element->d1 = 0;
			uip->ui_queue_list_element->d2 = 0;
		}
	}
	else if(uip->ui_song_list_element->id1 >= 0)
	{
		al_stop_timer(t3f_timer);
		maybe_stop_player(uip->app);
		if(prepare_queue(uip->app, uip->selected_song < 0 ? uip->app->library->filtered_song_entry_count : 1))
		{
			if(uip->selected_song < 0)
			{
				for(i = 0; i < uip->app->library->filtered_song_entry_count; i++)
				{
					omo_add_file_to_queue(uip->app->player->queue, uip->app->library->entry[uip->app->library->filtered_song_entry[i]]->filename, uip->app->library->entry[uip->app->library->filtered_song_entry[i]]->sub_filename, uip->app->library->entry[uip->app->library->filtered_song_entry[i]]->track, true);
				}
				omo_menu_playback_shuffle(0, uip->app);
			}
			else
			{
				omo_add_file_to_queue(uip->app->player->queue, uip->app->library->entry[uip->app->library->filtered_song_entry[uip->selected_song]]->filename, uip->app->library->entry[uip->app->library->filtered_song_entry[uip->selected_song]]->sub_filename, uip->app->library->entry[uip->app->library->filtered_song_entry[uip->selected_song]]->track, true);
			}
			if(!t3f_key_held(ALLEGRO_KEY_LSHIFT) && !t3f_key_held(ALLEGRO_KEY_RSHIFT))
			{
				uip->ui_queue_list_element->d1 = 0;
				uip->ui_queue_list_element->d2 = 0;
			}
			uip->app->spawn_queue_thread = true;
			maybe_start_player(uip->app);
		}
		uip->ui_song_list_element->id1 = -1;
		al_start_timer(t3f_timer);
	}

	if(uip->apply_artist_search_filter)
	{
		omo_filter_library_artist_list(uip->app->library, uip->ui_artist_search_element->dp);
		uip->apply_artist_search_filter = false;
	}
	if(uip->apply_album_search_filter)
	{
		omo_filter_library_album_list(uip->app->library, uip->ui_album_search_element->dp);
		uip->apply_album_search_filter = false;
	}
	if(uip->apply_song_search_filter)
	{
		omo_filter_library_song_list(uip->app->library, uip->ui_song_search_element->dp);
		uip->apply_song_search_filter = false;
	}

	if(t3f_key_pressed(ALLEGRO_KEY_ENTER))
	{
		if(uip->ui_artist_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_artist_search_element->dp))
			{
				t3gui_set_focus_element(uip->ui_artist_list_element);
				uip->ui_artist_list_element->d1 = 2;
			}
		}
		else if(uip->ui_album_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_album_search_element->dp))
			{
				t3gui_set_focus_element(uip->ui_album_list_element);
				uip->ui_album_list_element->d1 = 2;
			}
		}
		else if(uip->ui_song_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_song_search_element->dp))
			{
				t3gui_set_focus_element(uip->ui_song_list_element);
				uip->ui_song_list_element->d1 = 1;
			}
		}
		t3f_use_key_press(ALLEGRO_KEY_ENTER);
	}
	else if(t3f_key_pressed(ALLEGRO_KEY_TAB))
	{
		if(uip->ui_artist_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_artist_search_element->dp))
			{
				uip->ui_artist_list_element->d1 = 2;
			}
		}
		else if(uip->ui_album_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_album_search_element->dp))
			{
				uip->ui_album_list_element->d1 = 2;
			}
		}
		else if(uip->ui_song_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(uip->ui_song_search_element->dp))
			{
				uip->ui_song_list_element->d1 = 1;

			}
		}
	}
}
