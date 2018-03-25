#include "t3f/t3f.h"

#include <adplug/adplug.h>
#include <adplug/emuopl.h>

#include "../codec_handler.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static int buf_size = 1024;

typedef struct
{
	Copl * opl_emu;
	CPlayer * player;
	ALLEGRO_AUDIO_STREAM * codec_stream;
	ALLEGRO_THREAD * codec_thread;
	ALLEGRO_MUTEX * codec_mutex;
	int subsong;
	unsigned long sample_count;
	bool paused;
	bool done;
	float elapsed_time;
	int ticker;

	char player_filename[1024];
	char codec_file_extension[16];
	char codec_tag_buffer[1024];
	char tag_name[32];
	bool loop;         // let us know we are looping this song
	double loop_start;
	double loop_end;
	double fade_time;  // how long to fade out
	int loop_count;    // how many times we want to loop
	int current_loop;  // how many times have we looped
	double last_pos;   // compare last_pos to current stream pos to detect loop
	bool fade_out;     // let us know we are fading out
	double fade_start; // time the fade began
	float volume;
	char info[256];

} CODEC_DATA;

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * data;

	printf("break 1\n");
	data = (CODEC_DATA *)malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->opl_emu = new CEmuopl(44100, 1, 1);
		if(!data->opl_emu)
		{
			free(data);
			printf("break 2\n");
			return NULL;
		}
		data->player = CAdPlug::factory(fn, data->opl_emu);
		if(data->player)
		{
			data->volume = 1.0;
			if(subfn)
			{
				data->subsong = atoi(subfn);
			}
			else
			{
				data->subsong = 0;
			}
		}
		else
		{
			free(data);
			printf("break 3\n");
			return NULL;
		}
	}
	printf("break 4\n");
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	delete codec_data->player;
	delete codec_data->opl_emu;
	free(data);
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(strcmp(name, "Title"))
	{
		return codec_data->player->gettitle().c_str();
	}
	else if(strcmp(name, "Artist"))
	{
		return codec_data->player->getauthor().c_str();
	}
	else if(strcmp(name, "Comment"))
	{
		return codec_data->player->getdesc().c_str();
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	Copl * opl_emu;
	CPlayer * player;
	int track_count = 1;

	opl_emu = new CEmuopl(44100, 1, 1);
	if(!opl_emu)
	{
		return 1;
	}
	player = CAdPlug::factory(fn, opl_emu);
	if(player)
	{
		track_count = player->getsubsongs();
	}
	else
	{
		delete opl_emu;
		return 1;
	}
	delete player;
	delete opl_emu;

	return track_count;
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	return false;
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

static int write_audio(void * data, char * bytes)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	// fill sound buffer
	long towrite = buf_size;
	char *sndbufpos = bytes;
	long toadd = 0;
	bool stopped;
	int sampsize = 4;

	while(towrite > 0)
	{
		while(toadd < 0)
		{
			toadd += 44100;
			codec_data->ticker++;
			stopped = !codec_data->player->update();
			codec_data->elapsed_time += 1000 / codec_data->player->getrefresh();
		}

		long i = MIN(towrite,(long)(toadd / codec_data->player->getrefresh() + 4) & ~3);

		codec_data->opl_emu->update((short *)sndbufpos, i);

		sndbufpos += i * sampsize;
		towrite -= i;
		i = (long)(i * codec_data->player->getrefresh());
		toadd -= MAX(1, i);
	}
	return 1;
}

static void * adplug_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_TIMER * timer;
	float refresh_rate;
	char * fragment;
	double last_update = 0.0;
	double elapsed_time = 0.0;
	bool done = false;

	al_lock_mutex(codec_data->codec_mutex);
	refresh_rate = codec_data->player->getrefresh();
	al_unlock_mutex(codec_data->codec_mutex);
	timer = al_create_timer(1.0 / 1000.0);
	if(!timer)
	{
		return NULL;
	}
	al_start_timer(timer);
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_audio_stream_event_source(codec_data->codec_stream));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	while(!done)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (char *)al_get_audio_stream_fragment(codec_data->codec_stream);
			if(fragment)
			{
				if(codec_data->paused)
				{
					memset(fragment, 0, sizeof(short) * buf_size * 2);
				}
				else
				{
					al_lock_mutex(codec_data->codec_mutex);
					write_audio(codec_data, fragment);
					codec_data->sample_count += buf_size;
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
	al_destroy_timer(timer);

	return NULL;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->player->rewind(codec_data->subsong);
	codec_data->codec_stream = al_create_audio_stream(4, buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	codec_data->sample_count = 0;
	codec_data->paused = false;
	codec_data->done = false;
	if(codec_data->codec_stream)
	{
		al_set_audio_stream_gain(codec_data->codec_stream, codec_data->volume);
		al_attach_audio_stream_to_mixer(codec_data->codec_stream, al_get_default_mixer());
		codec_data->codec_thread = al_create_thread(adplug_update_thread, codec_data);
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

	if(al_set_audio_stream_playing(codec_data->codec_stream, false))
	{
		return true;
	}
	return false;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(al_set_audio_stream_playing(codec_data->codec_stream, true))
	{
		return true;
	}
	return false;
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

	al_lock_mutex(codec_data->codec_mutex);
	codec_data->player->seek(pos * 1000.0);
	codec_data->sample_count = pos * 44100.0 * 2.0;
	al_unlock_mutex(codec_data->codec_mutex);
	return true;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return (codec_data->sample_count / 2) / 44100.0;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->player->songlength(codec_data->subsong) / 1000.0;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	float volume;

	if(codec_data->codec_stream)
	{
		if(!codec_data->player->update())
		{
			return true;
		}
	}
	return false;
}

static const char * codec_get_info(void * data)
{
	return "AdPlug";
}

static OMO_CODEC_HANDLER codec_handler;

extern "C" OMO_CODEC_HANDLER * omo_codec_adplug_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.get_tag = codec_get_tag;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.set_loop = codec_set_loop;
	codec_handler.set_volume = codec_set_volume;
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
	omo_codec_handler_add_type(&codec_handler, ".cmf");
	return &codec_handler;
}
