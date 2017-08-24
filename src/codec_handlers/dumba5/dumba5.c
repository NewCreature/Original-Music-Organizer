#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "../codec_handler.h"

static DUMBA5_PLAYER * codec_player = NULL;
static DUH * codec_module = NULL;
static double codec_length = 0.0;

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
	DUMB_IT_SIGDATA * sd;
	int start = 0;
	long length;

	strcpy(player_sub_filename, "");
	if(subfn)
	{
		strcpy(player_sub_filename, subfn);
	}
	codec_module = dumba5_load_module(fn);
	if(codec_module)
	{
		if(subfn && strlen(subfn) > 0)
		{
			start = atoi(subfn);
		}
		sd = duh_get_it_sigdata(codec_module);
		if(sd)
		{
			/* getting the correct length from start orders > 0 only supported in
			   newer versions of DUMB */
			#if DUMB_MAJOR_VERSION > 0
				length = dumb_it_build_checkpoints(sd, start);
			#else
				length = dumb_it_build_checkpoints(sd);
			#endif
			codec_length = (double)length / 65536.0;
		}
		return true;
	}
	return false;
}

static void codec_unload_file(void)
{
	unload_duh(codec_module);
	codec_module = NULL;
}

static char tag_buffer[1024] = {0};

static const char * codec_get_tag(const char * name)
{
	const char * tag;
	const unsigned char * comment;
	DUMB_IT_SIGDATA * sd;
	int i;

	if(!strcmp(name, "Title"))
	{
		tag = duh_get_tag(codec_module, "TITLE");
		if(tag && strlen(tag))
		{
			return tag;
		}
	}
	else if(!strcmp(name, "Comment"))
	{
		sd = duh_get_it_sigdata(codec_module);
		if(sd)
		{
			comment = dumb_it_sd_get_song_message(sd);
			if(comment && strlen((char *)comment))
			{
				strcpy(tag_buffer, "");
				for(i = 0; i < strlen((char *)comment) && i < 1023; i++)
				{
					if(comment[i] < 128)
					{
						tag_buffer[i] = comment[i];
					}
					else
					{
						tag_buffer[i] = '_';
					}
					if(tag_buffer[i] == '\r')
					{
						tag_buffer[i] = ' ';
					}
				}
				return tag_buffer;
			}
		}
	}
	return NULL;
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
	codec_player = dumba5_create_player(codec_module, start, false, 4096, 44100, true);
	if(codec_player)
	{
		dumba5_start_player(codec_player);
		return true;
	}
	return false;
}

static bool codec_pause(void)
{
	dumba5_pause_player(codec_player);
	return true;
}

static bool codec_resume(void)
{
	dumba5_resume_player(codec_player);
	return true;
}

static void codec_stop(void)
{
	dumba5_stop_player(codec_player);
}

static double codec_get_length(void)
{
	return codec_length;
}

static double codec_get_position(void)
{
	return (double)dumba5_get_player_position(codec_player) / 65536.0;
}

static bool codec_done_playing(void)
{
	return dumba5_player_playback_finished(codec_player);
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_dumba5_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = codec_initialize;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.get_tag = codec_get_tag;
	codec_handler.get_track_count = codec_get_track_count;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = NULL;
	codec_handler.get_length = codec_get_length;
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
