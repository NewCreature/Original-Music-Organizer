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
	unsigned long sample_count;
} CODEC_DATA;

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
		codec_data->player_handler->RegisterPlayerEngine(new VGMPlayer);
		codec_data->config = codec_data->player_handler->GetConfiguration();
		codec_data->config.masterVol = 0x10000;
		codec_data->config.loopCount = 1;
		codec_data->config.fadeSmpls = 44100 * 8;	// fade over 4 seconds
		codec_data->config.endSilenceSmpls = 44100 / 2;	// 0.5 seconds of silence at the end
		codec_data->config.pbSpeed = 1.0;
		codec_data->player_handler->SetConfiguration(codec_data->config);
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
		printf("VGM v%3X, Total Length: %.2f s, Loop Length: %.2f s", codec_data->vgm_header->fileVer,
				codec_data->player->Tick2Second(codec_data->player->GetTotalTicks()), codec_data->player->Tick2Second(codec_data->player->GetLoopTicks()));
		codec_data->player_handler->SetLoopCount(codec_data->vgm_player->GetModifiedLoopCount(10));
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
	DataLoader_Deinit(codec_data->dLoad);
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	const char* songTitle = NULL;
	const char* songAuthor = NULL;
	const char* songGame = NULL;
	const char* songSystem = NULL;
	const char* songDate = NULL;
	const char* songComment = NULL;

	const char* const* tagList = codec_data->player->GetTags();
	for (const char* const* t = tagList; *t; t += 2)
	{
		if (!strcmp(t[0], "TITLE"))
			songTitle = t[1];
		else if (!strcmp(t[0], "ARTIST"))
			songAuthor = t[1];
		else if (!strcmp(t[0], "GAME"))
			songGame = t[1];
		else if (!strcmp(t[0], "SYSTEM"))
			songSystem = t[1];
		else if (!strcmp(t[0], "DATE"))
			songDate = t[1];
		else if (!strcmp(t[0], "COMMENT"))
			songComment = t[1];
	}
	
	if (songTitle != NULL && songTitle[0] != '\0')
		printf("\nSong Title: %s", songTitle);
//	if (showTags >= 2)
	{
		if (songAuthor != NULL && songAuthor[0] != '\0')
			printf("\nSong Author: %s", songAuthor);
		if (songGame != NULL && songGame[0] != '\0')
			printf("\nSong Game: %s", songGame);
		if (songSystem != NULL && songSystem[0] != '\0')
			printf("\nSong System: %s", songSystem);
		if (songDate != NULL && songDate[0] != '\0')
			printf("\nSong Date: %s", songDate);
		if (songComment != NULL && songComment[0] != '\0')
			printf("\nSong Comment: %s", songComment);
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 0;
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	return false;
}

static bool codec_set_volume(void * data, float volume)
{
	return true;
}

static void * _libvgm_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	char * fragment;
	bool done = false;
	PLR_DEV_OPTS devOpts;
	UINT32 devOptID;
	int ret;

	printf("libvgm thread\n");
	queue = al_create_event_queue();
	if(!queue)
	{
		printf("break 1\n");
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
				if(codec_data->paused)
				{
					memset(fragment, 0, sizeof(short) * 4096 * 2);
				}
				else
				{
					if (! (codec_data->player_handler->GetState() & PLAYSTATE_PLAY))
					{
						printf("not playing!\n");
					}
					printf("fill buffer %lu\n", codec_data->sample_count);
					al_lock_mutex(codec_data->codec_mutex);
					int renderedBytes = codec_data->player_handler->Render(8192, fragment);
					codec_data->sample_count += renderedBytes;
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
	
	printf("play 1\n");
	codec_data->player_handler->SetOutputSettings(44100, 2, 16, 4096);

	codec_data->codec_stream = al_create_audio_stream(4, 4096, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1);
	if(!codec_data->codec_stream)
	{
	printf("play 2\n");
		goto fail;
	}

	al_attach_audio_stream_to_mixer(codec_data->codec_stream, al_get_default_mixer());
	codec_data->codec_thread = al_create_thread(_libvgm_update_thread, codec_data);
	if(!codec_data->codec_thread)
	{
	printf("play 3\n");
		goto fail;
	}
	codec_data->codec_mutex = al_create_mutex();
	if(!codec_data->codec_mutex)
	{
	printf("play 4\n");
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
	printf("play 5\n");

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
	return true;
}

static double codec_get_position(void * data)
{
	return 0;
}

static double codec_get_length(void * data)
{
	return 0;
}

static bool codec_done_playing(void * data)
{
	return false;
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
