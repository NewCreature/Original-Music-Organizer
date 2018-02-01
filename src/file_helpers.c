#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "file_helpers.h"
#include "profile.h"

static bool omo_process_file_mtime(const char * fn, bool isfolder, void * data)
{
	ALLEGRO_FS_ENTRY * fs_entry;
	time_t * mtime = (time_t *)data;
	time_t new_mtime;

	if(isfolder)
	{
		fs_entry = al_create_fs_entry(fn);
		if(fs_entry)
		{
			new_mtime = al_get_fs_entry_mtime(fs_entry);
			if(new_mtime > *mtime)
			{
				*mtime = new_mtime;
			}
		}
	}
	return false;
}

time_t omo_get_folder_mtime(const char * path)
{
	time_t mtime = 0;

	t3f_scan_files(path, omo_process_file_mtime, false, &mtime);

	return mtime;
}

void omo_setup_file_helper_data(OMO_FILE_HELPER_DATA * fhdp, OMO_ARCHIVE_HANDLER_REGISTRY * ahrp, OMO_CODEC_HANDLER_REGISTRY * chrp, OMO_LIBRARY * lp, OMO_QUEUE * qp, ALLEGRO_PATH * temp_path, void * user_data)
{
	fhdp->archive_handler_registry = ahrp;
	fhdp->codec_handler_registry = chrp;
	fhdp->library = lp;
	fhdp->queue = qp;
	fhdp->file_count = 0;
	fhdp->temp_path = temp_path;
	fhdp->user_data = user_data;
	fhdp->cancel_scan = false;
	fhdp->scan_done = false;
}

static bool file_needs_scan(const char * fn, OMO_LIBRARY * lp)
{
	ALLEGRO_FS_ENTRY * fs_entry;
	const char * val;
	time_t file_time;
	bool ret = false;

	if(lp)
	{
		val = omo_get_database_value(lp->file_database, fn, "file_time");
		if(val)
		{
			file_time = atol(val);
			fs_entry = al_create_fs_entry(fn);
			if(fs_entry)
			{
				if(file_time < al_get_fs_entry_mtime(fs_entry))
				{
					ret = true;
				}
				al_destroy_fs_entry(fs_entry);
			}
		}
		else
		{
			ret = true;
		}
	}
	else
	{
		ret = true;
	}
	return ret;
}

static int omo_get_archive_file_count(const char * fn, OMO_FILE_HELPER_DATA * file_helper_data)
{
	OMO_CODEC_HANDLER * codec_handler;
	char buf[32] = {0};
	int c = 0;
	int file_count = 0;
	const char * val;
	int i;

	if(file_helper_data->library)
	{
		val = omo_get_database_value(file_helper_data->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		else
		{
			return -1;
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = omo_get_database_value(file_helper_data->library->file_database, fn, buf);
			if(val)
			{
				codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, val);
				if(codec_handler)
				{
					sprintf(buf, "entry_%d_tracks", i);
					val = omo_get_database_value(file_helper_data->library->file_database, fn, buf);
					if(val)
					{
						file_count += atoi(val);
					}
					else
					{
						file_count += 1;
					}
				}
			}
		}
		return file_count;
	}
	return -1;
}

static void omo_count_archive_files(const char * fn, OMO_ARCHIVE_HANDLER * archive_handler, OMO_FILE_HELPER_DATA * file_helper_data)
{
	ALLEGRO_FS_ENTRY * fs_entry;
	OMO_CODEC_HANDLER * codec_handler;
	void * archive_handler_data = NULL;
	time_t file_time;
	char buf[32] = {0};
	char buf2[32] = {0};
	char fn_buffer[1024] = {0};
	const char * val;
	const char * target_fn = NULL;
	const char * extracted_fn;
	bool need_scan = false;
	int c = 0;
	int c2 = 0;
	int i;

	val = NULL;
	need_scan = file_needs_scan(fn, file_helper_data->library);
	if(!need_scan)
	{
		c = omo_get_archive_file_count(fn, file_helper_data);
		if(c >= 0)
		{
			file_helper_data->file_count += c;
			return;
		}
	}
	archive_handler_data = archive_handler->open_archive(fn, file_helper_data->temp_path);
	if(archive_handler_data)
	{
		if(file_helper_data->library)
		{
			fs_entry = al_create_fs_entry(fn);
			if(fs_entry)
			{
				file_time = al_get_fs_entry_mtime(fs_entry);
				sprintf(buf, "%lu", file_time);
				omo_set_database_value(file_helper_data->library->file_database, fn, "file_time", buf);
				al_destroy_fs_entry(fs_entry);
			}
		}
		c = archive_handler->count_files(archive_handler_data);
		if(file_helper_data->library)
		{
			sprintf(buf, "%d", c);
			omo_set_database_value(file_helper_data->library->file_database, fn, "archive_files", buf);
		}
		for(i = 0; i < c; i++)
		{
			target_fn = archive_handler->get_file(archive_handler_data, i, fn_buffer);
			if(file_helper_data->library)
			{
				sprintf(buf, "entry_%d", i);
				omo_set_database_value(file_helper_data->library->file_database, fn, buf, target_fn);
			}
			codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, target_fn);
			if(codec_handler)
			{
				extracted_fn = archive_handler->extract_file(archive_handler_data, i, fn_buffer);
				if(extracted_fn)
				{
					c2 = codec_handler->get_track_count(NULL, extracted_fn);
					al_remove_filename(extracted_fn);
					if(file_helper_data->library)
					{
						sprintf(buf, "entry_%d_tracks", i);
						sprintf(buf2, "%d", c2);
						omo_set_database_value(file_helper_data->library->file_database, fn, buf, buf2);
					}
				}
			}
			file_helper_data->file_count += c2;
		}
	}
}

