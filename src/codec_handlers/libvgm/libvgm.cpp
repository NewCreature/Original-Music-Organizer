#include "t3f/t3f.h"

#include <vgm/common_def.h>
#include <vgm/utils/DataLoader.h>
#include <vgm/utils/FileLoader.h>
#include <vgm/utils/MemoryLoader.h>
#include <vgm/player/playerbase.hpp>
#include <vgm/player/vgmplayer.hpp>
#include <vgm/player/playera.hpp>
//#include <vgm/audio/AudioStream.h>
//#include <vgm/audio/AudioStream_SpcDrvFuns.h>
#include <vgm/emu/Resampler.h>
#include <vgm/emu/SoundDevs.h>	// for DEVID_*
#include <vgm/emu/EmuCores.h>
#include <vgm/utils/OSMutex.h>

#include "../codec_handler.h"

static const int _libvgm_buf_size = 4096;

typedef struct
{
	DATA_LOADER * dLoad;
	PlayerA * player_handler;
	PlayerA::Config config;
	PlayerBase * player;
	VGMPlayer * vgm_player;
	const VGM_HEADER * vgm_header;
	ALLEGRO_AUDIO_STREAM * codec_stream;
	ALLEGRO_THREAD * codec_thread;
	ALLEGRO_MUTEX * codec_mutex;
	bool paused;
	float volume;
	unsigned long sample_count;
	double fade_time;
	int loop_count;
	double length;
	const char * tag_title;
	const char * tag_artist;
	const char * tag_album;
	const char * tag_system;
	const char * tag_date;
	const char * tag_comment;
	char tag_loop_start[64];
	char tag_loop_end[64];
	char tag_fade_time[64];
} CODEC_DATA;

