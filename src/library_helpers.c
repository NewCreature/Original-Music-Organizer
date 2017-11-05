#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "library_cache.h"

static int sort_names(const void *e1, const void *e2)
{
	char ** s1 = (char **)e1;
	char ** s2 = (char **)e2;
    return strcasecmp(*s1, *s2);
}

static bool omo_setup_library_helper(APP_INSTANCE * app)
{
	const char * val;
	char buffer[64];
	int c, i, j;

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->player->queue, NULL, app->library_loading_message);
	sprintf(app->library_loading_message, "Loading library databases...");
	app->loading_library = omo_create_library(app->file_database_fn, app->entry_database_fn);
	if(!app->loading_library)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}

	/* scan library paths */
	val = al_get_config_value(app->library_config, "Settings", "library_folders");
	if(!val || atoi(val) < 1)
	{
		sprintf(app->library_loading_message, "No Library Folders");
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	c = atoi(val);
	sprintf(app->library_loading_message, "Attempting to load cached library data...");
	if(!omo_load_library_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.library")))
	{
		omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->loading_library, app->player->queue, app->library_temp_path, app->library_loading_message);
		for(j = 0; j < c; j++)
		{
			sprintf(app->library_loading_message, "Scanning folder %d of %d...", j + 1, c);
			sprintf(buffer, "library_folder_%d", j);
			val = al_get_config_value(app->library_config, "Settings", buffer);
			if(val)
			{
				t3f_scan_files(val, omo_count_file, false, NULL, &app->loading_library_file_helper_data);
				sprintf(app->library_loading_message, "Saving progress...");
				omo_save_library(app->loading_library);
			}
		}
		omo_allocate_library(app->loading_library, app->loading_library_file_helper_data.file_count);
		for(j = 0; j < c; j++)
		{
			sprintf(buffer, "library_folder_%d", j);
			val = al_get_config_value(app->library_config, "Settings", buffer);
			if(app->loading_library_file_helper_data.cancel_scan)
			{
				app->loading_library_file_helper_data.scan_done = true;
				return false;
			}
			sprintf(app->library_loading_message, "Scanning folder %d of %d...", j + 1, c);
			t3f_scan_files(val, omo_add_file, false, NULL, &app->loading_library_file_helper_data);
			omo_save_library(app->loading_library);
			sprintf(app->library_loading_message, "Saving progress...");
		}
	}

	/* tally up artists */
	if(!omo_load_library_artists_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.artists")))
	{
		if(app->loading_library_file_helper_data.cancel_scan)
		{
			app->loading_library_file_helper_data.scan_done = true;
			return false;
		}
		sprintf(app->library_loading_message, "Creating artist list...");
		omo_add_artist_to_library(app->loading_library, "All Artists");
		omo_add_artist_to_library(app->loading_library, "Unknown Artist");
		for(i = 0; i < app->loading_library->entry_count; i++)
		{
			val = al_get_config_value(app->loading_library->entry_database, app->loading_library->entry[i]->id, "Artist");
			if(val)
			{
				omo_add_artist_to_library(app->loading_library, val);
			}
		}
		if(app->loading_library_file_helper_data.cancel_scan)
		{
			app->loading_library_file_helper_data.scan_done = true;
			return false;
		}
		sprintf(app->library_loading_message, "Sorting artist list...");
		if(app->loading_library->artist_entry_count > 2)
		{
			qsort(&app->loading_library->artist_entry[2], app->loading_library->artist_entry_count - 2, sizeof(char *), sort_names);
		}
		omo_save_library_artists_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.artists"));
	}

	/* tally up albums */
	if(!omo_load_library_albums_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.albums")))
	{
		if(app->loading_library_file_helper_data.cancel_scan)
		{
			app->loading_library_file_helper_data.scan_done = true;
			return false;
		}
		sprintf(app->library_loading_message, "Creating album list...");
		omo_get_library_album_list(app->loading_library, "All Artists");
		if(app->loading_library_file_helper_data.cancel_scan)
		{
			app->loading_library_file_helper_data.scan_done = true;
			return false;
		}
	}

	/* make song list */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->library_loading_message, "Creating song list...");
	omo_get_library_song_list(app->loading_library, "All Artists", "All Albums");

	app->loading_library_file_helper_data.scan_done = true;
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		return false;
	}
	return true;
}

static void * library_setup_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(omo_setup_library_helper(app))
	{
		app->library = app->loading_library;
	}
	return NULL;
}

void omo_cancel_library_setup(APP_INSTANCE * app)
{
	if(app->library_thread)
	{
		app->loading_library_file_helper_data.cancel_scan = true;
		al_join_thread(app->library_thread, NULL);
		al_destroy_thread(app->library_thread);
		app->library_thread = NULL;
	}
}

void omo_setup_library(APP_INSTANCE * app, const char * file_database_fn, const char * entry_database_fn, ALLEGRO_CONFIG * config)
{
	strcpy(app->file_database_fn, file_database_fn);
	strcpy(app->entry_database_fn, entry_database_fn);
	app->library_config = t3f_config;
	if(config)
	{
		app->library_config = config;
	}
	strcpy(app->library_loading_message, "");
	app->library_thread = al_create_thread(library_setup_thread_proc, app);
	if(app->library_thread)
	{
		al_start_thread(app->library_thread);
	}
}
