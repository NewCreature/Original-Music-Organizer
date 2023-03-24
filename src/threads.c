#include "instance.h"
#include "library_helpers.h"
#include "queue_helpers.h"
#include "cloud.h"

void omo_spawn_cloud_thread(APP_INSTANCE * app)
{
	if(!app->disable_cloud_syncing)
	{
		app->spawn_cloud_thread = true;
	}
}

static void kill_all_threads(APP_INSTANCE * app)
{
	if(app->player->queue && app->player->queue->thread)
	{
		al_destroy_thread(app->player->queue->thread);
		app->player->queue->thread = NULL;
	}
	if(app->cloud_thread)
	{
		al_destroy_thread(app->cloud_thread);
		app->cloud_thread = NULL;
	}
	omo_cancel_library_setup(app);
}

void omo_threads_logic(APP_INSTANCE * app)
{
	char file_database_fn[1024];
	char entry_database_fn[1024];
	char buffer[1024];
	int i;
	const char * script_url;

	/* destroy thread when library scan finished */
	if(app->library_thread && app->loading_library_file_helper_data.scan_done)
	{
		al_destroy_thread(app->library_thread);
		app->library_thread = NULL;
		if(app->library && app->player->queue)
		{
			app->spawn_queue_thread = true;
		}
		if(app->library)
		{
			if(app->library->loaded)
			{
				app->ui->apply_artist_search_filter = true;
				app->ui->apply_album_search_filter = true;
				app->ui->apply_song_search_filter = true;
				for(i = 0; i < app->library->artist_entry_count; i++)
				{
					if(!strcmp(app->library->artist_entry[i], app->edit_artist))
					{
						app->ui->ui_artist_list_element->d1 = i;
						break;
					}
				}
				for(i = 0; i < app->library->song_entry_count; i++)
				{
					if(!strcmp(app->library->entry[app->library->song_entry[i]]->id, app->edit_song_id))
					{
						app->ui->ui_song_list_element->d1 = i + 1;
						break;
					}
				}
				for(i = 0; i < app->library->album_entry_count; i++)
				{
					if(!strcmp(app->library->album_entry[i].name, app->edit_album) && (app->library->album_entry[i].disambiguation ? !strcmp(app->library->album_entry[i].disambiguation, app->edit_disambiguation) : 1))
					{
						app->ui->ui_album_list_element->d1 = i;
						break;
					}
				}
			}
			else
			{
				strcpy(app->edit_artist, "");
				strcpy(app->edit_album, "");
				strcpy(app->edit_song_id, "");
				app->spawn_library_lists_thread = true;
			}
		}
	}
	if(app->cloud_thread_done)
	{
		al_destroy_thread(app->cloud_thread);
		app->cloud_thread = NULL;
		app->cloud_thread_done = false;
	}

	if(app->spawn_library_thread)
	{
		kill_all_threads(app);
		strcpy(file_database_fn, t3f_get_filename(t3f_data_path, "files.ini", buffer, 1024));
		strcpy(entry_database_fn, t3f_get_filename(t3f_data_path, "database.ini", buffer, 1024));
		if(app->library)
		{
			omo_destroy_library(app->library);
			app->library = NULL;
		}
		omo_setup_library(app, file_database_fn, entry_database_fn, NULL);
		app->spawn_library_thread = false;
	}
	if(app->spawn_library_lists_thread && app->library)
	{
		omo_setup_library_lists(app);
		app->spawn_library_lists_thread = false;
	}
	if(app->spawn_cloud_thread && app->library)
	{
		app->cloud_thread_done = false;
		script_url = al_get_config_value(t3f_config, "Settings", "tag_track_url");
		if(script_url)
		{
			omo_submit_library_tags(app, script_url);
		}
		app->spawn_cloud_thread = false;
	}
	if(app->spawn_queue_thread)
	{
		omo_get_queue_tags(app->player->queue, app->library, app);
		app->spawn_queue_thread = false;
	}
}
