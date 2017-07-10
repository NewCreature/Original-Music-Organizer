#include "t3f/t3f.h"

#include "player.h"

bool omo_codec_ogg_play(const char * fn)
{
	if(t3f_play_music(fn))
	{
		return true;
	}
	return false;
}

bool omo_codec_ogg_pause(bool paused)
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

void omo_codec_ogg_stop(void)
{
	t3f_stop_music();
}

float omo_codec_ogg_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
}

static OMO_PLAYER omo_ogg_player;

OMO_PLAYER * omo_codec_allegro_acodec_get_player(void)
{
	omo_ogg_player.initialize = NULL;
	omo_ogg_player.play_file = omo_codec_ogg_play;
	omo_ogg_player.pause = omo_codec_ogg_pause;
	omo_ogg_player.stop = omo_codec_ogg_stop;
	omo_ogg_player.seek = NULL;
	omo_ogg_player.get_position = omo_codec_ogg_get_position;
	omo_ogg_player.get_length = NULL;
	omo_ogg_player.types = 0;
	omo_player_add_type(&omo_ogg_player, ".ogg");
	omo_player_add_type(&omo_ogg_player, ".flac");
	omo_player_add_type(&omo_ogg_player, ".wav");
	return &omo_ogg_player;
}
