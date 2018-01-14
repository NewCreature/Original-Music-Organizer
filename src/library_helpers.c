#include "t3f/t3f.h"
#include "t3f/file.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "library_cache.h"
#include "constants.h"

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
			mtime = omo_get_folder_mtime(val);
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
				t3f_scan_files(val, omo_count_file, false, &app->loading_library_file_helper_data);
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
			t3f_scan_files(val, omo_add_file, false, &app->loading_library_file_helper_data);
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
		val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
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
	app->loading_library_file_helper_data.scan_done = true;
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
		omo_cancel_library_sort();
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

static bool omo_setup_library_lists_helper(APP_INSTANCE * app)
{

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->player->queue, NULL, app->status_bar_text);

	/* tally up artists */
	if(app->library->modified || !omo_load_library_artists_cache(app->library, t3f_get_filename(t3f_data_path, "omo.artists")))
	{
		sprintf(app->status_bar_text, "Creating artist list...");
		if(!omo_build_library_artists_list(app, app->library))
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
	omo_get_library_album_list(app->library, "All Artists");
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
	omo_get_library_song_list(app->library, "All Artists", "All Albums");

	app->loading_library_file_helper_data.scan_done = true;
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		return false;
	}
	sprintf(app->status_bar_text, "Library ready.");
	return true;
}

static void * library_lists_setup_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(omo_setup_library_lists_helper(app))
	{
		app->library->loaded = true;
	}
	return NULL;
}

void omo_setup_library_lists(APP_INSTANCE * app)
{
	omo_cancel_library_setup(app);
	app->library->loaded = false;
	app->loading_library_file_helper_data.scan_done = false;
	app->loading_library_file_helper_data.cancel_scan = false;
	app->library_thread = al_create_thread(library_lists_setup_thread_proc, app);
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
	return omo_get_database_value(lp->file_database, full_path, "id");
}

const char * omo_get_library_file_base_id(OMO_LIBRARY * lp, const char * fn, char * buffer)
{
	const char * val;
	char base_size[256];

	val = omo_get_database_value(lp->file_database, fn, "id");
	if(val)
	{
		/* get the file ID of the base file */
		strcpy(buffer, val);
		buffer[32] = 0;
		sprintf(base_size, "%lu", t3f_file_size(fn));
		strcat(buffer, base_size);
		return buffer;
	}
	return NULL;
}

int omo_get_library_entry(OMO_LIBRARY * lp, const char * id)
{
	int i;

	for(i = 0; i < lp->entry_count; i++)
	{
		if(!strcmp(lp->entry[i]->id, id))
		{
			return i;
		}
	}
	return -1;
}

int omo_get_library_base_entry(OMO_LIBRARY * lp, const char * id)
{
	int i;

	for(i = 0; i < lp->entry_count; i++)
	{
		if(!memcmp(lp->entry[i]->id, id, strlen(id)))
		{
			return i;
		}
	}
	return -1;
}

