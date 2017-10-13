#include "../t3f/t3f.h"

#include "../instance.h"

static char ui_queue_text[1024] = {0};
static char ui_artist_text[1024] = {0};
static char ui_album_text[1024] = {0};
static char ui_song_text[1024] = {0};

static void get_path_filename(const char * fn, char * outfn)
{
	int i, j;
	int pos = 0;
	char path_separator;

	#ifdef ALLEGRO_WINDOWS
		path_separator = '\\';
	#else
		path_separator = '/';
	#endif
	for(i = strlen(fn) - 1; i >= 0; i--)
	{
		if(fn[i] == path_separator)
		{
			for(j = i + 1; j < strlen(fn); j++)
			{
				outfn[pos] = fn[j];
				pos++;
			}
			outfn[pos] = 0;
			break;
		}
	}
}

char * ui_queue_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char display_fn[256] = {0};
	char prefix[16] = {0};

    if(index < 0)
    {
        if(list_size && app->player)
        {
            *list_size = app->player->queue ? app->player->queue->entry_count : 0;
        }
        return NULL;
    }
    if(app->player && app->player->queue && index < app->player->queue->entry_count)
    {
		sprintf(prefix, "%s", index == app->player->queue_pos ? "" : "");
		if(strlen(app->player->queue->entry[index]->tags.track))
		{
			strcat(prefix, app->player->queue->entry[index]->tags.track);
			strcat(prefix, ". ");
		}
		if(strlen(app->player->queue->entry[index]->tags.artist) && strlen(app->player->queue->entry[index]->tags.album))
		{
			if(strlen(app->player->queue->entry[index]->tags.title))
			{
				sprintf(ui_queue_text, "%s%s - %s (%s)", prefix, app->player->queue->entry[index]->tags.artist, app->player->queue->entry[index]->tags.title, app->player->queue->entry[index]->tags.album);
			}
			else if(strlen(app->player->queue->entry[index]->tags.track))
			{
				sprintf(ui_queue_text, "%s%s - %s - Track %s", prefix,app->player->queue->entry[index]->tags.artist, app->player->queue->entry[index]->tags.album, app->player->queue->entry[index]->tags.track);
			}
			else
			{
				sprintf(ui_queue_text, "%s%s - %s - Unknown", prefix, app->player->queue->entry[index]->tags.artist, app->player->queue->entry[index]->tags.album);
			}
		}
		else if(strlen(app->player->queue->entry[index]->tags.album) && strlen(app->player->queue->entry[index]->tags.track))
		{
			sprintf(ui_queue_text, "%s%s - Track %s", prefix, app->player->queue->entry[index]->tags.album, app->player->queue->entry[index]->tags.track);
		}
		else if(strlen(app->player->queue->entry[index]->tags.title))
		{
			sprintf(ui_queue_text, "%s%s", prefix, app->player->queue->entry[index]->tags.title);
		}
        else
        {
            get_path_filename(app->player->queue->entry[index]->file, display_fn);
            sprintf(ui_queue_text, "%s%s", prefix, display_fn);
			if(app->player->queue->entry[index]->sub_file)
			{
				strcat(ui_queue_text, "/");
				strcat(ui_queue_text, app->player->queue->entry[index]->sub_file);
			}
			if(app->player->queue->entry[index]->track)
			{
				strcat(ui_queue_text, ":");
				strcat(ui_queue_text, app->player->queue->entry[index]->track);
			}
        }
		return ui_queue_text;
   }
   return NULL;
}

char * ui_artist_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(index < 0)
    {
        if(list_size && app->library && app->library->artist_entry)
        {
            *list_size = app->library->artist_entry_count;
        }
		else
		{
			*list_size = 1;
		}
        return NULL;
    }
    if(app->library && app->library->artist_entry)
    {
		sprintf(ui_artist_text, "%s", app->library->artist_entry[index]);
		return ui_artist_text;
	}
	strcpy(ui_artist_text, app->library_loading_message);
	return ui_artist_text;
}

char * ui_song_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * title;
	const char * album;
	const char * track;

    if(index < 0)
    {
        if(list_size && app->library && app->library->song_entry)
        {
            *list_size = app->library->song_entry_count + 1;
        }
        return NULL;
    }
    if(app->library && app->library->song_entry)
    {
		if(index == 0)
		{
			strcpy(ui_song_text, "Shuffle All");
		}
		else
		{
			index--;
			title = al_get_config_value(app->library->entry_database, app->library->entry[app->library->song_entry[index]]->id, "Title");
			album = al_get_config_value(app->library->entry_database, app->library->entry[app->library->song_entry[index]]->id, "Album");
			track = al_get_config_value(app->library->entry_database, app->library->entry[app->library->song_entry[index]]->id, "Track");
			if(title)
			{
				strcpy(ui_song_text, title);
			}
			else if(album && track)
			{
				sprintf(ui_song_text, "%s - Track %s", album, track);
			}
			else
			{
				strcpy(ui_song_text, app->library->entry[app->library->song_entry[index]]->filename);
			}
		}
		return ui_song_text;
   }
   return NULL;
}

char * ui_album_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(index < 0)
    {
        if(list_size && app->library && app->library->album_entry)
        {
            *list_size = app->library->album_entry_count;
        }
        return NULL;
    }
    if(app->library && app->library->album_entry)
    {
		sprintf(ui_album_text, "%s", app->library->album_entry[index]);
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
