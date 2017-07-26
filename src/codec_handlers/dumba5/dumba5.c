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

static OMO_CODEC_HANDLER codec_player;

OMO_CODEC_HANDLER * omo_codec_dumba5_get_player(void)
{
	memset(&codec_player, 0, sizeof(OMO_CODEC_HANDLER));
	codec_player.initialize = codec_initialize;
	codec_player.load_file = codec_load_file;
	codec_player.get_track_count = codec_get_track_count;
	codec_player.play = codec_play;
	codec_player.pause = codec_pause;
	codec_player.stop = codec_stop;
	codec_player.seek = NULL;
	codec_player.get_position = codec_get_position;
	codec_player.get_length = NULL;
	codec_player.done_playing = codec_done_playing;
	codec_player.types = 0;
	omo_player_add_type(&codec_player, ".mod");
	omo_player_add_type(&codec_player, ".s3m");
	omo_player_add_type(&codec_player, ".xm");
	omo_player_add_type(&codec_player, ".it");
	#if DUMB_MAJOR_VERSION > 0
		omo_player_add_type(&codec_player, ".stm");
		omo_player_add_type(&codec_player, ".669");
		omo_player_add_type(&codec_player, ".ptm");
		omo_player_add_type(&codec_player, ".psm");
		omo_player_add_type(&codec_player, ".mtm");
		omo_player_add_type(&codec_player, ".dsm");
		omo_player_add_type(&codec_player, ".amf");
		omo_player_add_type(&codec_player, ".umx");
		omo_player_add_type(&codec_player, ".j2b");
	#endif
	if(codec_player.initialize())
	{
		return &codec_player;
	}
	return NULL;
}
