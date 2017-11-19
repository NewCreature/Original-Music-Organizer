#include "gme/gme.h"
#include "t3f/t3f.h"

#include "../codec_handler.h"

static int buf_size = 1024;

typedef struct
{
	Music_Emu * emu;
	gme_info_t * info;
	bool paused;
	ALLEGRO_THREAD * codec_thread;
	ALLEGRO_AUDIO_STREAM * codec_stream;
	ALLEGRO_MUTEX * codec_mutex;
	int start_track;
	char track_buffer[16];
	char tag_buffer[1024];

} CODEC_DATA;

static void * gme_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	short *fragment;

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_audio_stream_event_source(codec_data->codec_stream));

	while(1)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (short *)al_get_audio_stream_fragment(codec_data->codec_stream);
			if(fragment)
			{
				if(codec_data->paused)
				{
					memset(fragment, 0, sizeof(short) * buf_size * 2);
				}
				else
				{
					al_lock_mutex(codec_data->codec_mutex);
					gme_play(codec_data->emu, buf_size * 2, fragment);
					al_unlock_mutex(codec_data->codec_mutex);
				}
				if(!al_set_audio_stream_fragment(codec_data->codec_stream, fragment))
				{
				}
			}
		}
		if(al_get_thread_should_stop(thread))
		{
			break;
		}
	}

	al_destroy_event_queue(queue);

	return NULL;
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->start_track = 0;
		if(subfn)
		{
			data->start_track = atoi(subfn);
		}

		if(!gme_open_file(fn, &(data->emu), 44100))
		{
			gme_track_info(data->emu, &(data->info), data->start_track);
			if(!data->info)
			{
				gme_delete(data->emu);
				free(data);
				return NULL;
			}
		}
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->info)
	{
		gme_free_info(codec_data->info);
	}
	gme_delete(codec_data->emu);
	codec_data->emu = NULL;
	free(data);
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->info->length = ((loop_start + loop_end) * 1000.0) * loop_count;

	return true;
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->info)
	{
		if(!strcmp(name, "Album"))
		{
			if(strlen(codec_data->info->game))
			{
				return codec_data->info->game;
			}
		}
		else if(!strcmp(name, "Artist"))
		{
			if(strlen(codec_data->info->author))
			{
				return codec_data->info->author;
			}
		}
		else if(!strcmp(name, "Album"))
		{
			if(strlen(codec_data->info->game))
			{
				return codec_data->info->game;
			}
		}
		else if(!strcmp(name, "Title"))
		{
			if(strlen(codec_data->info->song))
			{
				return codec_data->info->song;
			}
		}
		else if(!strcmp(name, "Copyright"))
		{
			if(strlen(codec_data->info->copyright))
			{
				return codec_data->info->copyright;
			}
		}
		else if(!strcmp(name, "Comment"))
		{
			if(strlen(codec_data->info->comment))
			{
				return codec_data->info->comment;
			}
		}
		else if(!strcmp(name, "Track"))
		{
			if(gme_track_count(codec_data->emu) > 1)
			{
				sprintf(codec_data->track_buffer, "%d", codec_data->start_track + 1);
			}
			return codec_data->track_buffer;
		}
		else if(!strcmp(name, "Loop Start"))
		{
			if(codec_data->info->intro_length <= 0)
			{
				return NULL;
			}
			sprintf(codec_data->tag_buffer, "%f", (double)codec_data->info->intro_length / 1000.0);
			return codec_data->tag_buffer;
		}
		else if(!strcmp(name, "Loop End"))
		{
			if(codec_data->info->loop_length <= 0)
			{
				return NULL;
			}
			sprintf(codec_data->tag_buffer, "%f", (double)codec_data->info->length / 1000.0);
			return codec_data->tag_buffer;
		}
		else if(!strcmp(name, "Fade Time"))
		{
			if(codec_data->info->loop_length <= 0)
			{
				return NULL;
			}
			sprintf(codec_data->tag_buffer, "0.0");
			return codec_data->tag_buffer;
		}
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	Music_Emu * local_emu = NULL;
	int count = 0;

	gme_open_file(fn, &local_emu, 44100);
	if(local_emu)
	{
		count = gme_track_count(local_emu);
		gme_delete(local_emu);
	}
	return count;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	gme_start_track(codec_data->emu, codec_data->start_track);

	/* calculate track length */
	if(codec_data->info->length <= 0)
	{
		codec_data->info->length = codec_data->info->intro_length + codec_data->info->loop_length * 2;
	}
	if(codec_data->info->length <= 0)
	{
		codec_data->info->length = (long) (2.5 * 60 * 1000);
	}
	gme_set_fade(codec_data->emu, codec_data->info->length);
	codec_data->paused = false;

	codec_data->codec_stream = al_create_audio_stream(4, buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	if(codec_data->codec_stream)
	{
		al_attach_audio_stream_to_mixer(codec_data->codec_stream, al_get_default_mixer());
		codec_data->codec_thread = al_create_thread(gme_update_thread, codec_data);
		if(codec_data->codec_thread)
		{
			codec_data->codec_mutex = al_create_mutex();
			if(codec_data->codec_mutex)
			{
				al_start_thread(codec_data->codec_thread);
				return true;
			}
		}
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->paused = true;
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->paused = false;
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_destroy_thread(codec_data->codec_thread);
	codec_data->codec_thread = NULL;
	al_destroy_mutex(codec_data->codec_mutex);
	codec_data->codec_mutex = NULL;
	al_destroy_audio_stream(codec_data->codec_stream);
	codec_data->codec_stream = NULL;
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	gme_err_t ret;

	al_lock_mutex(codec_data->codec_mutex);
	ret = gme_seek(codec_data->emu, pos * 1000.0);
	al_unlock_mutex(codec_data->codec_mutex);

	return !ret;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return (double)gme_tell(codec_data->emu) / 1000.0;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return (double)codec_data->info->length / 1000.0 + 8.0;
}

/*static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
} */

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(gme_track_ended(codec_data->emu))
	{
		return true;
	}
	return false;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_gme_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.set_loop = codec_set_loop;
	codec_handler.get_tag = codec_get_tag;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = codec_seek;
	codec_handler.get_position = codec_get_position;
	codec_handler.get_length = codec_get_length;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".ay");
	omo_codec_handler_add_type(&codec_handler, ".gbs");
	omo_codec_handler_add_type(&codec_handler, ".gym");
	omo_codec_handler_add_type(&codec_handler, ".hes");
	omo_codec_handler_add_type(&codec_handler, ".kss");
	omo_codec_handler_add_type(&codec_handler, ".nsf");
	omo_codec_handler_add_type(&codec_handler, ".sap");
	omo_codec_handler_add_type(&codec_handler, ".spc");
	omo_codec_handler_add_type(&codec_handler, ".sp1");
	omo_codec_handler_add_type(&codec_handler, ".sp2");
	omo_codec_handler_add_type(&codec_handler, ".sp3");
	omo_codec_handler_add_type(&codec_handler, ".sp4");
	omo_codec_handler_add_type(&codec_handler, ".sp5");
	omo_codec_handler_add_type(&codec_handler, ".sp6");
	omo_codec_handler_add_type(&codec_handler, ".sp7");
	omo_codec_handler_add_type(&codec_handler, ".sp8");
	omo_codec_handler_add_type(&codec_handler, ".sp9");
	omo_codec_handler_add_type(&codec_handler, ".vgm");
	omo_codec_handler_add_type(&codec_handler, ".vg");
	omo_codec_handler_add_type(&codec_handler, ".vgz");
	return &codec_handler;
}
