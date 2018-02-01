#include "t3f/t3f.h"

#include "library.h"
#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "defines.h"
#include "constants.h"
#include "md5.h"
#include "queue_helpers.h"
#include "library_cache.h"
#include "cloud.h"
#include "profile.h"

static bool library_sort_cancelled = false;

OMO_LIBRARY * omo_create_library(const char * file_db_fn, const char * entry_db_fn)
{
	OMO_LIBRARY * lp = NULL;

	lp = malloc(sizeof(OMO_LIBRARY));
	if(lp)
	{
		memset(lp, 0, sizeof(OMO_LIBRARY));
		lp->file_database = omo_create_database(file_db_fn);
		if(!lp->file_database)
		{
			goto fail;
		}
		lp->entry_database = omo_create_database(entry_db_fn);
		if(!lp->entry_database)
		{
			goto fail;
		}
		if(lp->file_database->empty || lp->entry_database->empty)
		{
			omo_clear_library_cache();
		}
	}
	return lp;

	fail:
	{
		if(lp)
		{
			if(lp->entry_database)
			{
				omo_destroy_database(lp->entry_database);
			}
			if(lp->file_database)
			{
				omo_destroy_database(lp->file_database);
			}
			free(lp);
		}
	}
	return NULL;
}

bool omo_allocate_library(OMO_LIBRARY * lp, int total_files)
{
	lp->entry = malloc(sizeof(OMO_LIBRARY_ENTRY *) * total_files);
	if(!lp->entry)
	{
		goto fail;
	}
	lp->entry_size = total_files;
	lp->entry_count = 0;

	lp->artist_entry = malloc(sizeof(char *) * total_files + 2);
	if(!lp->artist_entry)
	{
		goto fail;
	}
	lp->artist_entry_size = total_files + 2;
	lp->artist_entry_count = 0;
	strcpy(lp->last_artist_name, "");

	lp->album_entry = malloc(sizeof(char *) * total_files + 2);
	if(!lp->album_entry)
	{
		goto fail;
	}
	lp->album_entry_size = total_files + 2;
	lp->album_entry_count = 0;
	strcpy(lp->last_album_name, "");

	lp->song_entry = NULL;

	return true;

	fail:
	{
		if(lp->entry)
		{
			free(lp->entry);
			lp->entry = NULL;
		}
		if(lp->artist_entry)
		{
			free(lp->artist_entry);
			lp->artist_entry = NULL;
		}
		if(lp->album_entry)
		{
			free(lp->album_entry);
			lp->album_entry = NULL;
		}
	}
	return false;
}

void omo_free_album_list(OMO_LIBRARY * lp)
{
	int i;

	if(lp->album_entry)
	{
		for(i = 0; i < lp->album_entry_count; i++)
		{
			if(lp->album_entry[i])
			{
				free(lp->album_entry[i]);
				lp->album_entry[i] = NULL;
			}
		}
		lp->album_entry_count = 0;
		free(lp->album_entry);
		lp->album_entry = NULL;
	}
}

void omo_destroy_library(OMO_LIBRARY * lp)
{
	int i;

	if(lp->entry)
	{
		for(i = 0; i < lp->entry_count; i++)
		{
			if(lp->entry[i]->filename)
			{
				free(lp->entry[i]->filename);
			}
			free(lp->entry[i]);
		}
		free(lp->entry);
	}
	if(lp->artist_entry)
	{
		for(i = 0; i < lp->artist_entry_count; i++)
		{
			free(lp->artist_entry[i]);
		}
		free(lp->artist_entry);
	}
	omo_free_album_list(lp);
	if(lp->song_entry)
	{
		free(lp->song_entry);
	}
	omo_destroy_database(lp->file_database);
	omo_destroy_database(lp->entry_database);
	free(lp);
}

bool omo_save_library(OMO_LIBRARY * lp)
{
	bool ret = true;

	if(!omo_save_database(lp->file_database))
	{
		ret = false;
	}
	if(!omo_save_database(lp->entry_database))
	{
		ret = false;
	}
	return ret;
}

