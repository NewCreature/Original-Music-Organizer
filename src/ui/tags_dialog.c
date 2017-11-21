#include "../t3f/t3f.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "dialog_proc.h"

void omo_tags_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool update_artists = false;
	bool update_albums = false;
	bool update_songs = false;
	char artist[256] = {0};
	char album[256] = {0};
	char title[256] = {0};
	char id[256] = {0};
	const char * val;
	int i;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_tags_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		val = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(artist, val);
		}
		val = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(album, val);
		}
		val = ui_song_list_proc(app->ui->ui_song_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(title, val);
		}
		strcpy(id, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1 - 1]]->id);
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(omo_tag_type[i] && strcmp(app->ui->tags_text[i], app->ui->original_tags_text[i]))
			{
				if(!strcmp(omo_tag_type[i], "Artist"))
				{
					update_artists = true;
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Album"))
				{
					update_albums = true;
					update_songs = true;
				}
				else if(!strcmp(omo_tag_type[i], "Title"))
				{
					update_songs = true;
				}
				al_set_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i], app->ui->tags_text[i]);
			}
		}
		omo_close_tags_dialog(app->ui, app);
		if(app->ui->tags_queue_entry >= 0)
		{
			omo_get_queue_entry_tags(app->player->queue, app->ui->tags_queue_entry, app->library);
		}
		else
		{
			omo_get_queue_tags(app->player->queue, app->library, data);
		}
		if(update_artists)
		{
			omo_build_library_artists_list(app, app->library);
			for(i = 0; i < app->library->artist_entry_count; i++)
			{
				if(!strcmp(app->library->artist_entry[i], artist))
				{
					app->ui->ui_artist_list_element->d1 = i;
					break;
				}
			}
		}
		if(update_songs)
		{
			omo_get_library_song_list(app->library, artist, album);
			for(i = 0; i < app->library->song_entry_count; i++)
			{
				if(!strcmp(app->library->entry[app->library->song_entry[i]]->id, id))
				{
					app->ui->ui_song_list_element->d1 = i + 1;
					break;
				}
			}
		}
		if(update_albums)
		{
			for(i = 0; i < app->library->album_entry_count; i++)
			{
				if(!strcmp(app->library->album_entry[i], album))
				{
					app->ui->ui_album_list_element->d1 = i;
					break;
				}
			}
			if(i == app->library->album_entry_count)
			{
				val = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
				if(val)
				{
					strcpy(album, val);
				}
				omo_get_library_song_list(app->library, artist, album);
			}
		}
		if(update_artists || update_albums || update_songs)
		{
			omo_clear_library_cache();
		}
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_tags_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
