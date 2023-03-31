#include <ctype.h>
#include "t3f/t3f.h"
#include "t3f/file.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "library_cache.h"
#include "constants.h"
#include "profile.h"
#include "library_helpers.h"

static OMO_LIBRARY * library = NULL;
static bool library_sort_cancelled = false;

void omo_start_library_sort(void)
{
	library_sort_cancelled = false;
}

void omo_cancel_library_sort(void)
{
	library_sort_cancelled = true;
}
static const char * skip_articles_word(const char * s, const char * skip_word)
{
	int l = strlen(s);
	int skip_l = strlen(skip_word);
	int i;

	for(i = 0; i < l && i < skip_l; i++)
	{
		if(tolower(s[i]) != tolower(skip_word[i]))
		{
			return s;
		}
	}
	if(i == skip_l)
	{
		if(s[i] == ' ')
		{
			return &s[i + 1];
		}
	}
	return s;
}

static const char * skip_articles(const char * s)
{
	const char * skip_word[3] = {"a", "and", "the"};
	const char * final_s = s;
	int pos = 0;
	int i;

	if(s[pos] == '\"')
	{
		pos++;
	}
	if(s[pos] == '\'')
	{
		pos++;
	}
	final_s = &s[pos];
	for(i = 0; i < 3; i++)
	{
		final_s = skip_articles_word(final_s, skip_word[i]);
	}
	return final_s;
}

static bool strmatch_helper(const char * s1, const char * s2)
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	int i;

	if(l2 < l1)
	{
		return false;
	}
	for(i = 0; i < l1 && i < l2; i++)
	{
		if(tolower(s1[i]) != tolower(s2[i]))
		{
			return false;
		}
	}
	return true;
}

static bool strmatch(const char * s1, const char * s2)
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	int i;

	if(l2 < l1)
	{
		return false;
	}
	for(i = 0; i < l2; i++)
	{
		if(tolower(s2[i]) == tolower(s1[0]))
		{
			if(strmatch_helper(s1, &s2[i]))
			{
				return true;
			}
		}
	}
	return false;
}

/* when all else fails, sort by path */
static int sort_by_path(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	char section1[1024];
	char section2[1024];

	if(library_sort_cancelled)
	{
		return 0;
	}

	strcpy(section1, library->entry[entry1]->filename);
	if(library->entry[entry1]->sub_filename)
	{
		strcat(section1, "/");
		strcat(section1, library->entry[entry1]->sub_filename);
	}
	if(library->entry[entry1]->track)
	{
		strcat(section1, ":");
		strcat(section1, library->entry[entry1]->track);
	}
	strcpy(section2, library->entry[entry2]->filename);
	if(library->entry[entry2]->sub_filename)
	{
		strcat(section2, "/");
		strcat(section2, library->entry[entry2]->sub_filename);
	}
	if(library->entry[entry2]->track)
	{
		strcat(section2, ":");
		strcat(section2, library->entry[entry2]->track);
	}

	return strcmp(section1, section2);
}

/* sort by artist, album, title */
static int sort_by_artist_album_title(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	const char * sort_field[3] = {"Artist", "Album", "Title"};
	int sort_type[3] = {0, 0, 0};
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i1, i2;
	int i, c;

	if(library_sort_cancelled)
	{
		return 0;
	}

	id1 = library->entry[entry1]->id;
	id2 = library->entry[entry2]->id;

	for(i = 0; i < 3; i++)
	{
		val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
		val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
		if(val1 && val2)
		{
			if(sort_type[i] == 0)
			{
				val1 = skip_articles(val1);
				val2 = skip_articles(val2);
				c = strcasecmp(val1, val2);
				if(c != 0)
				{
					return c;
				}
			}
			else
			{
				i1 = atoi(val1);
				i2 = atoi(val2);
				if(i1 != i2)
				{
					return i1 - i2;
				}
			}
		}
	}
	return sort_by_path(e1, e2);
}