static bool get_tags(OMO_LIBRARY * lp, const char * id, const char * fn, const char * track, OMO_CODEC_HANDLER_REGISTRY * crp)
{
	OMO_CODEC_HANDLER * codec_handler;
	void * codec_data = NULL;
	const char * val;
	int i;

	codec_handler = omo_get_codec_handler(crp, fn);
	if(codec_handler)
	{
		if(codec_handler->get_tag)
		{
			codec_data = codec_handler->load_file(fn, track);
			if(codec_data)
			{
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					if(omo_tag_type[i])
					{
						val = codec_handler->get_tag(codec_data, omo_tag_type[i]);
						if(val && strlen(val))
						{
							omo_set_database_value(lp->entry_database, id, omo_tag_type[i], val);
						}
					}
				}
				if(codec_handler->unload_file)
				{
					codec_handler->unload_file(codec_data);
				}
				return true;
			}
		}
	}
	return false;
}

static unsigned long get_file_size(const char * fn)
{
	ALLEGRO_FS_ENTRY * fs_entry;
	unsigned long size = 0;

	fs_entry = al_create_fs_entry(fn);
	if(fs_entry)
	{
		size = al_get_fs_entry_size(fs_entry);
		al_destroy_fs_entry(fs_entry);
	}
	return size;
}

int omo_add_file_to_library(OMO_LIBRARY * lp, const char * fn, const char * subfn, const char * track, OMO_ARCHIVE_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER_REGISTRY * crp, ALLEGRO_PATH * temp_path)
{
	const char * val = NULL;
	const char * val2 = NULL;
	char sum_string[128];
	char section[1024];
	int ret = true;
	int retval = 2;
	const char * extracted_filename = NULL;
	OMO_ARCHIVE_HANDLER * archive_handler;
	void * archive_handler_data;
	bool hashed = false;
	unsigned long file_size;
	const char * md5_hash = NULL;
	char fn_buffer[1024] = {0};

	if(lp->entry_count < lp->entry_size)
	{
		sprintf(section, "%s", fn);
		if(subfn)
		{
			strcat(section, "/");
			strcat(section, subfn);
		}
		if(track)
		{
			strcat(section, ":");
			strcat(section, track);
		}
		val = omo_get_database_value(lp->file_database, section, "id");
		if(val)
		{
			val2 = omo_get_database_value(lp->entry_database, val, "scanned");
		}
		if(!val || !val2)
		{
			if(subfn)
			{
				archive_handler = omo_get_archive_handler(rp, fn);
				if(archive_handler)
				{
					archive_handler_data = archive_handler->open_archive(fn, temp_path);
					if(archive_handler_data)
					{
						extracted_filename = archive_handler->extract_file(archive_handler_data, atoi(subfn), fn_buffer);
						archive_handler->close_archive(archive_handler_data);
					}
				}
			}
			if(extracted_filename)
			{
				md5_hash = md5_file(extracted_filename);
				hashed = true;
			}
			else if(!subfn) // if we are here and subfn != NULL, we failed extraction
			{
				md5_hash = md5_file(fn);
				hashed = true;
			}

			/* if hash succeeded, add file and info to databases */
			if(hashed && md5_hash)
			{
				if(extracted_filename)
				{
					file_size = get_file_size(extracted_filename);
				}
				else
				{
					file_size = get_file_size(fn);
				}
				sprintf(sum_string, "%s%lu", md5_hash, file_size);
				if(track)
				{
					strcat(sum_string, track);
				}
				omo_set_database_value(lp->file_database, section, "id", sum_string);
				if(subfn)
				{
					omo_set_database_value(lp->file_database, section, "subfn", subfn);
				}
				if(extracted_filename)
				{
					get_tags(lp, sum_string, extracted_filename, track, crp);
					al_remove_filename(extracted_filename);
				}
				else
				{
					get_tags(lp, sum_string, fn, track, crp);
				}
				omo_retrieve_track_tags(lp, sum_string, "http://www.t3-i.com/omo/get_track_tags.php");
				omo_set_database_value(lp->entry_database, sum_string, "scanned", "1");
				retval = 1;
			}
		}
		lp->entry[lp->entry_count] = malloc(sizeof(OMO_LIBRARY_ENTRY));
		if(lp->entry[lp->entry_count])
		{
			lp->entry[lp->entry_count]->filename = malloc(strlen(fn) + 1);
			if(lp->entry[lp->entry_count]->filename)
			{
				strcpy(lp->entry[lp->entry_count]->filename, fn);
			}
			lp->entry[lp->entry_count]->sub_filename = NULL;
			if(subfn)
			{
				lp->entry[lp->entry_count]->sub_filename = malloc(strlen(subfn) + 1);
				if(lp->entry[lp->entry_count]->sub_filename)
				{
					strcpy(lp->entry[lp->entry_count]->sub_filename, subfn);
				}
				else
				{
					ret = false;
				}
			}
			lp->entry[lp->entry_count]->track = NULL;
			if(track)
			{
				lp->entry[lp->entry_count]->track = malloc(strlen(track) + 1);
				if(lp->entry[lp->entry_count]->track)
				{
					strcpy(lp->entry[lp->entry_count]->track, track);
				}
				else
				{
					ret = false;
				}
			}
			if(ret)
			{
				lp->entry[lp->entry_count]->id = omo_get_database_value(lp->file_database, section, "id");
				if(lp->entry[lp->entry_count]->id)
				{
					lp->entry_count++;
					return retval;
				}
			}
			if(lp->entry[lp->entry_count]->track)
			{
				free(lp->entry[lp->entry_count]->track);
			}
			if(lp->entry[lp->entry_count]->sub_filename)
			{
				free(lp->entry[lp->entry_count]->sub_filename);
			}
			if(lp->entry[lp->entry_count]->filename)
			{
				free(lp->entry[lp->entry_count]->filename);
			}
			free(lp->entry[lp->entry_count]);
		}
	}
	return 0;
}

