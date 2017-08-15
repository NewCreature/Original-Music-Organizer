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
	int i, c;
	const char * val;
	const char * target_fn;
	char buf[32] = {0};

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
			sprintf(buf, "%d", c);
			al_set_config_value(app->library->file_database, fn, "archive_files", buf);
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
				target_fn = archive_handler->get_file(fn, i);
				al_set_config_value(app->library->file_database, fn, buf, target_fn);
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
			if(codec_handler)
			{
				total_files++;
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			total_files += codec_handler->get_track_count(fn);
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
	int i, c = 0;
	char buf[32] = {0};
	const char * val;
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
				omo_add_file_to_library(app->library, fn, buf);
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			c = codec_handler->get_track_count(fn);
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_library(app->library, fn, c > 1 ? buf : NULL);
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
	int i, c = 0;
	char buf[32] = {0};
	const char * val;
	const char * target_fn;

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
				omo_add_file_to_queue(app->player->queue, fn, buf);
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			c = codec_handler->get_track_count(fn);
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_queue(app->player->queue, fn, c > 1 ? buf : NULL);
			}
		}
	}

    return false;
}