static int sort_by_track(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i1, i2;

	if(library_sort_cancelled)
	{
		return 0;
	}

	id1 = library->entry[entry1]->id;
	id2 = library->entry[entry2]->id;

	/* sort by disc first */
	val1 = omo_get_database_value(library->entry_database, id1, "Disc");
	val2 = omo_get_database_value(library->entry_database, id2, "Disc");
	if(val1 && val2)
	{
		i1 = atoi(val1);
		i2 = atoi(val2);
		if(i1 != i2)
		{
			return i1 - i2;
		}
	}
	else if(val1)
	{
		return -1;
	}
	else if(val2)
	{
		return 1;
	}

	/* if discs match, sort by track */
	val1 = omo_get_database_value(library->entry_database, id1, "Track");
	val2 = omo_get_database_value(library->entry_database, id2, "Track");
	if(val1 && val2)
	{
		i1 = atoi(val1);
		i2 = atoi(val2);
		if(i1 != i2)
		{
			return i1 - i2;
		}
	}
	else if(val1)
	{
		return -1;
	}
	else if(val2)
	{
		return 1;
	}
	return sort_by_artist_album_title(e1, e2);
}

static int sort_by_title(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	char buf1[1024];
	char buf2[1024];
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int c;

	if(library_sort_cancelled)
	{
		return 0;
	}

	id1 = library->entry[entry1]->id;
	id2 = library->entry[entry2]->id;

	val1 = omo_get_database_value(library->entry_database, id1, "Title");
	if(!val1)
	{
		val1 = omo_get_database_value(library->entry_database, id1, "Album");
		if(val1)
		{
			strcpy(buf1, val1);
			strcat(buf1, " - Track ");
			val1 = omo_get_database_value(library->entry_database, id1, "Track");
			if(val1)
			{
				strcat(buf1, val1);
				val1 = buf1;
			}
			else
			{
				val1 = NULL;
			}
		}
	}
	val2 = omo_get_database_value(library->entry_database, id2, "Title");
	if(!val2)
	{
		val2 = omo_get_database_value(library->entry_database, id2, "Album");
		if(val2)
		{
			strcpy(buf2, val2);
			strcat(buf2, " - Track ");
			val2 = omo_get_database_value(library->entry_database, id2, "Track");
			if(val2)
			{
				strcat(buf2, val2);
				val2 = buf2;
			}
			else
			{
				val2 = NULL;
			}
		}
	}
	if(val1 && val2)
	{
		val1 = skip_articles(val1);
		val2 = skip_articles(val2);
		c = strcasecmp(val1, val2);
		if(c != 0)
		{
			return c;
		}
	}
	else if(val1)
	{
		return -1;
	}
	else if(val2)
	{
		return 1;
	}

	return sort_by_path(e1, e2);
}

static void library_sort_by_track(OMO_LIBRARY * lp)
{
	library = lp;
	qsort(lp->song_entry, lp->song_entry_count, sizeof(unsigned long), sort_by_track);
}

static void library_sort_by_title(OMO_LIBRARY * lp)
{
	library = lp;
	qsort(lp->song_entry, lp->song_entry_count, sizeof(unsigned long), sort_by_title);
}

static int sort_names(const void *e1, const void *e2)
{
	char ** s1 = (char **)e1;
	char ** s2 = (char **)e2;
	return strcasecmp(skip_articles(*s1), skip_articles(*s2));
}

static int sort_albums(const void *e1, const void *e2)
{
	OMO_ALBUM_ENTRY * s1 = (OMO_ALBUM_ENTRY *)e1;
	OMO_ALBUM_ENTRY * s2 = (OMO_ALBUM_ENTRY *)e2;
	return strcasecmp(skip_articles(s1->name), skip_articles(s2->name));
}

