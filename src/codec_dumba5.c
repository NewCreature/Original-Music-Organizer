#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "player.h"

bool omo_codec_mod_initialize(void)
{
	if(dumba5_init(DUMB_RQ_CUBIC))
	{
		return true;
	}
	return false;
}

bool omo_codec_mod_play(const char * fn)
{
	if(dumba5_load_and_play_module(fn, 0, false, 44100, true))
	{
		return true;
	}
	return false;
}

bool omo_codec_mod_pause(bool paused)
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

void omo_codec_mod_stop(void)
{
	dumba5_stop_module();
}

float omo_codec_mod_get_position(void)
{
	return (float)dumba5_get_module_position() / 65536.0;
}

static OMO_PLAYER omo_mod_player;

OMO_PLAYER * omo_codec_dumba5_get_player(void)
{
	omo_mod_player.initialize = omo_codec_mod_initialize;
	omo_mod_player.play_file = omo_codec_mod_play;
	omo_mod_player.pause = omo_codec_mod_pause;
	omo_mod_player.stop = omo_codec_mod_stop;
	omo_mod_player.seek = NULL;
	omo_mod_player.get_position = omo_codec_mod_get_position;
	omo_mod_player.get_length = NULL;
	omo_mod_player.types = 0;
	omo_player_add_type(&omo_mod_player, ".mod");
	omo_player_add_type(&omo_mod_player, ".s3m");
	omo_player_add_type(&omo_mod_player, ".xm");
	omo_player_add_type(&omo_mod_player, ".it");
	if(omo_mod_player.initialize())
	{
		return &omo_mod_player;
	}
	return NULL;
}
