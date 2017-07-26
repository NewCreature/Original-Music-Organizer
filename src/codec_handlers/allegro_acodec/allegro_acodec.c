#include "t3f/t3f.h"
#include "t3f/music.h"

#include "../codec_handler.h"

static char player_filename[1024] = {0};
static OMO_CODEC_HANDLER codec_handler;

static bool codec_load_file(const char * fn, const char * subfn)
{
	strcpy(player_filename, fn);
	return true;
}

static int codec_get_track_count(const char * fn)
{
	return 1;
}

static bool codec_play(void)
{
	t3f_disable_music_looping();
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

static bool codec_done_playing(void)
{
	if(t3f_stream)
	{
		return !al_get_audio_stream_playing(t3f_stream);
	}
	return false;
}

OMO_CODEC_HANDLER * omo_codec_allegro_acodec_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = NULL;
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
	omo_codec_handler_add_type(&codec_handler, ".ogg");
	omo_codec_handler_add_type(&codec_handler, ".flac");
	omo_codec_handler_add_type(&codec_handler, ".wav");
	return &codec_handler;
}
