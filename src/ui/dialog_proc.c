#include "../t3f/t3f.h"

#include "../instance.h"
#include "../library_helpers.h"
#include "../text_helpers.h"
#include "queue_list.h"

static char buffer[1024] = {0};
static char ui_artist_text[1024] = {0};
static char ui_album_text[1024] = {0};
static char ui_song_text[1024] = {0};

char * ui_queue_list_proc(int index, int *list_size, bool * multi, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(index < 0)
	{
		if(list_size && app->player)
		{
			*list_size = app->player->queue ? app->player->queue->entry_count : 0;
		}
		if(multi)
		{
			*multi = true;
		}
		return NULL;
	}
	if(app->player && app->player->queue && index < app->player->queue->entry_count)
	{
		return omo_get_queue_item_text(app->player->queue, index, buffer);
	}
	return NULL;
}

char * ui_artist_list_proc(int index, int *list_size, bool * multi, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(index < 0)
	{
		if(list_size && app->library && app->library->loaded && app->library->artist_entry)
		{
			*list_size = app->library->filtered_artist_entry_count;
		}
		else
		{
			*list_size = 0;
		}
		if(multi)
		{
			*multi = false;
		}
		return NULL;
	}
	if(app->library && app->library->loaded && app->library->artist_entry)
	{
		sprintf(ui_artist_text, "%s", app->library->filtered_artist_entry[index]);
		return ui_artist_text;
	}
	return ui_artist_text;
}

char * ui_song_list_proc(int index, int *list_size, bool * multi, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * title;
	const char * album;
	const char * track;
	char buf[64];

	if(index < 0)
	{
		if(list_size && app->library && app->library->loaded && app->library->song_entry)
		{
			*list_size = app->library->filtered_song_entry_count + 1;
		}
		if(multi)
		{
			*multi = false;
		}
		return NULL;
	}
	if(app->library && app->library->loaded && app->library->song_entry)
	{
		if(index == 0)
		{
			sprintf(ui_song_text, "Shuffle All (%d Track%s)", app->library->filtered_song_entry_count, app->library->filtered_song_entry_count != 1 ? "s" : "");
		}
		else
		{
			index--;
			title = omo_get_database_value(app->library->entry_database, app->library->entry[app->library->filtered_song_entry[index]]->id, "Title");
			album = omo_get_database_value(app->library->entry_database, app->library->entry[app->library->filtered_song_entry[index]]->id, "Album");
			track = omo_get_database_value(app->library->entry_database, app->library->entry[app->library->filtered_song_entry[index]]->id, "Track");
			if(title)
			{
				strcpy(ui_song_text, strlen(title) ? title : "No Title");
			}
			else if(album && track)
			{
				sprintf(ui_song_text, "%s - Track %s", album, track);
			}
			else
			{
				strcpy(ui_song_text, app->library->entry[app->library->filtered_song_entry[index]]->filename);
			}
			strcat(ui_song_text, "\t");
			strcat(ui_song_text, omo_sec_to_clock(omo_get_library_entry_length(app->library, app->library->entry[app->library->filtered_song_entry[index]]->id) + 0.5, buf, 64));
		}
		return ui_song_text;
   }
   return NULL;
}

char * ui_album_list_proc(int index, int *list_size, bool * multi, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(index < 0)
	{
		if(list_size && app->library && app->library->loaded && app->library->album_entry)
		{
			*list_size = app->library->filtered_album_entry_count;
		}
		if(multi)
		{
			*multi = false;
		}
		return NULL;
	}
	if(app->library && app->library->loaded && app->library->album_entry)
	{
		sprintf(ui_album_text, "%s", app->library->filtered_album_entry[index]);
		return ui_album_text;
   }
   return NULL;
}

int ui_player_button_proc(T3GUI_ELEMENT * ep, void * dp3)
{
	APP_INSTANCE * app = (APP_INSTANCE *)dp3;

	app->button_pressed = ep->d2;
	return 0;
}

int ui_tags_button_proc(T3GUI_ELEMENT * ep, void * dp3)
{
	APP_INSTANCE * app = (APP_INSTANCE *)dp3;

	app->button_pressed = ep->d2;
	return 0;
}