static const char * get_library_folder(ALLEGRO_CONFIG * cp, int folder, char * buffer, int buffer_size)
{
	const char * val;
	const char * profile_section;
	char section_buffer[256];
	char buf[64];
	ALLEGRO_PATH * path;
	ALLEGRO_FS_ENTRY * fs_entry;
	uint32_t mode;
	char * ret = NULL;

	profile_section = omo_get_profile_section(cp, omo_get_profile(omo_get_current_profile()), section_buffer);
	if(!profile_section)
	{
		return NULL;
	}
	sprintf(buf, "library_folder_%d", folder);
	val = al_get_config_value(cp, profile_section, buf);
	if(val)
	{
		fs_entry = al_create_fs_entry(val);
		if(fs_entry)
		{
			mode = al_get_fs_entry_mode(fs_entry);
			al_destroy_fs_entry(fs_entry);
			if(mode & ALLEGRO_FILEMODE_ISDIR)
			{
				if(strlen(val) < buffer_size)
				{
					strcpy(buffer, val);
					ret = buffer;
				}
			}
			else
			{
				path = al_create_path(val);
				if(path)
				{
					al_set_path_filename(path, NULL);
					val = al_path_cstr(path, '/');
					if(val && strlen(val) < buffer_size)
					{
						strcpy(buffer, val);
						if(strlen(buffer) > 0)
						{
							if(buffer[strlen(buffer) - 1] == '/')
							{
								buffer[strlen(buffer) - 1] = 0;
							}
						}
						ret = buffer;
					}
					al_destroy_path(path);
				}
			}
		}
	}

	return ret;
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
	char section_buffer[1024];
	char buffer[1024];
	time_t mtime;
	const char * val;
	int c, i, j;
	char cache_fn[1024];

	val = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.library", cache_fn, 1024);
	if(!val)
	{
		return false;
	}
	val = al_get_config_value(t3f_config, "Settings", "manual_scan");
	if(val && !strcmp(val, "true"))
	{
		if(omo_load_library_cache(app->loading_library, cache_fn))
		{
			return true;
		}
	}
	val = al_get_config_value(app->library_config, omo_get_profile_section(app->library_config, omo_get_profile(omo_get_current_profile()), section_buffer), "library_folders");
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
		val = get_library_folder(app->library_config, i, buffer, 1024);
		if(val)
		{
			mtime = omo_get_folder_mtime(val);
			if(mtime > app->loading_library->modified_time)
			{
				app->loading_library->modified_time = mtime;
			}
		}
	}
	if(app->loading_library->modified_time > get_path_mtime(cache_fn))
	{
		app->loading_library->modified = true;
	}
	sprintf(app->status_bar_text, "Attempting to load cached library data...");
	if(app->loading_library->modified || !omo_load_library_cache(app->loading_library, cache_fn))
	{
		omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, al_get_config_value(t3f_config, section_buffer, "filter"), app->loading_library, app->player->queue, app->library_temp_path, app->status_bar_text);
		for(j = 0; j < c; j++)
		{
			sprintf(app->status_bar_text, "Scanning folder %d of %d...", j + 1, c);
			val = get_library_folder(app->library_config, j, buffer, 1024);
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
			val = get_library_folder(app->library_config, j, buffer, 1024);
			if(val)
			{
				sprintf(app->status_bar_text, "Scanning folder %d of %d...", j + 1, c);
				t3f_scan_files(val, omo_add_file, false, &app->loading_library_file_helper_data);
				omo_save_library(app->loading_library);
				sprintf(app->status_bar_text, "Saving progress...");
			}
		}
	}
	return true;
}

bool omo_build_library_artists_list(APP_INSTANCE * app, OMO_LIBRARY * lp)
{
	char buffer[1024];
	const char * val;
	const char * fn;
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
		val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
		if(val)
		{
			omo_add_artist_to_library(lp, val);
		}
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
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.artists", buffer, 1024);
	if(fn)
	{
		omo_save_library_artists_cache(lp, fn);
	}
	omo_filter_library_artist_list(lp, NULL);

	return true;
}

