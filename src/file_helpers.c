#include "t3f/t3f.h"
#include "instance.h"

static unsigned long total_files = 0;

void omo_reset_file_count(void)
{
	total_files = 0;
}

bool omo_count_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	ALLEGRO_FS_ENTRY * fs_entry;
	int i, c = 0, c2 = 0;
	time_t file_time;
	const char * val;
	const char * target_fn = NULL;
	const char * extracted_fn;
	char buf[32] = {0};
	char buf2[32] = {0};
	bool rescan = false;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "file_time");
		if(val)
		{
			file_time = atol(val);
			fs_entry = al_create_fs_entry(fn);
			if(fs_entry)
			{
				if(file_time < al_get_fs_entry_mtime(fs_entry))
				{
					rescan = true;
				}
				al_destroy_fs_entry(fs_entry);
			}
		}
		else
		{
			rescan = true;
		}
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val && !rescan)
		{
			c = atoi(val);
		}
		else
		{
			fs_entry = al_create_fs_entry(fn);
			if(fs_entry)
			{
				file_time = al_get_fs_entry_mtime(fs_entry);
				sprintf(buf, "%lu", file_time);
				al_set_config_value(app->library->file_database, fn, "file_time", buf);
				al_destroy_fs_entry(fs_entry);
			}
			c = archive_handler->count_files(fn);
			if(app->library)
			{
				sprintf(buf, "%d", c);
				al_set_config_value(app->library->file_database, fn, "archive_files", buf);
			}
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val && !rescan)
			{
				target_fn = val;
			}
			else
			{
				target_fn = archive_handler->get_file(fn, i);
				al_set_config_value(app->library->file_database, fn, buf, target_fn);
			}
			sprintf(buf, "entry_%d_tracks", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				c2 = atoi(val);
			}
			else
			{
				codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
				if(codec_handler)
				{
					extracted_fn = archive_handler->extract_file(fn, i);
					if(extracted_fn)
					{
						c2 = codec_handler->get_track_count(extracted_fn);
						al_remove_filename(extracted_fn);
						sprintf(buf2, "%d", c2);
						al_set_config_value(app->library->file_database, fn, buf, buf2);
					}
				}
			}
			total_files += c2;
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = al_get_config_value(app->library->file_database, fn, "tracks");
			if(val)
			{
				c2 = atoi(val);
			}
			else
			{
				c2 = codec_handler->get_track_count(fn);
				sprintf(buf2, "%d", c2);
				al_set_config_value(app->library->file_database, fn, "tracks", buf2);
			}
			total_files += c2;
		}
	}

    return false;
}

unsigned long omo_get_file_count(void)
{
	return total_files;
}

bool omo_add_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i, j, c = 0, c2 = 0;
	char buf[32] = {0};
	char buf2[32] = {0};
	const char * val;
	const char * val2;
	const char * target_fn = NULL;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
			if(codec_handler)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				sprintf(buf2, "entry_%d_tracks", i);
				val2 = al_get_config_value(app->library->file_database, fn, buf2);
				if(val2)
				{
					c2 = atoi(val);
					for(j = 0; j < c2; j++)
					{
						sprintf(buf2, "%d", j);
						omo_add_file_to_library(app->library, fn, buf, c2 > 1 ? buf2 : NULL, app->archive_handler_registry, app->codec_handler_registry);
					}
				}
				else
				{
					omo_add_file_to_library(app->library, fn, buf, NULL, app->archive_handler_registry, app->codec_handler_registry);
				}
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = al_get_config_value(app->library->file_database, fn, "tracks");
			if(val)
			{
				c = atoi(val);
			}
			else
			{
				c = codec_handler->get_track_count(fn);
			}
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_library(app->library, fn, NULL, c > 1 ? buf : NULL, app->archive_handler_registry, app->codec_handler_registry);
			}
		}
	}

    return false;
}

bool omo_queue_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i, j, c = 0, c2 = 0;
	char buf[32] = {0};
	char buf2[32] = {0};
	const char * val;
	const char * val2;
	const char * target_fn = NULL;
	const char * extracted_fn;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		else
		{
			c = archive_handler->count_files(fn);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			else
			{
				target_fn = archive_handler->get_file(fn, c);
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
			if(codec_handler)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				sprintf(buf2, "entry_%d_tracks", i);
				val2 = al_get_config_value(app->library->file_database, fn, buf2);
				if(val2)
				{
					c2 = atoi(val2);
				}
				else
				{
					extracted_fn = archive_handler->extract_file(fn, i);
					if(extracted_fn)
					{
						c2 = codec_handler->get_track_count(extracted_fn);
			            al_remove_filename(extracted_fn);
					}
				}
				for(j = 0; j < c2; j++)
				{
					sprintf(buf2, "%d", j);
					omo_add_file_to_queue(app->player->queue, fn, buf, c2 > 1 ? buf2 : NULL);
				}
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			val = al_get_config_value(app->library->file_database, fn, "tracks");
			if(val)
			{
				c = atoi(val);
			}
			else
			{
				c = codec_handler->get_track_count(fn);
			}
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_queue(app->player->queue, fn, NULL, c > 1 ? buf : NULL);
			}
		}
	}

    return false;
}
