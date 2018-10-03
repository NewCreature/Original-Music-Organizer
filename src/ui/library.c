#include "../instance.h"
#include "../queue_helpers.h"
#include "dialog_proc.h"
#include "menu_proc.h"
#include "../library_helpers.h"

static int old_artist_d1 = -1;
static int old_album_d1 = -1;
static int old_artist_filter_length = -1;
static int old_album_filter_length = -1;
static int old_song_filter_length = -1;

void omo_library_pre_gui_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->library_view)
	{
		old_artist_d1 = app->ui->ui_artist_list_element->d1;
		old_album_d1 = app->ui->ui_album_list_element->d1;
		old_artist_filter_length = strlen(app->ui->ui_artist_search_element->dp);
		old_album_filter_length = strlen(app->ui->ui_album_search_element->dp);
		old_song_filter_length = strlen(app->ui->ui_song_search_element->dp);
		if(app->library && app->library->loaded)
		{
			app->ui->ui_artist_search_element->flags &= ~D_DISABLED;
			app->ui->ui_album_search_element->flags &= ~D_DISABLED;
			app->ui->ui_song_search_element->flags &= ~D_DISABLED;
		}
		else
		{
			app->ui->ui_artist_search_element->flags |= D_DISABLED;
			app->ui->ui_album_search_element->flags |= D_DISABLED;
			app->ui->ui_song_search_element->flags |= D_DISABLED;
		}
	}
}

static void maybe_stop_player(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!t3f_key[ALLEGRO_KEY_LSHIFT] && !t3f_key[ALLEGRO_KEY_RSHIFT])
	{
		switch(app->player->state)
		{
			case OMO_PLAYER_STATE_PLAYING:
			case OMO_PLAYER_STATE_PAUSED:
			{
				omo_stop_player(app->player);
				break;
			}
		}
	}
}

static void maybe_start_player(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!t3f_key[ALLEGRO_KEY_LSHIFT] && !t3f_key[ALLEGRO_KEY_RSHIFT])
	{
		app->player->queue_pos = 0;
		omo_start_player(app->player);
	}
}

static bool prepare_queue(void * data, int count)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int final_count = count;

	if(t3f_key[ALLEGRO_KEY_LSHIFT] || t3f_key[ALLEGRO_KEY_RSHIFT])
	{
		if(app->player->queue)
		{
			final_count += app->player->queue->entry_count;
		}
	}
	if(app->player->queue)
	{
		if(final_count == count)
		{
			omo_destroy_queue(app->player->queue);
			app->player->queue = NULL;
		}
		else if(!omo_resize_queue(&app->player->queue, final_count))
		{
			return false;
		}
	}
	if(!app->player->queue)
	{
		app->player->queue = omo_create_queue(final_count);
	}
	if(app->player->queue)
	{
		return true;
	}
	return false;
}

static void queue_song_list(void * data, OMO_LIBRARY * lp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	al_stop_timer(t3f_timer);
	maybe_stop_player(app);
	if(prepare_queue(app, lp->filtered_song_entry_count))
	{
		for(i = 0; i < lp->filtered_song_entry_count; i++)
		{
			omo_add_file_to_queue(app->player->queue, lp->entry[lp->filtered_song_entry[i]]->filename, lp->entry[lp->filtered_song_entry[i]]->sub_filename, lp->entry[lp->filtered_song_entry[i]]->track, true);
		}
		maybe_start_player(app);
		app->spawn_queue_thread = true;
	}
	al_start_timer(t3f_timer);
}