void omo_filter_library_artist_list(OMO_LIBRARY * lp, const char * filter)
{
	int count = 2;
	int i;

	if(filter && strlen(filter))
	{
		for(i = 2; i < lp->artist_entry_count; i++)
		{
			if(strmatch(filter, lp->artist_entry[i]))
			{
				lp->filtered_artist_entry[count] = lp->artist_entry[i];
				count++;
			}
		}
		lp->filtered_artist_entry_count = count;
	}
	else
	{
		for(i = 0; i < lp->artist_entry_count; i++)
		{
			lp->filtered_artist_entry[i] = lp->artist_entry[i];
		}
		lp->filtered_artist_entry_count = lp->artist_entry_count;
	}
}

bool omo_get_library_album_list(OMO_LIBRARY * lp, const char * artist)
{
	char buffer[1024];
	const char * val;
	const char * val2;
	const char * fn;
	bool cache_loaded = false;
	bool all_artists = false;
	int i;

	omo_free_album_list(lp);
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.albums", buffer, 1024);
	if(!fn)
	{
		return false;
	}
	lp->album_entry = malloc(sizeof(OMO_ALBUM_ENTRY *) * lp->entry_count + 2);
	if(lp->album_entry)
	{
		lp->filtered_album_entry = malloc(sizeof(OMO_ALBUM_ENTRY *) * lp->entry_count + 2);
		if(!lp->filtered_album_entry)
		{
			free(lp->album_entry);
			return false;
		}
		strcpy(lp->last_album_name, "");
		strcpy(lp->last_album_disambiguation, "");
		memset(lp->album_entry, 0, sizeof(OMO_ALBUM_ENTRY *) * lp->entry_count + 2);
		if(!strcmp(artist, "All Artists"))
		{
			all_artists = true;
			if(lp->modified || !omo_load_library_albums_cache(lp, fn))
			{
				omo_add_album_to_library(lp, "All Albums", NULL);
				omo_add_album_to_library(lp, "Unknown Album", NULL);
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
					if(val)
					{
						omo_add_album_to_library(lp, val, val2);
					}
				}
			}
			else
			{
				cache_loaded = true;
			}
		}
		else if(!strcmp(artist, "Unknown Artist"))
		{
			omo_add_album_to_library(lp, "All Albums", NULL);
			omo_add_album_to_library(lp, "Unknown Album", NULL);
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				if(!val)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				}
				if(!val)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
					if(val)
					{
						omo_add_album_to_library(lp, val, val2);
					}
				}
			}
		}
		else
		{
			omo_add_album_to_library(lp, "All Albums", NULL);
			omo_add_album_to_library(lp, "Unknown Album", NULL);
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if((val && !strcmp(val, artist)) || (val2 && !strcmp(val2, artist)))
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
					if(val)
					{
						omo_add_album_to_library(lp, val, val2);
					}
				}
			}
		}
		if(lp->album_entry_count > 2 && !cache_loaded)
		{
			qsort(&lp->album_entry[2], lp->album_entry_count - 2, sizeof(OMO_ALBUM_ENTRY), sort_albums);
			if(all_artists)
			{
				omo_save_library_albums_cache(lp, fn);
			}
		}
	}
	omo_filter_library_album_list(lp, NULL);
	return true;
}

void omo_filter_library_album_list(OMO_LIBRARY * lp, const char * filter)
{
	int count = 2;
	int i;

	if(filter && strlen(filter))
	{
		for(i = 2; i < lp->album_entry_count; i++)
		{
			if(strmatch(filter, lp->album_entry[i].name) || (lp->album_entry[i].disambiguation ? strmatch(filter, lp->album_entry[i].disambiguation) : 0))
			{
				lp->filtered_album_entry[count] = lp->album_entry[i];
				count++;
			}
		}
		lp->filtered_album_entry_count = count;
	}
	else
	{
		for(i = 0; i < lp->album_entry_count; i++)
		{
			lp->filtered_album_entry[i] = lp->album_entry[i];
		}
		lp->filtered_album_entry_count = lp->album_entry_count;
	}
}

