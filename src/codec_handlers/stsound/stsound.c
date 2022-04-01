#include "t3f/t3f.h"
#include "StSoundLibrary/StSoundLibrary.h"

#include "../codec_handler.h"

static int buf_size = 1024;

typedef struct
{

	YMMUSIC * music_data;
	ymMusicInfo_t info;
	ALLEGRO_THREAD * codec_thread;
	ALLEGRO_AUDIO_STREAM * codec_stream;
	ALLEGRO_MUTEX * codec_mutex;
	char info_buffer[256];
	float volume;

} CODEC_DATA;

static void * stsound_update_thread(ALLEGRO_THREAD * thread, void * arg)
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
				ymMusicCompute(codec_data->music_data, fragment, buf_size);
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
	if(!data)
	{
		goto fail;
	}
	memset(data, 0, sizeof(CODEC_DATA));
	data->music_data = ymMusicCreate();
	if(!data->music_data)
	{
		goto fail;
	}
	if(!ymMusicLoad(data->music_data, fn))
	{
		goto fail;
	}
	ymMusicGetInfo(data->music_data, &data->info);
	data->volume = 1.0;

	return data;

	fail:
	{
		if(data)
		{
			if(data->music_data)
			{
				ymMusicDestroy(data->music_data);
			}
			free(data);
		}
		return NULL;
	}
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	ymMusicDestroy(codec_data->music_data);
	free(data);
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->volume = volume;
	if(codec_data->codec_stream)
	{
		al_set_audio_stream_gain(codec_data->codec_stream, codec_data->volume);
	}
	return true;
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(!strcmp(name, "Artist"))
	{
		if(strlen(codec_data->info.pSongAuthor))
		{
			return codec_data->info.pSongAuthor;
		}
	}
	else if(!strcmp(name, "Title"))
	{
		if(strlen(codec_data->info.pSongName))
		{
			return codec_data->info.pSongName;
		}
	}
	else if(!strcmp(name, "Comment"))
	{
		if(strlen(codec_data->info.pSongComment))
		{
			return codec_data->info.pSongComment;
		}
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	ymMusicPlay(codec_data->music_data);

	codec_data->codec_stream = al_create_audio_stream(4, buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1);
	if(codec_data->codec_stream)
	{
		al_set_audio_stream_gain(codec_data->codec_stream, codec_data->volume);
		al_attach_audio_stream_to_mixer(codec_data->codec_stream, al_get_default_mixer());
		codec_data->codec_thread = al_create_thread(stsound_update_thread, codec_data);
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

	ymMusicPause(codec_data->music_data);

	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	ymMusicPlay(codec_data->music_data);

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
	bool ret;

	al_lock_mutex(codec_data->codec_mutex);
	if(ymMusicIsSeekable(codec_data->music_data))
	{
		ymMusicSeek(codec_data->music_data, pos * 1000.0);
		ret = true;
	}
	else
	{
		ret = false;
	}
	al_unlock_mutex(codec_data->codec_mutex);

	return ret;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	float pos;

	al_lock_mutex(codec_data->codec_mutex);
	pos = ymMusicGetPos(codec_data->music_data) / 1000.0;
	al_unlock_mutex(codec_data->codec_mutex);
	return pos;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->info.musicTimeInMs / 1000.0;
}

/*static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
} */

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	bool ret;

	al_lock_mutex(codec_data->codec_mutex);
	ret = ymMusicIsOver(codec_data->music_data);
	al_unlock_mutex(codec_data->codec_mutex);

	return ret;
}

static const char * codec_get_info(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	sprintf(codec_data->info_buffer, "StSound (%s - %s)", codec_data->info.pSongType, codec_data->info.pSongPlayer);
	return codec_data->info_buffer;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_stsound_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	strcpy(codec_handler.id, "StSound");
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.set_loop = NULL;
	codec_handler.set_length = NULL;
	codec_handler.set_volume = codec_set_volume;
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
	codec_handler.get_info = codec_get_info;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".ym");
	return &codec_handler;
}
