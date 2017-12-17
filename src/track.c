#include "t3f/t3f.h"
#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "track.h"

OMO_TRACK * omo_load_track(OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, const char * fn, const char * subfn, const char * track, ALLEGRO_PATH * temp_path)
{
	OMO_ARCHIVE_HANDLER * archive_handler;
	void * archive_handler_data;
	const char * subfile;
	char fn_buffer[1024] = {0};
	OMO_TRACK * tp;

	tp = malloc(sizeof(OMO_TRACK));
	memset(tp, 0, sizeof(OMO_TRACK));

	archive_handler = omo_get_archive_handler(archive_handler_registry, fn);
	if(archive_handler)
	{
		al_stop_timer(t3f_timer);
		archive_handler_data = archive_handler->open_archive(fn, temp_path);
		al_start_timer(t3f_timer);
		if(archive_handler_data)
		{
			if(subfn)
			{
				tp->codec_handler = omo_get_codec_handler(codec_handler_registry, archive_handler->get_file(archive_handler_data, atoi(subfn), fn_buffer));
				if(tp->codec_handler)
				{
					al_stop_timer(t3f_timer);
					subfile = archive_handler->extract_file(archive_handler_data, atoi(subfn), fn_buffer);
					al_start_timer(t3f_timer);
					if(strlen(subfile) > 0)
					{
						strcpy(tp->extracted_filename, subfile);
						al_stop_timer(t3f_timer);
						tp->codec_data = tp->codec_handler->load_file(tp->extracted_filename, track);
						al_start_timer(t3f_timer);
					}
				}
			}
			archive_handler->close_archive(archive_handler_data);
		}
	}
	else
	{
		tp->codec_handler = omo_get_codec_handler(codec_handler_registry, fn);
		if(tp->codec_handler)
		{
			al_stop_timer(t3f_timer);
			tp->codec_data = tp->codec_handler->load_file(fn, track);
			al_start_timer(t3f_timer);
		}
	}
	return tp;
}

void omo_unload_track(OMO_TRACK * tp)
{
	if(tp->codec_handler)
	{
		if(tp->codec_handler->unload_file)
		{
			tp->codec_handler->unload_file(tp->codec_data);
		}

		/* delete previously extracted file if we played from archive */
		if(strlen(tp->extracted_filename) > 0)
		{
			al_remove_filename(tp->extracted_filename);
		}
	}
}
