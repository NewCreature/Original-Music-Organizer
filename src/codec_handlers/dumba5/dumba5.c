#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "../codec_handler.h"

typedef struct
{

	DUMBA5_PLAYER * codec_player;
	DUH * codec_module;
	double codec_length;
	char player_sub_filename[1024];
	char tag_buffer[1024];
	bool loop;         // let us know we are looping this song
	int loop_count;
	double loop_start;
	double loop_end;
	double playback_total;
	double fade_time;  // how long to fade out
	bool fade_out;     // let us know we are fading out
	char info[256];
	float volume;

} CODEC_DATA;

static bool codec_initialize(void)
{
	if(dumba5_init(DUMB_RQ_CUBIC))
	{
		return true;
	}
	return false;
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	DUMB_IT_SIGDATA * sd;
	int start = 0;
	long length;
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		strcpy(data->player_sub_filename, "");
		if(subfn)
		{
			strcpy(data->player_sub_filename, subfn);
		}
		data->codec_module = dumba5_load_module(fn);
		if(data->codec_module)
		{
			if(subfn && strlen(subfn) > 0)
			{
				start = atoi(subfn);
			}
			sd = duh_get_it_sigdata(data->codec_module);
			if(sd)
			{
				/* getting the correct length from start orders > 0 only supported in
				   newer versions of DUMB */
				#if DUMB_MAJOR_VERSION > 0
					length = dumb_it_build_checkpoints(sd, start);
				#else
					length = dumb_it_build_checkpoints(sd);
				#endif
				data->codec_length = (double)length / 65536.0;
			}
		}
		data->volume = 1.0;
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	unload_duh(codec_data->codec_module);
	codec_data->codec_module = NULL;
	free(data);
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->loop_start = loop_start;
	codec_data->loop_end = loop_end;
	codec_data->playback_total = loop_start + (loop_end - loop_start) * (double)loop_count;
	codec_data->loop = true;
	codec_data->loop_count = loop_count;
	codec_data->fade_time = fade_time;

	return true;
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->volume = volume;
	if(codec_data->codec_player)
	{
		dumba5_set_player_volume(codec_data->codec_player, codec_data->volume);
	}
	return true;
}

static const char * codec_get_tag(void * data, const char * name)
{
	const char * tag;
	const unsigned char * comment;
	DUMB_IT_SIGDATA * sd;
	int i;
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(!strcmp(name, "Title"))
	{
		tag = duh_get_tag(codec_data->codec_module, "TITLE");
		if(tag && strlen(tag))
		{
			return tag;
		}
	}
	else if(!strcmp(name, "Comment"))
	{
		sd = duh_get_it_sigdata(codec_data->codec_module);
		if(sd)
		{
			comment = dumb_it_sd_get_song_message(sd);
			if(comment && strlen((char *)comment))
			{
				strcpy(codec_data->tag_buffer, "");
				for(i = 0; i < strlen((char *)comment) && i < 1023; i++)
				{
					if(comment[i] < 128)
					{
						codec_data->tag_buffer[i] = comment[i];
					}
					else
					{
						codec_data->tag_buffer[i] = '_';
					}
					if(codec_data->tag_buffer[i] == '\r')
					{
						codec_data->tag_buffer[i] = ' ';
					}
				}
				return codec_data->tag_buffer;
			}
		}
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_play(void * data)
{
	int start = 0;
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(strlen(codec_data->player_sub_filename) > 0)
	{
		start = atoi(codec_data->player_sub_filename);
	}
	codec_data->codec_player = dumba5_create_player(codec_data->codec_module, start, codec_data->loop, 1024, 44100, true);
	dumba5_set_player_volume(codec_data->codec_player, codec_data->volume);
	if(codec_data->codec_player)
	{
		dumba5_start_player(codec_data->codec_player);
		return true;
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	dumba5_pause_player(codec_data->codec_player);
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	dumba5_resume_player(codec_data->codec_player);
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	dumba5_stop_player(codec_data->codec_player);
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return dumba5_set_player_position(codec_data->codec_player, pos);
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->loop)
	{
		return codec_data->loop_start + (codec_data->loop_end - codec_data->loop_start) * codec_data->loop_count + codec_data->fade_time;
	}
	return codec_data->codec_length;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return dumba5_get_player_time(codec_data->codec_player);
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	double volume = 1.0;

	if(codec_data->codec_player)
	{
		if(codec_data->loop)
		{
			if(dumba5_get_player_time(codec_data->codec_player) > codec_data->playback_total)
			{
				codec_data->fade_out = true;
			}
			else
			{
				if(codec_data->fade_out)
				{
					dumba5_set_player_volume(codec_data->codec_player, 1.0);
				}
				codec_data->fade_out = false;
			}
			if(codec_data->fade_out)
			{
				volume = 1.0 - (dumba5_get_player_time(codec_data->codec_player) - codec_data->playback_total) / codec_data->fade_time;
				dumba5_set_player_volume(codec_data->codec_player, volume * codec_data->volume);
				if(dumba5_get_player_time(codec_data->codec_player) >= codec_data->playback_total + codec_data->fade_time)
				{
					return true;
				}
			}
			return false;
		}
	}
	return dumba5_player_playback_finished(codec_data->codec_player);
}

static const char * codec_get_info(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	DUMB_IT_SIGRENDERER * sr;
	DUMB_IT_CHANNEL_STATE channel_state;
	int i;

	sr = duh_get_it_sigrenderer(dumba5_get_player_sigrenderer(codec_data->codec_player));
	if(sr)
	{
		sprintf(codec_data->info, "%d: %d", dumb_it_sr_get_current_order(sr) + 1, dumb_it_sr_get_current_row(sr) + 1);
/*		sprintf(codec_data->info, "%d: %d - ", dumb_it_sr_get_current_order(sr) + 1, dumb_it_sr_get_current_row(sr) + 1);
		for(i = 0; i < 64; i++)
		{
			dumb_it_sr_get_channel_state(sr, i, &channel_state);
			if(channel_state.sample > 0)
			{
				if(channel_state.volume > 0.25)
				{
					strcat(codec_data->info, "O");
				}
				else
				{
					strcat(codec_data->info, "o");
				}
			}
			else
			{
				strcat(codec_data->info, "_");
			}
		} */
	}
	else
	{
		sprintf(codec_data->info, "DUMBA5\n");
	}
	return codec_data->info;
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
	codec_handler.set_loop = codec_set_loop;
	codec_handler.set_volume = codec_set_volume;
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
	codec_handler.stop = codec_stop;
	codec_handler.seek = codec_seek;
	codec_handler.get_length = codec_get_length;
	codec_handler.get_position = codec_get_position;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.get_info = codec_get_info;
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
