#include "../t3f/t3f.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../cloud.h"
#include "dialog_proc.h"
#include "menu_proc.h"

void omo_tags_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool update_artists = false;
	bool update_albums = false;
	bool update_songs = false;
	bool update_tags = false;
	char title[256] = {0};
	const char * val;
	int i;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_tags_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if((t3f_key[ALLEGRO_KEY_LCTRL] || t3f_key[ALLEGRO_KEY_RCTRL] || t3f_key[ALLEGRO_KEY_COMMAND]) && t3f_key[ALLEGRO_KEY_C])
	{
		omo_menu_edit_copy_tags(data);
		t3f_key[ALLEGRO_KEY_C] = 0;
	}
	else if((t3f_key[ALLEGRO_KEY_LCTRL] || t3f_key[ALLEGRO_KEY_RCTRL] || t3f_key[ALLEGRO_KEY_COMMAND]) && t3f_key[ALLEGRO_KEY_V])
	{
		omo_menu_edit_paste_tags(data);
		t3f_key[ALLEGRO_KEY_V] = 0;
	}

	if(app->button_pressed == 0)
	{
		val = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(app->edit_artist, val);
		}
		val = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(app->edit_album, val);
		}
		val = ui_song_list_proc(app->ui->ui_song_list_element->d1, NULL, app);
		if(val)
		{
			strcpy(title, val);
		}
		strcpy(app->edit_song_id, app->ui->tags_entry);
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
				if(strlen(app->ui->tags_text[i]) == 0)
				{
					omo_remove_database_key(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
				}
				else
				{
					omo_set_database_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i], app->ui->tags_text[i]);
				}
				update_tags = true;
			}
		}
		if(update_tags)
		{
			omo_set_database_value(app->library->entry_database, app->ui->tags_entry, "Submitted", "false");
			app->spawn_cloud_thread = true;
		}
		omo_discard_entry_backup(app->library);
		omo_close_tags_dialog(app->ui, app);
		if(app->ui->tags_queue_entry >= 0)
		{
			omo_get_queue_entry_tags(app->player->queue, app->ui->tags_queue_entry, app->library);
		}
		else
		{
			app->spawn_queue_thread = true;
		}
		if(update_artists || update_albums || update_songs)
		{
			omo_clear_library_cache();
			app->spawn_library_lists_thread = true;
		}
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_restore_entry_tags(app->library);
		omo_close_tags_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