static const char * get_fn(const char * fn)
{
	int i;

	for(i = strlen(fn) - 1; i >= 0; i--)
	{
		if(fn[i] == '/' || fn[i] == '\\')
		{
			return &fn[i + 1];
		}
	}
	return fn;
}

bool omo_count_file(const char * fn, bool isfolder, void * data)
{
	OMO_FILE_HELPER_DATA * file_helper_data = (OMO_FILE_HELPER_DATA *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	char * message = (char *)file_helper_data->user_data;
	int c2 = 0;
	const char * val;
	char buf2[32] = {0};

	if(isfolder)
	{
		return false;
	}
	if(file_helper_data->cancel_scan)
	{
		return false;
	}
	archive_handler = omo_get_archive_handler(file_helper_data->archive_handler_registry, fn);
	if(archive_handler)
	{
		if(message)
		{
			sprintf(message, "Scanning archive: %s", get_fn(fn));
		}
		omo_count_archive_files(fn, archive_handler, file_helper_data);
	}
	else
	{
		codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = NULL;
			if(file_helper_data->library)
			{
				val = omo_get_database_value(file_helper_data->library->file_database, fn, "tracks");
			}
			if(val)
			{
				c2 = atoi(val);
			}
			else
			{
				c2 = codec_handler->get_track_count(NULL, fn);
				if(file_helper_data->library)
				{
					sprintf(buf2, "%d", c2);
					omo_set_database_value(file_helper_data->library->file_database, fn, "tracks", buf2);
				}
			}
			file_helper_data->file_count += c2;
		}
	}

	return false;
}

