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
	double length;
	float volume;

} CODEC_DATA;

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * data;

	data = (CODEC_DATA *)malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->opl_emu = new CEmuopl(44100, 1, 0);
		if(!data->opl_emu)
		{
			free(data);
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
			data->length = data->player->songlength(data->subsong) / 1000.0;
		}
		else
		{
			free(data);
			return NULL;
		}
	}
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
	std::string tag;

	if(!strcmp(name, "Title"))
	{
		tag = codec_data->player->gettitle();
		if(!tag.empty())
		{
			return tag.c_str();
		}
	}
	else if(!strcmp(name, "Artist"))
	{
		tag = codec_data->player->getauthor();
		if(!tag.empty())
		{
			return tag.c_str();
		}
	}
	else if(!strcmp(name, "Comment"))
	{
		tag = codec_data->player->getdesc();
		if(!tag.empty())
		{
			return tag.c_str();
		}
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

static void new_write_audio(void * data, char * bytes)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	static long minicnt = 0;
    long i, towrite = buf_size;
    char *pos = bytes;

    // Prepare audiobuf with emulator output
    while(towrite > 0)
	{
		while(minicnt < 0)
		{
			minicnt += 44100;
			codec_data->done = !codec_data->player->update();
		}
		i = MIN(towrite, (long)(minicnt / codec_data->player->getrefresh() + 4) & ~3);
		codec_data->opl_emu->update((short *)pos, i);
		pos += i * 2;
		towrite -= i;
    	i = (long)(codec_data->player->getrefresh() * i);
    	minicnt -= MAX(1, i);
	}
}

static void * adplug_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	float refresh_rate;
	char * fragment;
	bool done = false;

	al_lock_mutex(codec_data->codec_mutex);
	refresh_rate = codec_data->player->getrefresh();
	al_unlock_mutex(codec_data->codec_mutex);
	queue = al_create_event_queue();
	if(!queue)
	{
		return NULL;
	}
	al_register_event_source(queue, al_get_audio_stream_event_source(codec_data->codec_stream));

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
					memset(fragment, 0, sizeof(short) * buf_size);
				}
				else
				{
					al_lock_mutex(codec_data->codec_mutex);
					new_write_audio(codec_data, fragment);
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

	return NULL;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->player->rewind(codec_data->subsong);
	codec_data->codec_stream = al_create_audio_stream(4, buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1);
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

	al_lock_mutex(codec_data->codec_mutex);
	codec_data->player->seek(pos * 1000.0);
	codec_data->sample_count = pos * 44100.0;
	al_unlock_mutex(codec_data->codec_mutex);
	return true;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return (codec_data->sample_count) / 44100.0;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->length;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->codec_stream)
	{
		return codec_data->done;
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
	strcpy(codec_handler.id, "AdPlug");
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
//	omo_codec_handler_add_type(&codec_handler, ".a2m");
	omo_codec_handler_add_type(&codec_handler, ".adl");
	omo_codec_handler_add_type(&codec_handler, ".amd");
	omo_codec_handler_add_type(&codec_handler, ".bam");
	omo_codec_handler_add_type(&codec_handler, ".cff");
	omo_codec_handler_add_type(&codec_handler, ".cmf");
	omo_codec_handler_add_type(&codec_handler, ".d00");
	omo_codec_handler_add_type(&codec_handler, ".dfm");
	omo_codec_handler_add_type(&codec_handler, ".dmo");
	omo_codec_handler_add_type(&codec_handler, ".dro");
	omo_codec_handler_add_type(&codec_handler, ".dtm");
	omo_codec_handler_add_type(&codec_handler, ".hsc");
	omo_codec_handler_add_type(&codec_handler, ".hsp");
	omo_codec_handler_add_type(&codec_handler, ".imf");
	omo_codec_handler_add_type(&codec_handler, ".ksm");
	omo_codec_handler_add_type(&codec_handler, ".laa");
	omo_codec_handler_add_type(&codec_handler, ".lds");
	omo_codec_handler_add_type(&codec_handler, ".m");
	omo_codec_handler_add_type(&codec_handler, ".mad");
	omo_codec_handler_add_type(&codec_handler, ".mid");
	omo_codec_handler_add_type(&codec_handler, ".mkj");
	omo_codec_handler_add_type(&codec_handler, ".msc");
	omo_codec_handler_add_type(&codec_handler, ".mtk");
	omo_codec_handler_add_type(&codec_handler, ".rad");
//	omo_codec_handler_add_type(&codec_handler, ".raw");
	omo_codec_handler_add_type(&codec_handler, ".rix");
	omo_codec_handler_add_type(&codec_handler, ".rol");
	omo_codec_handler_add_type(&codec_handler, ".s3m");
	omo_codec_handler_add_type(&codec_handler, ".sa2");
	omo_codec_handler_add_type(&codec_handler, ".sat");
	omo_codec_handler_add_type(&codec_handler, ".sci");
	omo_codec_handler_add_type(&codec_handler, ".sng");
	omo_codec_handler_add_type(&codec_handler, ".xad");
	omo_codec_handler_add_type(&codec_handler, ".xms");
	omo_codec_handler_add_type(&codec_handler, ".xsm");
	omo_codec_handler_add_type(&codec_handler, ".mdi");
	return &codec_handler;
}
