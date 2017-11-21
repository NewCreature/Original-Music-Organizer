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

static const char * get_library_folder(ALLEGRO_CONFIG * cp, int folder)
{
	const char * val;
	char buffer[64];

	sprintf(buffer, "library_folder_%d", folder);
	val = al_get_config_value(cp, "Settings", buffer);

	return val;
}

static time_t get_path_mtime(const char * fn)
{
	ALLEGRO_FS_ENTRY * ep;
	time_t mtime = 0;

	ep = al_create_fs_entry(fn);
	if(ep)
	{
		mtime = al_get_fs_entry_mtime(ep);
		al_destroy_fs_entry(ep);
	}

	return mtime;
}

static bool omo_scan_library_folders(APP_INSTANCE * app)
{
	time_t mtime;
	const char * val;
	int c, i, j;

	val = al_get_config_value(app->library_config, "Settings", "library_folders");
	if(!val || atoi(val) < 1)
	{
		sprintf(app->status_bar_text, "No Library Folders");
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	c = atoi(val);
	app->loading_library->modified_time = 0;
	for(i = 0; i < c; i++)
	{
		val = get_library_folder(app->library_config, i);
		if(val)
		{
			mtime = get_path_mtime(val);
			if(mtime > app->loading_library->modified_time)
			{
				app->loading_library->modified_time = mtime;
			}
		}
	}
	if(app->loading_library->modified_time > get_path_mtime(t3f_get_filename(t3f_data_path, "omo.library")))
	{
		app->loading_library->modified = true;
	}
	sprintf(app->status_bar_text, "Attempting to load cached library data...");
	if(app->loading_library->modified || !omo_load_library_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.library")))
	{
		omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->loading_library, app->player->queue, app->library_temp_path, app->status_bar_text);
		for(j = 0; j < c; j++)
		{
			sprintf(app->status_bar_text, "Scanning folder %d of %d...", j + 1, c);
			val = get_library_folder(app->library_config, j);
			if(val)
			{
				t3f_scan_files(val, omo_count_file, false, NULL, &app->loading_library_file_helper_data);
				sprintf(app->status_bar_text, "Saving progress...");
				omo_save_library(app->loading_library);
			}
		}
		omo_allocate_library(app->loading_library, app->loading_library_file_helper_data.file_count);
		for(j = 0; j < c; j++)
		{
			if(app->loading_library_file_helper_data.cancel_scan)
			{
				app->loading_library_file_helper_data.scan_done = true;
				return false;
			}
			val = get_library_folder(app->library_config, j);
			sprintf(app->status_bar_text, "Scanning folder %d of %d...", j + 1, c);
			t3f_scan_files(val, omo_add_file, false, NULL, &app->loading_library_file_helper_data);
			omo_save_library(app->loading_library);
			sprintf(app->status_bar_text, "Saving progress...");
		}
	}
	return true;
}

bool omo_build_library_artists_list(APP_INSTANCE * app, OMO_LIBRARY * lp)
{
	const char * val;
	int i;

	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	strcpy(lp->last_artist_name, "");
	lp->artist_entry_count = 0;
	sprintf(app->status_bar_text, "Creating artist list...");
	omo_add_artist_to_library(lp, "All Artists");
	omo_add_artist_to_library(lp, "Unknown Artist");
	for(i = 0; i < lp->entry_count; i++)
	{
		val = al_get_config_value(lp->entry_database, lp->entry[i]->id, "Artist");
		if(val)
		{
			omo_add_artist_to_library(lp, val);
		}
	}
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->status_bar_text, "Sorting artist list...");
	if(lp->artist_entry_count > 2)
	{
		qsort(&lp->artist_entry[2], lp->artist_entry_count - 2, sizeof(char *), sort_names);
	}
	omo_save_library_artists_cache(lp, t3f_get_filename(t3f_data_path, "omo.artists"));

	return true;
}

static bool omo_setup_library_helper(APP_INSTANCE * app)
{

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->player->queue, NULL, app->status_bar_text);
	sprintf(app->status_bar_text, "Loading library databases...");
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
	if(!omo_scan_library_folders(app))
	{
		return false;
	}
	/* tally up artists */
	if(app->loading_library->modified || !omo_load_library_artists_cache(app->loading_library, t3f_get_filename(t3f_data_path, "omo.artists")))
	{
		if(!omo_build_library_artists_list(app, app->loading_library))
		{
			return false;
		}
	}

	/* tally up albums */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->status_bar_text, "Creating album list...");
	omo_get_library_album_list(app->loading_library, "All Artists");
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}

	/* make song list */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->status_bar_text, "Creating song list...");
	omo_get_library_song_list(app->loading_library, "All Artists", "All Albums");

	app->loading_library_file_helper_data.scan_done = true;
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		return false;
	}
	sprintf(app->status_bar_text, "Library ready.");
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
	strcpy(app->status_bar_text, "");
	app->library_thread = al_create_thread(library_setup_thread_proc, app);
	if(app->library_thread)
	{
		al_start_thread(app->library_thread);
	}
}

const char * omo_get_library_file_id(OMO_LIBRARY * lp, const char * fn, const char * subfn, const char * track)
{
	char full_path[1024];

	strcpy(full_path, fn);
	if(subfn)
	{
		strcat(full_path, "/");
		strcat(full_path, subfn);
	}
	if(track)
	{
		strcat(full_path, ":");
		strcat(full_path, track);
	}
	return al_get_config_value(lp->file_database, full_path, "id");
}