static bool find_artist(OMO_LIBRARY * lp, const char * name)
{
	int i;

	/* optimize finding artist if it's the same as the previously added one */
	if(!strcmp(name, lp->last_artist_name))
	{
		return true;
	}

	for(i = 0; i < lp->artist_entry_count; i++)
	{
		if(!strcmp(lp->artist_entry[i], name))
		{
			return true;
		}
	}
	return false;
}

bool omo_add_artist_to_library(OMO_LIBRARY * lp, const char * name)
{
	if(lp->artist_entry_count < lp->artist_entry_size)
	{
		if(!find_artist(lp, name))
		{
			lp->artist_entry[lp->artist_entry_count] = malloc(strlen(name) + 1);
			if(lp->artist_entry[lp->artist_entry_count])
			{
				strcpy(lp->artist_entry[lp->artist_entry_count], name);
				strcpy(lp->last_artist_name, name);
				lp->artist_entry_count++;
				return true;
			}
		}
	}
	return false;
}

static bool find_album(OMO_LIBRARY * lp, const char * name)
{
	int i;

	/* optimize finding artist if it's the same as the previously added one */
	if(!strcmp(name, lp->last_album_name))
	{
		return true;
	}

	for(i = 0; i < lp->album_entry_count; i++)
	{
		if(!strcmp(lp->album_entry[i], name))
		{
			return true;
		}
	}
	return false;
}

bool omo_add_album_to_library(OMO_LIBRARY * lp, const char * name)
{
	if(lp->album_entry_count < lp->album_entry_size)
	{
		if(!find_album(lp, name))
		{
			lp->album_entry[lp->album_entry_count] = malloc(strlen(name) + 1);
			if(lp->album_entry[lp->album_entry_count])
			{
				strcpy(lp->album_entry[lp->album_entry_count], name);
				strcpy(lp->last_album_name, name);
				lp->album_entry_count++;
				return true;
			}
		}
	}
	return false;
}

