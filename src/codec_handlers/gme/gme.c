#include "gme/gme.h"
#include "t3f/t3f.h"

#include "../codec_handler.h"

static OMO_CODEC_HANDLER codec_handler;
static Music_Emu * emu = NULL;
static bool paused = false;
static int buf_size = 1024;
static ALLEGRO_THREAD * codec_thread = NULL;
static ALLEGRO_AUDIO_STREAM * codec_stream = NULL;
static int start_track;

static void * gme_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	ALLEGRO_EVENT_QUEUE * queue;
	short *fragment;

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_audio_stream_event_source(codec_stream));

	while(1)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (short *)al_get_audio_stream_fragment(codec_stream);
			if(fragment)
			{
				if(paused)
				{
					memset(fragment, 0, sizeof(short) * buf_size * 2);
				}
				else
				{
					gme_play(emu, buf_size * 2, fragment);
				}
				if(!al_set_audio_stream_fragment(codec_stream, fragment))
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

static bool codec_load_file(const char * fn, const char * subfn)
{
	start_track = 0;
	if(subfn)
	{
		start_track = atoi(subfn);
	}

	gme_open_file(fn, &emu, 44100);
	return true;
}

static int codec_get_track_count(const char * fn)
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

static bool codec_play(void)
{
	gme_info_t * track_info;
	int l;

	gme_start_track(emu, start_track);

	gme_track_info(emu, &track_info, start_track);

	/* calculate track length */
	if(track_info->length <= 0)
	{
		track_info->length = track_info->intro_length + track_info->loop_length * 2;
	}
	if(track_info->length <= 0)
	{
		track_info->length = (long) (2.5 * 60 * 1000);
	}
	gme_set_fade(emu, track_info->length);
	l = track_info->length / 1000;
	gme_free_info(track_info);

	codec_stream = al_create_audio_stream(4, buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	if(codec_stream)
	{
		al_attach_audio_stream_to_mixer(codec_stream, al_get_default_mixer());
		codec_thread = al_create_thread(gme_update_thread, NULL);
		if(codec_thread)
		{
			al_start_thread(codec_thread);
			return true;
		}
	}
	return false;
}

static bool codec_pause(void)
{
	paused = true;
	return true;
}

static bool codec_resume(void)
{
	paused = false;
	return true;
}

static void codec_stop(void)
{
	al_join_thread(codec_thread, NULL);
	codec_thread = NULL;
	al_destroy_audio_stream(codec_stream);
	codec_stream = NULL;
	gme_delete(emu);
	emu = NULL;
}

/*static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
} */

static bool codec_done_playing(void)
{
	if(gme_track_ended(emu))
	{
		return true;
	}
	return false;
}

OMO_CODEC_HANDLER * omo_codec_gme_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = NULL;
	codec_handler.get_position = NULL;
	codec_handler.get_length = NULL;
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
	return &codec_handler;
}