bool omo_split_track(OMO_LIBRARY * lp, const char * basefn, char * split_string)
{
	const char * base_id;
	char buffer[1024];
	char * token;
	bool done = false;
	int current_track = 0;
	const char * title_val;
	char title_buf[1024];
	char buf[256];
	char id[1024];
	char fn[1024];
	const char * val;
	int i;

	/* copy track info string to entry database first, before breaking up the
	   track list to put into the file database */
	base_id = omo_get_library_file_base_id(lp, basefn, buffer);
	if(!base_id)
	{
		return false;
	}
	omo_set_database_value(lp->entry_database, base_id, "Split Track Info", split_string);
	title_val = omo_get_database_value(lp->entry_database, base_id, "Title");

	token = strtok(split_string, ", ");
	while(!done)
	{
		if(!token)
		{
			if(current_track > 0)
			{
				sprintf(buf, "%d", current_track);
				omo_set_database_value(lp->file_database, basefn, "tracks", buf);
			}
			else
			{
				omo_set_database_value(lp->file_database, basefn, "tracks", "1");
				omo_remove_database_key(lp->file_database, basefn, "track_0");
				omo_remove_database_key(lp->entry_database, base_id, "Split Track Info");
			}
			break;
		}
		else
		{
			/* create entry in file database */
			sprintf(fn, "%s:%s", basefn, token);
			sprintf(id, "%s%s", base_id, token);
			omo_set_database_value(lp->file_database, fn, "id", id);
			sprintf(buf, "track_%d", current_track);
			omo_set_database_value(lp->file_database, basefn, buf, token);

			/* copy base tags to new track */
			for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
			{
				if(omo_tag_type[i])
				{
					val = omo_get_database_value(lp->entry_database, base_id, omo_tag_type[i]);
					if(val)
					{
						omo_set_database_value(lp->entry_database, id, omo_tag_type[i], val);
					}
				}
			}

			/* set appropriate title for new track */
			if(title_val)
			{
				sprintf(title_buf, "%s: Track %d", title_val, current_track + 1);
				omo_set_database_value(lp->entry_database, id, "Title", title_buf);
				omo_set_database_value(lp->entry_database, id, "scanned", "1");
			}
			current_track++;
		}
		token = strtok(NULL, ", ");
	}
	return true;
}

bool omo_backup_entry_tags(OMO_LIBRARY * lp, const char * id)
{
	const char * val;
	int i;

	/* if we already have a backup, don't overwrite */
	if(lp->entry_backup)
	{
		return false;
	}

	lp->entry_backup = al_create_config();
	if(lp->entry_backup)
	{
		al_set_config_value(lp->entry_backup, NULL, "id", id);
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(omo_tag_type[i])
			{
				val = omo_get_database_value(lp->entry_database, id, omo_tag_type[i]);
				if(val)
				{
					al_set_config_value(lp->entry_backup, id, omo_tag_type[i], val);
				}
			}
		}
		val = omo_get_database_value(lp->entry_database, id, "Split Track Info");
		if(val)
		{
			al_set_config_value(lp->entry_backup, id, "Split Track Info", val);
		}
		val = omo_get_database_value(lp->entry_database, id, "Detected Length");
		if(val)
		{
			al_set_config_value(lp->entry_backup, id, "Detected Length", val);
		}
		return true;
	}
	return false;
}

bool omo_restore_entry_tags(OMO_LIBRARY * lp)
{
	const char * val;
	const char * id;
	int i;

	if(lp->entry_backup)
	{
		id = al_get_config_value(lp->entry_backup, NULL, "id");
		if(id)
		{
			for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
			{
				if(omo_tag_type[i])
				{
					val = al_get_config_value(lp->entry_backup, id, omo_tag_type[i]);
					if(val)
					{
						omo_set_database_value(lp->entry_database, id, omo_tag_type[i], val);
					}
					else
					{
						omo_remove_database_key(lp->entry_database, id, omo_tag_type[i]);
					}
				}
			}
			val = al_get_config_value(lp->entry_backup, id, "Split Track Info");
			if(val)
			{
				omo_set_database_value(lp->entry_database, id, "Split Track Info", val);
			}
			else
			{
				omo_remove_database_key(lp->entry_database, id, "Split Track Info");
			}
			val = al_get_config_value(lp->entry_backup, id, "Detected Length");
			if(val)
			{
				omo_set_database_value(lp->entry_database, id, "Detected Length", val);
			}
			else
			{
				omo_remove_database_key(lp->entry_database, id, "Detected Length");
			}
			al_destroy_config(lp->entry_backup);
			lp->entry_backup = NULL;
			return true;
		}
	}
	return false;
}

void omo_discard_entry_backup(OMO_LIBRARY * lp)
{
	if(lp->entry_backup)
	{
		al_destroy_config(lp->entry_backup);
		lp->entry_backup = NULL;
	}
}