static OMO_LIBRARY * library = NULL;

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
	char section1[1024];
	char section2[1024];
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

	id1 = omo_get_database_value(library->file_database, section1, "id");
	id2 = omo_get_database_value(library->file_database, section2, "id");

	if(id1 && id2)
	{
		for(i = 0; i < 3; i++)
		{
			val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
			val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
			if(val1 && val2)
			{
				if(sort_type[i] == 0)
				{
					c = strcmp(val1, val2);
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
	}
	return sort_by_path(e1, e2);
}

static int sort_by_track(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	char section1[1024];
	char section2[1024];
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i1, i2;

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
	id1 = omo_get_database_value(library->file_database, section1, "id");
	id2 = omo_get_database_value(library->file_database, section2, "id");

	if(id1 && id2)
	{
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
	}
	return sort_by_artist_album_title(e1, e2);
}

static int sort_by_title(const void *e1, const void *e2)
{
	int entry1 = *((int *)e1);
	int entry2 = *((int *)e2);
	const char * sort_field[1] = {"Title"};
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	char path[1024];
	int i, c;

	if(library_sort_cancelled)
	{
		return 0;
	}

	strcpy(path, library->entry[entry1]->filename);
	if(library->entry[entry1]->sub_filename)
	{
		strcat(path, "/");
		strcat(path, library->entry[entry1]->sub_filename);
	}
	if(library->entry[entry1]->track)
	{
		strcat(path, ":");
		strcat(path, library->entry[entry1]->track);
	}
	id1 = omo_get_database_value(library->file_database, path, "id");
	strcpy(path, library->entry[entry2]->filename);
	if(library->entry[entry2]->sub_filename)
	{
		strcat(path, "/");
		strcat(path, library->entry[entry2]->sub_filename);
	}
	if(library->entry[entry2]->track)
	{
		strcat(path, ":");
		strcat(path, library->entry[entry2]->track);
	}
	id2 = omo_get_database_value(library->file_database, path, "id");

	if(id1 && id2)
	{
		for(i = 0; i < 1; i++)
		{
			val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
			val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
			if(val1 && val2)
			{
				c = strcmp(val1, val2);
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
		}
	}

	return sort_by_path(e1, e2);
}

/*static const char * skip_articles(const char * s)
{
	const char * article[5] = {"a", "an", "the"};
	int i, j, l;

	for(i = 0; i < 3; i++)
	{
		l = strlen(article[i]);
		if(strlen(s) > l + 1)
		{
			for(j = 0; j < l; j++)
			{
				if(tolower(s[j]) != article[j])
				{
					break;
				}
				if(j == )
			}
		}
	}
	if(strlen(s) > 3)
	{
		if(tolower(s[0]) == 't' && tolower(s[0]) == 't')
	}
}

static int sort_strcmp(const char * s1, const char * s2)
{
	const char * real_s1;
	const char * real_s2;

} */

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
	return strcasecmp(*s1, *s2);
}

bool omo_get_library_album_list(OMO_LIBRARY * lp, const char * artist)
{
	char buffer[1024];
	const char * val;
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
	lp->album_entry = malloc(sizeof(char *) * lp->entry_count + 2);
	if(lp->album_entry)
	{
		strcpy(lp->last_album_name, "");
		memset(lp->album_entry, 0, sizeof(char *) * lp->entry_count + 2);
		if(!strcmp(artist, "All Artists"))
		{
			all_artists = true;
			if(lp->modified || !omo_load_library_albums_cache(lp, fn))
			{
				omo_add_album_to_library(lp, "All Albums");
				omo_add_album_to_library(lp, "Unknown Album");
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						omo_add_album_to_library(lp, val);
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
			omo_add_album_to_library(lp, "All Albums");
			omo_add_album_to_library(lp, "Unknown Album");
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if(!val)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						omo_add_album_to_library(lp, val);
					}
				}
			}
		}
		else
		{
			omo_add_album_to_library(lp, "All Albums");
			omo_add_album_to_library(lp, "Unknown Album");
			for(i = 0; i < lp->entry_count; i++)
			{
				val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
				if(val)
				{
					if(!strcmp(val, artist))
					{
						val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
						if(val)
						{
							omo_add_album_to_library(lp, val);
						}
					}
				}
			}
		}
		if(lp->album_entry_count > 2 && !cache_loaded)
		{
			qsort(&lp->album_entry[2], lp->album_entry_count - 2, sizeof(char *), sort_names);
			if(all_artists)
			{
				omo_save_library_albums_cache(lp, fn);
			}
		}
	}
	return true;
}