bool omo_add_file(const char * fn, bool isfolder, void * data)
{
	OMO_FILE_HELPER_DATA * file_helper_data = (OMO_FILE_HELPER_DATA *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	char * message = (char *)file_helper_data->user_data;
	int i, j, c = 0, c2 = 0;
	char buf[32] = {0};
	char buf2[32] = {0};
	const char * val;
	const char * val2;
	const char * target_fn = NULL;
	int ret = 2; // don't update unless we get update signal from library function

	if(isfolder)
	{
		return false;
	}
	if(file_helper_data->cancel_scan)
	{
		return false;
	}
	if(message && file_helper_data->library)
	{
		sprintf(message, "Adding (%lu/%lu): %s", file_helper_data->library->entry_count, file_helper_data->library->entry_size, get_fn(fn));
	}
	archive_handler = omo_get_archive_handler(file_helper_data->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = omo_get_database_value(file_helper_data->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = omo_get_database_value(file_helper_data->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, target_fn);
			if(codec_handler)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				sprintf(buf2, "entry_%d_tracks", i);
				val2 = omo_get_database_value(file_helper_data->library->file_database, fn, buf2);
				if(val2)
				{
					c2 = atoi(val2);
					for(j = 0; j < c2; j++)
					{
						sprintf(buf2, "entry_%d_track_%d", i, j);
						val2 = omo_get_database_value(file_helper_data->library->file_database, fn, buf2);
						if(val2)
						{
							sprintf(buf2, "%s", val2);
						}
						else
						{
							sprintf(buf2, "%d", j);
						}
						ret = omo_add_file_to_library(file_helper_data->library, fn, buf, c2 > 1 ? buf2 : NULL, file_helper_data->archive_handler_registry, file_helper_data->codec_handler_registry, file_helper_data->temp_path);
					}
				}
				else
				{
					ret = omo_add_file_to_library(file_helper_data->library, fn, buf, NULL, file_helper_data->archive_handler_registry, file_helper_data->codec_handler_registry, file_helper_data->temp_path);
				}
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = omo_get_database_value(file_helper_data->library->file_database, fn, "tracks");
			if(val)
			{
				c = atoi(val);
			}
			else
			{
				c = codec_handler->get_track_count(NULL, fn);
			}
			for(i = 0; i < c; i++)
			{
				sprintf(buf2, "track_%d", i);
				val2 = omo_get_database_value(file_helper_data->library->file_database, fn, buf2);
				if(val2)
				{
					sprintf(buf, "%s", val2);
				}
				else
				{
					sprintf(buf, "%d", i);
				}
				ret = omo_add_file_to_library(file_helper_data->library, fn, NULL, c > 1 ? buf : NULL, file_helper_data->archive_handler_registry, file_helper_data->codec_handler_registry, file_helper_data->temp_path);
			}
		}
	}

	if(ret == 2)
	{
		return false;
	}
	return true;
}

bool omo_queue_file(const char * fn, bool isfolder, void * data)
{
	OMO_FILE_HELPER_DATA * file_helper_data = (OMO_FILE_HELPER_DATA *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	void * archive_handler_data;
	OMO_CODEC_HANDLER * codec_handler;
	int i, j, c = 0, c2 = 0;
	char buf[32] = {0};
	char buf2[32] = {0};
	const char * val;
	const char * val2;
	const char * target_fn = NULL;
	const char * extracted_fn;
	char fn_buffer[1024] = {0};

	if(isfolder)
	{
		return false;
	}
	if(file_helper_data->cancel_scan)
	{
		return false;
	}
	archive_handler = omo_get_archive_handler(file_helper_data->archive_handler_registry, fn);
	if(archive_handler)
	{
		archive_handler_data = archive_handler->open_archive(fn, file_helper_data->temp_path);
		if(archive_handler_data)
		{
			val = NULL;
			if(file_helper_data->library)
			{
				val = omo_get_database_value(file_helper_data->library->file_database, fn, "archive_files");
			}
			if(val)
			{
				c = atoi(val);
			}
			else
			{
				c = archive_handler->count_files(archive_handler_data);
			}
			for(i = 0; i < c; i++)
			{
				val = NULL;
				if(file_helper_data->library)
				{
					sprintf(buf, "entry_%d", i);
					val = omo_get_database_value(file_helper_data->library->file_database, fn, buf);
				}
				if(val)
				{
					target_fn = val;
				}
				else
				{
					target_fn = archive_handler->get_file(archive_handler_data, i, fn_buffer);
				}
				codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, target_fn);
				if(codec_handler)
				{
					sprintf(buf, "%d", i); // reference by index instead of filename
					val2 = NULL;
					if(file_helper_data->library)
					{
						sprintf(buf2, "entry_%d_tracks", i);
						val2 = omo_get_database_value(file_helper_data->library->file_database, fn, buf2);
					}
					if(val2)
					{
						c2 = atoi(val2);
					}
					else
					{
						extracted_fn = archive_handler->extract_file(archive_handler_data, i, fn_buffer);
						if(extracted_fn)
						{
							c2 = codec_handler->get_track_count(NULL, extracted_fn);
							al_remove_filename(extracted_fn);
						}
					}
					for(j = 0; j < c2; j++)
					{
						sprintf(buf2, "track_%d", i);
						val2 = NULL;
						if(c2 > 1)
						{
							if(file_helper_data->library)
							{
								val2 = omo_get_database_value(file_helper_data->library->file_database, fn, buf2);
							}
							if(val2)
							{
								strcpy(buf2, val2);
							}
							else
							{
								sprintf(buf2, "%d", j);
							}
						}
						omo_add_file_to_queue(file_helper_data->queue, fn, buf, c2 > 1 ? buf2 : NULL, false);
					}
				}
			}
			archive_handler->close_archive(archive_handler_data);
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(file_helper_data->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = NULL;
			if(file_helper_data->library)
			{
				val = omo_get_database_value(file_helper_data->library->file_database, fn, "tracks");
			}
			if(val)
			{
				c = atoi(val);
			}
			else
			{
				c = codec_handler->get_track_count(NULL, fn);
			}
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "track_%d", i);
				val = NULL;
				if(c > 1)
				{
					if(file_helper_data->library)
					{
						val = omo_get_database_value(file_helper_data->library->file_database, fn, buf);
					}
					if(val)
					{
						strcpy(buf, val);
					}
					else
					{
						sprintf(buf, "%d", i);
					}
				}
				omo_add_file_to_queue(file_helper_data->queue, fn, NULL, c > 1 ? buf : NULL, false);
			}
		}
	}

	return false;
}

void omo_clear_library_cache(void)
{
	char buffer[1024];
	const char * fn;

	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.library", buffer, 1024);
	if(fn)
	{
		al_remove_filename(fn);
	}
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.artists", buffer, 1024);
	if(fn)
	{
		al_remove_filename(fn);
	}
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.albums", buffer, 1024);
	if(fn)
	{
		al_remove_filename(fn);
	}
	fn = omo_get_profile_path(omo_get_profile(omo_get_current_profile()), "omo.songs", buffer, 1024);
	if(fn)
	{
		al_remove_filename(fn);
	}
}