static bool add_song(OMO_LIBRARY * lp, unsigned long entry, const char * ignore_genre)
{
	const char * genre;

	genre = omo_get_database_value(lp->entry_database, lp->entry[entry]->id, "Genre");
	if(!genre || !ignore_genre || strcmp(genre, ignore_genre))
	{
		lp->song_entry[lp->song_entry_count] = entry;
		lp->song_entry_count++;
		return true;
	}
	return false;
}

static bool song_already_listed(OMO_LIBRARY * lp, const char * id)
{
	int i;

	for(i = 0; i < lp->song_entry_count; i++)
	{
		if(!strcmp(lp->entry[lp->song_entry[i]]->id, id))
		{
			return true;
		}
	}
	return false;
}

bool omo_get_library_song_list(OMO_LIBRARY * lp, const char * artist, const char * album, const char * disambiguation)
{
	char buffer[1024];
	const char * val;
	const char * val2;
	const char * fn;
	const char * ignore_genre;
	int i;
	bool want_disambiguation;

	if(lp->song_entry)
	{
		free(lp->song_entry);
		lp->song_entry = NULL;
		free(lp->filtered_song_entry);
		lp->filtered_song_entry = NULL;
	}
	ignore_genre = al_get_config_value(t3f_config, "Settings", "Ignore Genre");
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.songs", buffer, 1024);
	if(!fn)
	{
		return false;
	}
	if(!strcmp(artist, "All Artists") && !strcmp(album, "All Albums") && !lp->modified && omo_load_library_songs_cache(lp, fn))
	{
		omo_filter_library_song_list(lp, NULL);
		return true;
	}
	else
	{
		lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
		lp->song_entry_count = 0;
		if(lp->song_entry)
		{
			lp->filtered_song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			if(!lp->filtered_song_entry)
			{
				free(lp->song_entry);
				return false;
			}
		}
	}
	if(!strcmp(artist, "All Artists"))
	{
		if(!strcmp(album, "All Albums"))
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				add_song(lp, i, ignore_genre);
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
			omo_save_library_songs_cache(lp, fn);
			omo_filter_library_song_list(lp, NULL);
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
				if(val)
				{
					if(!strcmp(val, "Unknown"))
					{
						add_song(lp, i, ignore_genre);
					}
				}
				else
				{
					add_song(lp, i, ignore_genre);
				}
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
		}
		else
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
				val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
				want_disambiguation = disambiguation ? strlen(disambiguation) >= 1 : 0;
				if(val)
				{
					if(!strcmp(val, album) && ((want_disambiguation && val2) ? !strcmp(val2, disambiguation) : (!want_disambiguation && !val2)) && !song_already_listed(lp, lp->entry[i]->id))
					{
						add_song(lp, i, ignore_genre);
					}
				}
			}
			omo_start_library_sort();
			library_sort_by_track(lp);
		}
	}
	else if(!strcmp(artist, "Unknown Artist"))
	{
		if(!strcmp(album, "All Albums"))
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				if(!val)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				}
				if(val)
				{
					if(!strcmp(val, artist))
					{
						add_song(lp, i, ignore_genre);
					}
				}
				else
				{
					add_song(lp, i, ignore_genre);
				}
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			bool unknown_artist;
			for(i = 0; i < lp->entry_count; i++)
			{
				unknown_artist = false;
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if(val)
				{
					if(!strcmp(val, artist))
					{
						unknown_artist = true;
					}
				}
				else
				{
					unknown_artist = true;
				}
				if(unknown_artist)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						if(!strcmp(val, album))
						{
							add_song(lp, i, ignore_genre);
						}
					}
					else
					{
						add_song(lp, i, ignore_genre);
					}
				}
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
		}
		else
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				if(!val)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				}
				if(!val || !strcmp(val, artist))
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
					if(val)
					{
						if(!strcmp(val, album) && ((disambiguation && val2) ? !strcmp(val2, disambiguation) : (!disambiguation && !val2)) && !song_already_listed(lp, lp->entry[i]->id))
						{
							add_song(lp, i, ignore_genre);
						}
					}
				}
			}
			omo_start_library_sort();
			library_sort_by_track(lp);
		}
	}
	else
	{
		if(!strcmp(album, "All Albums"))
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if((val && !strcmp(val, artist)) || (val2 && !strcmp(val2, artist)))
				{
					add_song(lp, i, ignore_genre);
				}
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			bool artist_match;

			for(i = 0; i < lp->entry_count; i++)
			{
				artist_match = false;
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if((val && !strcmp(val, artist)) || (val2 && !strcmp(val2, artist)))
				{
					artist_match = true;
				}
				if(artist_match)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						if(!strcmp(val, album))
						{
							add_song(lp, i, ignore_genre);
						}
					}
					else
					{
						add_song(lp, i, ignore_genre);
					}
				}
			}
			omo_start_library_sort();
			library_sort_by_title(lp);
		}
		else
		{
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album Artist");
				val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if((val && !strcmp(val, artist)) || (val2 && !strcmp(val2, artist)))
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					val2 = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Disambiguation");
					if(val)
					{
						if(!strcmp(val, album) && ((disambiguation && val2) ? !strcmp(val2, disambiguation) : (!disambiguation && !val2)) && !song_already_listed(lp, lp->entry[i]->id))
						{
							add_song(lp, i, ignore_genre);
						}
					}
				}
			}
			omo_start_library_sort();
			library_sort_by_track(lp);
		}
	}
	omo_filter_library_song_list(lp, NULL);

	return true;
}