bool omo_get_library_song_list(OMO_LIBRARY * lp, const char * artist, const char * album)
{
	char buffer[1024];
	const char * val;
	const char * fn;
	int i;

	if(lp->song_entry)
	{
		free(lp->song_entry);
		lp->song_entry = NULL;
	}
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.songs", buffer, 1024);
	if(!fn)
	{
		return false;
	}
	if(!strcmp(artist, "All Artists"))
	{
		if(!strcmp(album, "All Albums"))
		{
			if(lp->modified || !omo_load_library_songs_cache(lp, fn))
			{
				lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
				if(lp->song_entry)
				{
					for(i = 0; i < lp->entry_count; i++)
					{
						lp->song_entry[i] = i;
					}
					lp->song_entry_count = lp->entry_count;
					omo_start_library_sort();
					library_sort_by_title(lp);
					omo_save_library_songs_cache(lp, fn);
				}
			}
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						if(!strcmp(val, "Unknown"))
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
					else
					{
						lp->song_entry[lp->song_entry_count] = i;
						lp->song_entry_count++;
					}
				}
				omo_start_library_sort();
				library_sort_by_title(lp);
			}
		}
		else
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
					if(val)
					{
						if(!strcmp(val, album))
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_track(lp);
			}
		}
		omo_get_library_album_list(lp, artist);
	}
	else if(!strcmp(artist, "Unknown Artist"))
	{
		if(!strcmp(album, "All Albums"))
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
					if(val)
					{
						if(!strcmp(val, artist))
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
					else
					{
						lp->song_entry[lp->song_entry_count] = i;
						lp->song_entry_count++;
					}
				}
				omo_start_library_sort();
				library_sort_by_title(lp);
			}
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			bool unknown_artist;
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					unknown_artist = true;
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
								lp->song_entry[lp->song_entry_count] = i;
								lp->song_entry_count++;
							}
						}
						else
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_title(lp);
			}
		}
		else
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
					if(val)
					{
						if(!strcmp(val, artist))
						{
							val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
							if(val)
							{
								if(!strcmp(val, album))
								{
									lp->song_entry[lp->song_entry_count] = i;
									lp->song_entry_count++;
								}
							}
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_track(lp);
			}
		}
		omo_get_library_album_list(lp, artist);
	}
	else
	{
		if(!strcmp(album, "All Albums"))
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
					if(val)
					{
						if(!strcmp(val, artist))
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_title(lp);
			}
		}
		else if(!strcmp(album, "Unknown Album"))
		{
			bool artist_match;

			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					artist_match = false;
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
					if(val)
					{
						if(!strcmp(val, artist))
						{
							artist_match = true;
						}
					}
					if(artist_match)
					{
						val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
						if(val)
						{
							if(!strcmp(val, album))
							{
								lp->song_entry[lp->song_entry_count] = i;
								lp->song_entry_count++;
							}
						}
						else
						{
							lp->song_entry[lp->song_entry_count] = i;
							lp->song_entry_count++;
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_title(lp);
			}
		}
		else
		{
			lp->song_entry = malloc(sizeof(unsigned long) * lp->entry_count);
			lp->song_entry_count = 0;
			if(lp->song_entry)
			{
				for(i = 0; i < lp->entry_count; i++)
				{
					val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Artist");
					if(val)
					{
						if(!strcmp(val, artist))
						{
							val = omo_get_database_value(lp->entry_database, lp->entry[i]->id, "Album");
							if(val)
							{
								if(!strcmp(val, album))
								{
									lp->song_entry[lp->song_entry_count] = i;
									lp->song_entry_count++;
								}
							}
						}
					}
				}
				omo_start_library_sort();
				library_sort_by_track(lp);
			}
		}
		omo_get_library_album_list(lp, artist);
	}

	return true;
}

void omo_start_library_sort(void)
{
	library_sort_cancelled = false;
}

void omo_cancel_library_sort(void)
{
	library_sort_cancelled = true;
}
