#include "t3f/t3f.h"

#include <vgm/common_def.h>
#include <vgm/utils/DataLoader.h>
#include <vgm/utils/FileLoader.h>
#include <vgm/utils/MemoryLoader.h>
#include <vgm/player/playerbase.hpp>
#include <vgm/player/vgmplayer.hpp>
#include <vgm/player/playera.hpp>
#include <vgm/audio/AudioStream.h>
#include <vgm/audio/AudioStream_SpcDrvFuns.h>
#include <vgm/emu/Resampler.h>
#include <vgm/emu/SoundDevs.h>	// for DEVID_*
#include <vgm/emu/EmuCores.h>
#include <vgm/utils/OSMutex.h>

#include "../codec_handler.h"

typedef struct
{

	int placeholder;

} CODEC_DATA;

static void * codec_load_file(const char * fn, const char * subfn)
{
	return NULL;
}

static void codec_unload_file(void * data)
{
}

static const char * codec_get_tag(void * data, const char * name)
{
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

static bool codec_play(void * data)
{
	return false;
}

static bool codec_pause(void * data)
{
	return false;
}

static bool codec_resume(void * data)
{
	return false;
}

static void codec_stop(void * data)
{
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
	return &codec_handler;
}