void omo_library_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;
	const char * val2;
	int i;

	/* update library filter if user has typed into the search field */
	if(strlen(app->ui->ui_artist_search_element->dp) != old_artist_filter_length)
	{
		omo_filter_library_artist_list(app->library, app->ui->ui_artist_search_element->dp);
		app->ui->ui_artist_list_element->d1 = 0;
		app->ui->ui_artist_list_element->d2 = 0;
		app->ui->ui_album_list_element->d1 = 0;
		app->ui->ui_album_list_element->d2 = 0;
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
	}
	if(strlen(app->ui->ui_album_search_element->dp) != old_album_filter_length)
	{
		omo_filter_library_album_list(app->library, app->ui->ui_album_search_element->dp);
		app->ui->ui_album_list_element->d1 = 0;
		app->ui->ui_album_list_element->d2 = 0;
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
	}
	if(strlen(app->ui->ui_song_search_element->dp) != old_song_filter_length)
	{
		omo_filter_library_song_list(app->library, app->ui->ui_song_search_element->dp);
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
	}

	if(t3f_key[ALLEGRO_KEY_ENTER])
	{
		if(app->ui->ui_artist_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_artist_search_element->dp))
			{
				t3gui_set_focus_element(app->ui->ui_artist_list_element);
				app->ui->ui_artist_list_element->d1 = 2;
			}
		}
		else if(app->ui->ui_album_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_album_search_element->dp))
			{
				t3gui_set_focus_element(app->ui->ui_album_list_element);
				app->ui->ui_album_list_element->d1 = 2;
			}
		}
		else if(app->ui->ui_song_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_song_search_element->dp))
			{
				t3gui_set_focus_element(app->ui->ui_song_list_element);
				app->ui->ui_song_list_element->d1 = 1;
			}
		}
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(t3f_key[ALLEGRO_KEY_TAB])
	{
		if(app->ui->ui_artist_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_artist_search_element->dp))
			{
				app->ui->ui_artist_list_element->d1 = 2;
			}
		}
		else if(app->ui->ui_album_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_album_search_element->dp))
			{
				app->ui->ui_album_list_element->d1 = 2;
			}
		}
		else if(app->ui->ui_song_search_element->flags & D_GOTFOCUS)
		{
			if(strlen(app->ui->ui_song_search_element->dp))
			{
				app->ui->ui_song_list_element->d1 = 1;

			}
		}
	}

	app->ui->selected_song = app->ui->ui_song_list_element->d1 - 1;
	if(app->ui->ui_artist_list_element->d1 != old_artist_d1)
	{
		app->ui->ui_album_list_element->d1 = 0;
		app->ui->ui_album_list_element->d2 = 0;
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
		val2 = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
		al_stop_timer(t3f_timer);
		omo_get_library_album_list(app->library, val2);
		omo_get_library_song_list(app->library, val2, "All Albums");
		al_start_timer(t3f_timer);
	}
	else if(app->ui->ui_album_list_element->d1 != old_album_d1)
	{
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
		val = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
		val2 = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
		al_stop_timer(t3f_timer);
		omo_get_library_song_list(app->library, val, val2);
		al_start_timer(t3f_timer);
	}
	if(app->ui->ui_artist_list_element->id1 >= 0)
	{
		queue_song_list(app, app->library);
		app->ui->ui_artist_list_element->id1 = -1;
		if(!(t3f_key[ALLEGRO_KEY_LSHIFT] || t3f_key[ALLEGRO_KEY_RSHIFT]))
		{
			app->ui->ui_queue_list_element->d1 = 0;
			app->ui->ui_queue_list_element->d2 = 0;
		}
	}
	else if(app->ui->ui_album_list_element->id1 >= 0)
	{
		queue_song_list(app, app->library);
		app->ui->ui_album_list_element->id1 = -1;
		if(!(t3f_key[ALLEGRO_KEY_LSHIFT] || t3f_key[ALLEGRO_KEY_RSHIFT]))
		{
			app->ui->ui_queue_list_element->d1 = 0;
			app->ui->ui_queue_list_element->d2 = 0;
		}
	}
	else if(app->ui->ui_song_list_element->id1 >= 0)
	{
		al_stop_timer(t3f_timer);
		maybe_stop_player(app);
		if(prepare_queue(app, app->ui->selected_song < 0 ? app->library->filtered_song_entry_count : 1))
		{
			if(app->ui->selected_song < 0)
			{
				for(i = 0; i < app->library->filtered_song_entry_count; i++)
				{
					omo_add_file_to_queue(app->player->queue, app->library->entry[app->library->filtered_song_entry[i]]->filename, app->library->entry[app->library->filtered_song_entry[i]]->sub_filename, app->library->entry[app->library->filtered_song_entry[i]]->track, true);
				}
				omo_menu_playback_shuffle(0, app);
			}
			else
			{
				omo_add_file_to_queue(app->player->queue, app->library->entry[app->library->filtered_song_entry[app->ui->selected_song]]->filename, app->library->entry[app->library->filtered_song_entry[app->ui->selected_song]]->sub_filename, app->library->entry[app->library->filtered_song_entry[app->ui->selected_song]]->track, true);
			}
			if(!(t3f_key[ALLEGRO_KEY_LSHIFT] || t3f_key[ALLEGRO_KEY_RSHIFT]))
			{
				app->ui->ui_queue_list_element->d1 = 0;
				app->ui->ui_queue_list_element->d2 = 0;
			}
			app->spawn_queue_thread = true;
			maybe_start_player(app);
		}
		app->ui->ui_song_list_element->id1 = -1;
		al_start_timer(t3f_timer);
	}
}
