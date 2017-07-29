#include "t3f/t3f.h"

#include "../codec_handler.h"
#include "MP3A5/mp3a5.h"

static OMO_CODEC_HANDLER codec_handler;
static MP3A5_MP3 * codec_mp3 = NULL;;

static bool codec_initialize(void)
{
	mpg123_init();
	return true;
}

static void codec_exit(void)
{
	mpg123_exit();
}

static bool codec_load_file(const char * fn, const char * subfn)
{
	codec_mp3 = mp3a5_load_mp3(fn);
	if(!codec_mp3)
	{
		return false;
	}
	return true;
}

static int codec_get_track_count(const char * fn)
{
	return 1;
}

static bool codec_play(void)
{
	if(mp3a5_play_mp3(codec_mp3, 4, 1024))
	{
		return true;
	}
	return false;
}

static bool codec_pause(void)
{
	mp3a5_pause_mp3(codec_mp3);
	return true;
}

static bool codec_resume(void)
{
	mp3a5_resume_mp3(codec_mp3);
	return true;
}

static void codec_stop(void)
{
	mp3a5_stop_mp3(codec_mp3);
	mp3a5_destroy_mp3(codec_mp3);
	codec_mp3 = NULL;
}

/*static float codec_get_position(void)
{
	return al_get_audio_stream_position_secs(t3f_stream);
} */

static bool codec_done_playing(void)
{
	if(!codec_mp3 || codec_mp3->done)
	{
		return true;
	}
	return false;
}

OMO_CODEC_HANDLER * omo_codec_mp3a5_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = codec_initialize;
	codec_handler.exit = codec_exit;
	codec_handler.load_file = codec_load_file;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = NULL;
	codec_handler.get_position = NULL;
	codec_handler.get_length = NULL;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".mpg");
	omo_codec_handler_add_type(&codec_handler, ".mp2");
	omo_codec_handler_add_type(&codec_handler, ".mp3");
	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}