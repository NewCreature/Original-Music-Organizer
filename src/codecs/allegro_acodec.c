#include "t3f/t3f.h"

#include "player.h"

static char player_filename[1024] = {0};
static OMO_PLAYER codec_player;

static bool codec_load_file(const char * fn)
{
	strcpy(player_filename, fn);
	return true;
}

static bool codec_play(void)
{
	if(t3f_play_music(player_filename))
	{
		return true;
	}
	return false;
}

static bool codec_pause(bool paused)
{
	if(paused)
	{
		t3f_pause_music();
	}
	else
	{
		t3f_resume_music();
	}
	return true;
}

static void codec_stop(void)
{
	t3f_stop_music();
}

static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
}

OMO_PLAYER * omo_codec_allegro_acodec_get_player(void)
{
	codec_player.initialize = NULL;
	codec_player.load_file = codec_load_file;
	codec_player.play = codec_play;
	codec_player.pause = codec_pause;
	codec_player.stop = codec_stop;
	codec_player.seek = NULL;
	codec_player.get_position = codec_get_position;
	codec_player.get_length = NULL;
	codec_player.types = 0;
	omo_player_add_type(&codec_player, ".ogg");
	omo_player_add_type(&codec_player, ".flac");
	omo_player_add_type(&codec_player, ".wav");
	return &codec_player;
}