void omo_filter_library_song_list(OMO_LIBRARY * lp, const char * filter)
{
	const char * val;
	int count = 0;
	int i;

	if(filter && strlen(filter))
	{
		for(i = count; i < lp->song_entry_count; i++)
		{
			val = omo_get_database_value(lp->entry_database, lp->entry[lp->song_entry[i]]->id, "Title");
			if(val)
			{
				if(strmatch(filter, val))
				{
					lp->filtered_song_entry[count] = lp->song_entry[i];
					count++;
				}
			}
			else
			{
				if(strmatch(filter, lp->entry[lp->song_entry[i]]->filename))
				{
					lp->filtered_song_entry[count] = lp->song_entry[i];
					count++;
				}
			}
		}
		lp->filtered_song_entry_count = count;
	}
	else
	{
		for(i = 0; i < lp->song_entry_count; i++)
		{
			lp->filtered_song_entry[i] = lp->song_entry[i];
		}
		lp->filtered_song_entry_count = lp->song_entry_count;
	}
}

static bool omo_setup_library_helper(APP_INSTANCE * app)
{
	char buffer[1024];
	const char * fn;

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, NULL, app->player->queue, NULL, app->status_bar_text);
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
	if(!app->loading_library_file_helper_data.cancel_scan)
	{
		fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.library", buffer, 1024);
		if(fn)
		{
			omo_save_library_cache(app->loading_library, fn);
		}
	}
	app->loading_library_file_helper_data.scan_done = true;
	return true;
}

static void prune_library(OMO_LIBRARY * lp)
{
	int i, j;
	const char * val;

	for(i = 0; i < lp->entry_count; i++)
	{
		for(j = 0; j < OMO_MAX_TAG_TYPES; j++)
		{
			if(omo_tag_type[j])
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, omo_tag_type[j]);
				if(val && strlen(val) < 1)
				{
					omo_remove_database_key(lp->entry_database, lp->entry[i]->id, omo_tag_type[j]);
				}
			}
		}
	}
}

static void * library_setup_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(omo_setup_library_helper(app))
	{
		if(app->prune_library)
		{
			prune_library(app->loading_library);
		}
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
	char buffer[1024];

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, NULL, app->player->queue, NULL, app->status_bar_text);

	/* tally up artists */
	if(app->library->modified || !omo_load_library_artists_cache(app->library, t3f_get_filename(t3f_data_path, "omo.artists", buffer, 1024)))
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
	omo_get_library_song_list(app->library, "All Artists", "All Albums", NULL);

	app->loading_library_file_helper_data.scan_done = true;
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		return false;
	}
	app->library->modified = false;
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
	if(app->destroy_library_lists_cache)
	{
		omo_clear_library_cache();
		app->destroy_library_lists_cache = false;
	}
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