static void load_tags(CODEC_DATA * codec_data)
{
	const char* const* tagList = codec_data->player->GetTags();
	for (const char* const* t = tagList; *t; t += 2)
	{
		if (!strcmp(t[0], "TITLE"))
			codec_data->tag_title = t[1];
		else if (!strcmp(t[0], "ARTIST"))
			codec_data->tag_artist = t[1];
		else if (!strcmp(t[0], "GAME"))
			codec_data->tag_album = t[1];
		else if (!strcmp(t[0], "SYSTEM"))
			codec_data->tag_system = t[1];
		else if (!strcmp(t[0], "DATE"))
			codec_data->tag_date = t[1];
		else if (!strcmp(t[0], "COMMENT"))
			codec_data->tag_comment = t[1];
	}
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)malloc(sizeof(CODEC_DATA));

	if(codec_data)
	{
		memset(codec_data, 0, sizeof(CODEC_DATA));
		codec_data->player_handler = new PlayerA;
		if(!codec_data->player_handler)
		{
			goto fail;
		}
		codec_data->fade_time = 8.0; // default to 8 second fade
		codec_data->loop_count = 1; // default to 1 loop (play once)
		codec_data->length = -1.0; // cache length for faster access
		codec_data->player_handler->RegisterPlayerEngine(new VGMPlayer);
		codec_data->dLoad = FileLoader_Init(fn);
		if(!codec_data->dLoad)
		{
			goto fail;
		}
		DataLoader_SetPreloadBytes(codec_data->dLoad, 0x100);
		if(DataLoader_Load(codec_data->dLoad))
		{
			goto fail;
		}
		if(codec_data->player_handler->LoadFile(codec_data->dLoad))
		{
			goto fail;
		}
		codec_data->player = codec_data->player_handler->GetPlayer();
		codec_data->vgm_player = dynamic_cast<VGMPlayer*>(codec_data->player);
		codec_data->vgm_header = codec_data->vgm_player->GetFileHeader();
		if(codec_data->player->GetLoopTicks() == 0) // 0 fade when no loop data
		{
			codec_data->fade_time = 0.0;
		}
		load_tags(codec_data);
		return codec_data;
	}

	fail:
	{
		if(codec_data->player_handler)
		{
			delete codec_data->player_handler;
		}
		if(codec_data->dLoad)
		{
			DataLoader_Deinit(codec_data->dLoad);
		}
	}
	return NULL;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->player_handler->UnloadFile();
	delete codec_data->player_handler;
	DataLoader_Deinit(codec_data->dLoad);
	free(codec_data);
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(!strcmp(name, "Album"))
	{
		if(strlen(codec_data->tag_album))
		{
			return codec_data->tag_album;
		}
	}
	else if(!strcmp(name, "Artist"))
	{
		if(strlen(codec_data->tag_artist))
		{
			return codec_data->tag_artist;
		}
	}
	else if(!strcmp(name, "Album"))
	{
		if(strlen(codec_data->tag_album))
		{
			return codec_data->tag_album;
		}
	}
	else if(!strcmp(name, "Title"))
	{
		if(strlen(codec_data->tag_title))
		{
			return codec_data->tag_title;
		}
	}
	else if(!strcmp(name, "Copyright"))
	{
		if(strlen(codec_data->tag_date))
		{
			return codec_data->tag_date;
		}
	}
	else if(!strcmp(name, "Comment"))
	{
		if(strlen(codec_data->tag_comment))
		{
			return codec_data->tag_comment;
		}
	}
	else if(!strcmp(name, "Loop Start"))
	{
		if(codec_data->player->GetLoopTicks())
		{
			sprintf(codec_data->tag_loop_start, "%.3f", codec_data->player->Tick2Second(codec_data->player->GetTotalTicks() - (codec_data->player->GetLoopTicks() * codec_data->loop_count)) - codec_data->fade_time);
			return codec_data->tag_loop_start;
		}
		return NULL;
	}
	else if(!strcmp(name, "Loop End"))
	{
		if(codec_data->player->GetLoopTicks())
		{
			sprintf(codec_data->tag_loop_end, "%.3f", codec_data->player->Tick2Second(codec_data->player->GetTotalTicks() - (codec_data->player->GetLoopTicks() * codec_data->loop_count)) - codec_data->fade_time + codec_data->player->Tick2Second(codec_data->player->GetLoopTicks()));
			return codec_data->tag_loop_end;
		}
		return NULL;
	}
	else if(!strcmp(name, "Fade Time"))
	{
		if(codec_data->player->GetLoopTicks())
		{
			sprintf(codec_data->tag_fade_time, "8.0");
			return codec_data->tag_fade_time;
		}
		return NULL;
	}

	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static void update_player_settings(CODEC_DATA * codec_data)
{
	if(codec_data->codec_mutex)
	{
		al_lock_mutex(codec_data->codec_mutex);
	}
	codec_data->config = codec_data->player_handler->GetConfiguration();
	codec_data->config.masterVol = 0x10000;
	codec_data->config.loopCount = codec_data->loop_count;
	codec_data->config.fadeSmpls = 44100 * codec_data->fade_time;
	codec_data->config.endSilenceSmpls = 44100 / 2;	// 0.5 seconds of silence at the end
	codec_data->config.pbSpeed = 1.0;
	codec_data->player_handler->SetConfiguration(codec_data->config);
	if(codec_data->codec_mutex)
	{
		al_unlock_mutex(codec_data->codec_mutex);
	}
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->loop_count = loop_count;
	codec_data->fade_time = fade_time;
	update_player_settings(codec_data);

	return false;
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->volume = volume;
	if(codec_data->codec_mutex)
	{
		al_lock_mutex(codec_data->codec_mutex);
		if(codec_data->codec_stream)
		{
			al_set_audio_stream_gain(codec_data->codec_stream, volume);
		}
		al_unlock_mutex(codec_data->codec_mutex);
	}

	return true;
}

