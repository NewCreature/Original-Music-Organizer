#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "../codec_handler.h"

static char player_filename[1024] = {0};
static char player_sub_filename[1024] = {0};

static bool codec_initialize(void)
{
	if(dumba5_init(DUMB_RQ_CUBIC))
	{
		return true;
	}
	return false;
}

static bool codec_load_file(const char * fn, const char * subfn)
{
	strcpy(player_filename, fn);
	strcpy(player_sub_filename, "");
	if(subfn)
	{
		strcpy(player_sub_filename, subfn);
	}
	return true;
}

static int codec_get_track_count(const char * fn)
{
	return 1;
}

static bool codec_play(void)
{
	int start = 0;

	if(strlen(player_sub_filename) > 0)
	{
		start = atoi(player_sub_filename);
	}
	if(dumba5_load_and_play_module(player_filename, start, false, 44100, true))
	{
		return true;
	}
	return false;
}

static bool codec_pause(bool paused)
{
	if(paused)
	{
		dumba5_pause_module();
	}
	else
	{
		dumba5_resume_module();
	}
	return true;
}

static void codec_stop(void)
{
	dumba5_stop_module();
}

static float codec_get_position(void)
{
	return (float)dumba5_get_module_position() / 65536.0;
}

static bool codec_done_playing(void)
{
	return dumba5_module_playback_finished();
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_dumba5_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = codec_initialize;
	codec_handler.load_file = codec_load_file;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.stop = codec_stop;
	codec_handler.seek = NULL;
	codec_handler.get_position = codec_get_position;
	codec_handler.get_length = NULL;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".mod");
	omo_codec_handler_add_type(&codec_handler, ".s3m");
	omo_codec_handler_add_type(&codec_handler, ".xm");
	omo_codec_handler_add_type(&codec_handler, ".it");
	#if DUMB_MAJOR_VERSION > 0
		omo_codec_handler_add_type(&codec_handler, ".stm");
		omo_codec_handler_add_type(&codec_handler, ".669");
		omo_codec_handler_add_type(&codec_handler, ".ptm");
		omo_codec_handler_add_type(&codec_handler, ".psm");
		omo_codec_handler_add_type(&codec_handler, ".mtm");
		omo_codec_handler_add_type(&codec_handler, ".dsm");
		omo_codec_handler_add_type(&codec_handler, ".amf");
		omo_codec_handler_add_type(&codec_handler, ".umx");
		omo_codec_handler_add_type(&codec_handler, ".j2b");
	#endif
	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}
