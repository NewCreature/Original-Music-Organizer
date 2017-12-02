#include "t3f/t3f.h"

#include "../codec_handler.h"
#include "MP3A5/mp3a5.h"

typedef struct
{
	MP3A5_MP3 * codec_mp3;
	char tag_buffer[1024];
	bool loop;         // let us know we are looping this song
	double fade_time;  // how long to fade out
	int loop_count;    // how many times we want to loop
	int current_loop;  // how many times have we looped
	double last_pos;   // compare last_pos to current stream pos to detect loop
	bool fade_out;     // let us know we are fading out
	double fade_start; // time the fade began
	float volume;
} CODEC_DATA;

static bool codec_initialize(void)
{
	mpg123_init();
	return true;
}

static void codec_exit(void)
{
	mpg123_exit();
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->codec_mp3 = mp3a5_load_mp3(fn);
		if(!data->codec_mp3)
		{
			free(data);
			return NULL;
		}
		data->volume = 1.0;
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	mp3a5_destroy_mp3(codec_data->codec_mp3);
	codec_data->codec_mp3 = NULL;
	free(data);
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	mp3a5_set_mp3_loop(codec_data->codec_mp3, loop_start, loop_end);
	codec_data->loop = true;
	codec_data->fade_time = fade_time;
	codec_data->loop_count = loop_count;

	return true;
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	codec_data->volume = volume;
	if(codec_data->codec_mp3->audio_stream)
	{
		al_set_audio_stream_gain(codec_data->codec_mp3->audio_stream, codec_data->volume);
	}

	return true;
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->codec_mp3->tags)
	{
		if(!strcmp(name, "Artist"))
		{
			return codec_data->codec_mp3->tags->artist;
		}
		else if(!strcmp(name, "Album"))
		{
			return codec_data->codec_mp3->tags->album;
		}
		else if(!strcmp(name, "Disc"))
		{
			return codec_data->codec_mp3->tags->disc;
		}
		else if(!strcmp(name, "Track"))
		{
			return codec_data->codec_mp3->tags->track;
		}
		else if(!strcmp(name, "Title"))
		{
			return codec_data->codec_mp3->tags->title;
		}
		else if(!strcmp(name, "Genre"))
		{
			return codec_data->codec_mp3->tags->genre;
		}
		else if(!strcmp(name, "Year"))
		{
			return codec_data->codec_mp3->tags->year;
		}
		else if(!strcmp(name, "Copyright"))
		{
			return codec_data->codec_mp3->tags->copyright;
		}
		else if(!strcmp(name, "Comment"))
		{
			return codec_data->codec_mp3->tags->comment;
		}
	}
	return NULL;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(mp3a5_play_mp3(codec_data->codec_mp3, 4, 1024))
	{
		al_set_audio_stream_gain(codec_data->codec_mp3->audio_stream, codec_data->volume);
		return true;
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	mp3a5_pause_mp3(codec_data->codec_mp3);
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	mp3a5_resume_mp3(codec_data->codec_mp3);
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	mp3a5_stop_mp3(codec_data->codec_mp3);
}

/*static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
} */

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	double volume = 1.0;

	if(codec_data->codec_mp3 && codec_data->codec_mp3->audio_stream)
	{
		if(codec_data->loop)
		{
			if(mp3a5_get_position(codec_data->codec_mp3) < codec_data->last_pos)
			{
				codec_data->current_loop++;
			}
			codec_data->last_pos = mp3a5_get_position(codec_data->codec_mp3);
			if(codec_data->current_loop >= codec_data->loop_count && !codec_data->fade_out)
			{
				codec_data->fade_start = al_get_time();
				codec_data->fade_out = true;
			}
			if(codec_data->fade_out)
			{
				volume = 1.0 - (al_get_time() - codec_data->fade_start) / codec_data->fade_time;
				al_set_audio_stream_gain(codec_data->codec_mp3->audio_stream, volume);
				if(al_get_time() - codec_data->fade_start >= codec_data->fade_time)
				{
					return true;
				}
			}
		}
		if(codec_data->codec_mp3->done)
		{
			return true;
		}
		return false;
	}
	return true;
}

static const char * codec_get_info(void * data)
{
	return "MPG123";
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_mp3a5_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = codec_initialize;
	codec_handler.exit = codec_exit;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.set_loop = codec_set_loop;
	codec_handler.set_volume = codec_set_volume;
	codec_handler.get_tag = codec_get_tag;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = NULL;
	codec_handler.get_position = NULL;
	codec_handler.get_length = NULL;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.get_info = codec_get_info;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".mpg");
	omo_codec_handler_add_type(&codec_handler, ".mp2");
	omo_codec_handler_add_type(&codec_handler, ".mp3");
	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}