static void * _libvgm_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	char * fragment;
	uint16_t * fragment_16;
	bool done = false;
	PLR_DEV_OPTS devOpts;
	UINT32 devOptID;
	int ret;

	queue = al_create_event_queue();
	if(!queue)
	{
		return NULL;
	}
	al_register_event_source(queue, al_get_audio_stream_event_source(codec_data->codec_stream));

	devOptID = PLR_DEV_ID(DEVID_SN76496, 0);
	ret = codec_data->player->GetDeviceOptions(devOptID, devOpts);
	if (! (ret & 0x80))
	{
		static const INT16 panPos[4] = {0x00, -0x80, +0x80, 0x00};
		if (! devOpts.emuCore[0])
			devOpts.emuCore[0] = FCC_MAXM;
		memcpy(devOpts.panOpts.chnPan, panPos, sizeof(panPos));
		codec_data->player->SetDeviceOptions(devOptID, devOpts);
	}
	
	devOptID = PLR_DEV_ID(DEVID_YM2413, 0);
	ret = codec_data->player->GetDeviceOptions(devOptID, devOpts);
	if (! (ret & 0x80))
	{
		static const INT16 panPos[14] = {
			-0x100, +0x100, -0x80, +0x80, -0x40, +0x40, -0xC0, +0xC0, 0x00,
			-0x60, +0x60, 0x00, -0xC0, +0xC0};
		memcpy(devOpts.panOpts.chnPan, panPos, sizeof(panPos));
		codec_data->player->SetDeviceOptions(devOptID, devOpts);
	}
	
	devOptID = PLR_DEV_ID(DEVID_AY8910, 0);
	ret = codec_data->player->GetDeviceOptions(devOptID, devOpts);
	if (! (ret & 0x80))
	{
		static const INT16 panPos[3] = {-0x80, +0x80, 0x00};
		memcpy(devOpts.panOpts.chnPan, panPos, sizeof(panPos));
		codec_data->player->SetDeviceOptions(devOptID, devOpts);
	}

	devOptID = PLR_DEV_ID(DEVID_C6280, 0);
	ret = codec_data->player->GetDeviceOptions(devOptID, devOpts);
	if (! (ret & 0x80))
	{
		if (! devOpts.emuCore[0])
			devOpts.emuCore[0] = FCC_MAME;
		codec_data->player->SetDeviceOptions(devOptID, devOpts);
	}
	codec_data->player_handler->Start();

	while(!done)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (char *)al_get_audio_stream_fragment(codec_data->codec_stream);
			if(fragment)
			{
				fragment_16 = (uint16_t *)fragment;
				if(codec_data->paused)
				{
					memset(fragment, 0, sizeof(short) * _libvgm_buf_size * 2);
				}
				else
				{
					al_lock_mutex(codec_data->codec_mutex);
					int renderedBytes = codec_data->player_handler->Render(_libvgm_buf_size * 4, fragment);
					codec_data->sample_count += renderedBytes / 2;
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
	
	codec_data->config = codec_data->player_handler->GetConfiguration();
	codec_data->config.masterVol = 0x10000;
	codec_data->config.loopCount = codec_data->loop_count;
	codec_data->config.fadeSmpls = 44100 * codec_data->fade_time;
	codec_data->config.endSilenceSmpls = 44100 / 2;	// 0.5 seconds of silence at the end
	codec_data->config.pbSpeed = 1.0;
	codec_data->player_handler->SetConfiguration(codec_data->config);
	codec_data->player_handler->SetOutputSettings(44100, 2, 16, _libvgm_buf_size);

	codec_data->codec_stream = al_create_audio_stream(4, _libvgm_buf_size, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	if(!codec_data->codec_stream)
	{
		goto fail;
	}
	al_set_audio_stream_gain(codec_data->codec_stream, codec_data->volume);

	al_attach_audio_stream_to_mixer(codec_data->codec_stream, al_get_default_mixer());
	codec_data->codec_thread = al_create_thread(_libvgm_update_thread, codec_data);
	if(!codec_data->codec_thread)
	{
		goto fail;
	}
	codec_data->codec_mutex = al_create_mutex();
	if(!codec_data->codec_mutex)
	{
		goto fail;
	}
	al_start_thread(codec_data->codec_thread);
	return true;

	fail:
	{
		if(codec_data->codec_mutex)
		{
			al_destroy_mutex(codec_data->codec_mutex);
			codec_data->codec_mutex = NULL;
		}
		if(codec_data->codec_thread)
		{
			al_destroy_thread(codec_data->codec_thread);
			codec_data->codec_thread = NULL;
		}
		if(codec_data->codec_stream)
		{
			al_destroy_audio_stream(codec_data->codec_stream);
			codec_data->codec_stream = NULL;
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
	codec_data->player_handler->Stop();
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	unsigned long seek_sample = pos * 44100.0;

	al_lock_mutex(codec_data->codec_mutex);
	codec_data->player->Seek(PLAYPOS_SAMPLE, seek_sample);
	codec_data->sample_count = seek_sample * 2;
	al_unlock_mutex(codec_data->codec_mutex);
	
	return true;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return (float)codec_data->sample_count / 2.0 / 44100.0;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->length < 0.0)
	{
		if(codec_data->codec_mutex)
		{
			al_lock_mutex(codec_data->codec_mutex);
		}
		codec_data->length = codec_data->player->Tick2Second(codec_data->player->GetTotalTicks());
		codec_data->length += codec_data->fade_time;
		if(codec_data->codec_mutex)
		{
			al_unlock_mutex(codec_data->codec_mutex);
		}
	}

	return codec_data->length;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	UINT8 state = codec_data->player->GetState();

	if(codec_get_position(data) >= codec_get_length(data))
	{
		return true;
	}

	return state & (PLAYSTATE_FIN | PLAYSTATE_END);
}

static const char * codec_get_info(void * data)
{
	return "libvgm";
}

static OMO_CODEC_HANDLER codec_handler;

extern "C" OMO_CODEC_HANDLER * omo_codec_libvgm_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	strcpy(codec_handler.id, "libvgm");
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
	omo_codec_handler_add_type(&codec_handler, ".vgm");
	omo_codec_handler_add_type(&codec_handler, ".vgz");
	omo_codec_handler_add_type(&codec_handler, ".vg");
	return &codec_handler;
}