bool omo_backup_entry_tags(OMO_LIBRARY * lp, const char * id, bool first)
{
	const char * val;
	int i;

	/* if we already have a backup, don't overwrite */
	if(lp->entry_backup && first)
	{
		return false;
	}

	if(!lp->entry_backup)
	{
		lp->entry_backup = al_create_config();
	}
	if(lp->entry_backup)
	{
//		al_set_config_value(lp->entry_backup, NULL, "id", id);
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
	ALLEGRO_CONFIG_SECTION * iterator;
	const char * val;
	const char * id;
	int i;

	if(lp->entry_backup)
	{
		id = al_get_first_config_section(lp->entry_backup, &iterator);
		while(id)
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
			id = al_get_next_config_section(&iterator);
		}
		al_destroy_config(lp->entry_backup);
		lp->entry_backup = NULL;
		return true;
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

double omo_get_library_entry_length(OMO_LIBRARY * lp, const char * id)
{
	const char * length = NULL;
	const char * loop_start = NULL;
	const char * loop_end = NULL;
	const char * fade_time = NULL;
	const char * loop_length = NULL;
	const char * loop_count = NULL;
	double d_loop_start = 0.0;
	double d_loop_end = 0.0;
	double d_fade_time = 0.0;
	double d_loop_length = 0.0;
	double d_loop_count = 1.0;
	double entry_length = 0.0;

	/* retrieve relevant tags from the database */
	length = omo_get_database_value(lp->entry_database, id, "Detected Length");
	if(!length)
	{
		loop_start = omo_get_database_value(lp->entry_database, id, "Loop Start");
		loop_end = omo_get_database_value(lp->entry_database, id, "Loop End");
		fade_time = omo_get_database_value(lp->entry_database, id, "Fade Time");
		loop_length = omo_get_database_value(lp->entry_database, id, "Length");
		if(!loop_start || !loop_end)
		{
			length = omo_get_database_value(lp->entry_database, id, "Length");
		}
	}

	/* use 'Detected Length' or 'Length' database values */
	if(length)
	{
		entry_length = atof(length);
	}
	/* use 'Loop Start', 'Loop End', 'Fade Time', and 'Length' database values */
	else
	{
		if(loop_start && loop_end)
		{
			loop_count = al_get_config_value(t3f_config, "Settings", "loop_count");
			if(loop_count)
			{
				d_loop_count = atof(loop_count);
			}
			d_loop_start = atof(loop_start);
			d_loop_end = atof(loop_end);
			if(fade_time)
			{
				d_fade_time = atof(fade_time);
			}
			if(loop_length)
			{
				d_loop_length = atof(loop_length);
			}

			/* loop end is < 0.0 means we use the track's original length */
			if(d_loop_end < 0)
			{
				if(loop_length)
				{
					d_loop_end = d_loop_length;
				}
			}
			else
			{
				entry_length = d_loop_start + (d_loop_end - d_loop_start) * d_loop_count;
				if(fade_time)
				{
					entry_length += d_fade_time;
				}
			}
		}
	}
	return entry_length;
}

void omo_rebase_library_file_database(OMO_LIBRARY * lp, const char * base_path, const char * rebase_path)
{
	ALLEGRO_CONFIG * new_config;
	ALLEGRO_CONFIG_SECTION * config_section;
	ALLEGRO_CONFIG_ENTRY * config_entry;
	const char * section;
	const char * val;
	const char * suffix;
	int l, l2;
	char write_section[1024];
	bool temp_library = false;
	char file_database_fn[1024];
	char entry_database_fn[1024];

	if(!lp)
	{
		t3f_get_filename(t3f_data_path, "files.ini", file_database_fn, 1024);
		t3f_get_filename(t3f_data_path, "database.ini", entry_database_fn, 1024);
		lp = omo_create_library(file_database_fn, entry_database_fn);
		temp_library = true;
	}
	if(lp->file_database->config)
	{
		new_config = al_create_config();
		if(new_config)
		{
			al_lock_mutex(lp->file_database->mutex);
			section = al_get_first_config_section(lp->file_database->config, &config_section);
			while(section)
			{
				/* rewrite section name using rebase_path if the base matches the section name */
				l = strlen(section);
				l2 = strlen(rebase_path);
				if(l >= l2)
				{
					if(!memcmp(section, rebase_path, l2))
					{
						suffix = &section[l2];
						sprintf(write_section, "%s%s", base_path, suffix);
						val = al_get_first_config_entry(lp->file_database->config, section, &config_entry);
						while(val)
						{
							al_set_config_value(new_config, write_section, val, al_get_config_value(lp->file_database->config, section, val));
							val = al_get_next_config_entry(&config_entry);
						}
					}
				}
				section = al_get_next_config_section(&config_section);
			}
			al_destroy_config(lp->file_database->config);
			lp->file_database->config = new_config;
			al_unlock_mutex(lp->file_database->mutex);
		}
	}
	if(temp_library)
	{
		omo_save_library(lp);
		omo_destroy_library(lp);
	}
}

static void fix_scroll_position(T3GUI_ELEMENT * element, int entries)
{
	int visible = element->h / al_get_font_line_height(element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0]) - 1;

	if(entries > visible)
	{
		element->d2 = element->d1;
		if(element->d2 + visible > entries)
		{
			element->d2 = entries - visible;
		}
	}
}

bool omo_find_track(APP_INSTANCE * app, const char * track_id)
{
	const char * artist;
	const char * album;
	const char * disambiguation;
	int i;

	strcpy(app->ui->ui_artist_search_element->dp, "");
	strcpy(app->ui->ui_album_search_element->dp, "");
	strcpy(app->ui->ui_song_search_element->dp, "");
	omo_filter_library_artist_list(app->library, NULL);
	omo_filter_library_album_list(app->library, NULL);
	omo_filter_library_song_list(app->library, NULL);
	omo_get_library_song_list(app->library, "All Artists", "All Albums", NULL);
	if(track_id)
	{
		app->ui->ui_artist_list_element->d1 = 0;
		app->ui->ui_artist_list_element->d2 = 0;
		app->ui->ui_album_list_element->d1 = 0;
		app->ui->ui_album_list_element->d2 = 0;
		app->ui->ui_song_list_element->d1 = 0;
		app->ui->ui_song_list_element->d2 = 0;
		artist = omo_get_database_value(app->library->entry_database, track_id, "Artist");
		if(artist)
		{
			for(i = 0; i < app->library->artist_entry_count; i++)
			{
				if(!strcmp(artist, app->library->artist_entry[i]))
				{
					app->ui->ui_artist_list_element->d1 = i;
					fix_scroll_position(app->ui->ui_artist_list_element, app->library->artist_entry_count);
					break;
				}
			}
			omo_get_library_album_list(app->library, artist);
		}
		album = omo_get_database_value(app->library->entry_database, track_id, "Album");
		disambiguation = omo_get_database_value(app->library->entry_database, track_id, "Disambiguation");
		if(album)
		{
			for(i = 0; i < app->library->album_entry_count; i++)
			{
				if(!strcmp(album, app->library->album_entry[i].name) && (disambiguation ? !strcmp(disambiguation, app->library->album_entry[i].disambiguation) : 1))
				{
					app->ui->ui_album_list_element->d1 = i;
					fix_scroll_position(app->ui->ui_album_list_element, app->library->album_entry_count);
					break;
				}
			}
			omo_get_library_song_list(app->library, artist ? artist : "All Artists", album, disambiguation);
		}
		for(i = 0; i < app->library->song_entry_count; i++)
		{
			if(!strcmp(app->library->entry[app->library->song_entry[i]]->id, track_id))
			{
				app->ui->ui_song_list_element->d1 = i + 1;
				fix_scroll_position(app->ui->ui_song_list_element, app->library->song_entry_count);
				break;
			}
		}
	}
	return true;
}