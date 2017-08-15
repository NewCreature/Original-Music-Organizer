#include "../t3f/t3f.h"

#include "../instance.h"

static char ui_queue_text[1024] = {0};
static char ui_artist_text[1024] = {0};
static char ui_album_text[1024] = {0};

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
    char section[1024] = {0};
    char display_fn[256] = {0};
	char prefix[16] = {0};
    const char * val = NULL;
	const char * artist = NULL;
	const char * album = NULL;
	const char * title = NULL;
	const char * track = NULL;

    if(index < 0)
    {
        if(list_size && app->player)
        {
            *list_size = app->player->queue ? app->player->queue->entry_count : 0;
        }
        return NULL;
    }
    if(app->player && app->player->queue)
    {
        sprintf(section, "%s%s%s", app->player->queue->entry[index]->file, app->player->queue->entry[index]->sub_file ? "/" : "", app->player->queue->entry[index]->sub_file ? app->player->queue->entry[index]->sub_file : "");
        if(app->library)
        {
            val = al_get_config_value(app->library->file_database, section, "id");
            if(val)
            {
				artist = al_get_config_value(app->library->entry_database, val, "Artist");
				album = al_get_config_value(app->library->entry_database, val, "Album");
                title = al_get_config_value(app->library->entry_database, val, "Title");
				track = al_get_config_value(app->library->entry_database, val, "Track");
            }
        }
		sprintf(prefix, "%s", index == app->player->queue_pos ? ">" : " ");
		if(track)
		{
			strcat(prefix, track);
			strcat(prefix, ". ");
		}
        if(title)
        {
			if(artist && album)
			{
            	sprintf(ui_queue_text, "%s%s - %s (%s)", prefix, artist, title, album);
			}
			else if(artist)
			{
				sprintf(ui_queue_text, "%s%s - %s", prefix, artist, title);
			}
			else if(album)
			{
				sprintf(ui_queue_text, "%s%s - %s", prefix, album, title);
			}
			else
			{
				sprintf(ui_queue_text, "%s%s", prefix, title);
			}
        }
        else
        {
            get_path_filename(app->player->queue->entry[index]->file, display_fn);
            sprintf(ui_queue_text, "%s%s%s%s", prefix, display_fn, app->player->queue->entry[index]->sub_file ? "/" : "", app->player->queue->entry[index]->sub_file ? app->player->queue->entry[index]->sub_file : "");
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
        return NULL;
    }
    if(app->library && app->library->artist_entry)
    {
		sprintf(ui_artist_text, "%s", app->library->artist_entry[index]);
		return ui_artist_text;
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
