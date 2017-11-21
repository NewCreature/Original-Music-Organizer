#include "t3f/t3f.h"

#include <vorbis/vorbisfile.h>
#include <FLAC/metadata.h>

#include "../codec_handler.h"

typedef struct
{

	ALLEGRO_AUDIO_STREAM * player_stream;
	char player_filename[1024];
	char codec_file_extension[16];
	char codec_tag_buffer[1024];
	char tag_name[32];
	bool loop;         // let us know we are looping this song
	double loop_start;
	double loop_end;
	double fade_time;  // how long to fade out
	int loop_count;    // how many times we want to loop
	int current_loop;  // how many times have we looped
	double last_pos;   // compare last_pos to current stream pos to detect loop
	bool fade_out;     // let us know we are fading out
	double fade_start; // time the fade began
	float volume;
	char info[256];

} CODEC_DATA;

static void * codec_load_file(const char * fn, const char * subfn)
{
	ALLEGRO_PATH * path;
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->player_stream = al_load_audio_stream(fn, 4, 1024);
		if(data->player_stream)
		{
			strcpy(data->codec_file_extension, "");
			path = al_create_path(fn);
			if(path)
			{
				strcpy(data->codec_file_extension, al_get_path_extension(path));
				al_destroy_path(path);
			}
			strcpy(data->player_filename, fn);
		}
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_destroy_audio_stream(codec_data->player_stream);
	codec_data->player_stream = NULL;
	free(data);
}

static const char * get_tag_name(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(!strcmp(name, "Disc"))
	{
		strcpy(codec_data->tag_name, "DISCNUMBER");
	}
	else if(!strcmp(name, "Track"))
	{
		strcpy(codec_data->tag_name, "TRACKNUMBER");
	}
	else if(!strcmp(name, "Year"))
	{
		strcpy(codec_data->tag_name, "DATE");
	}
	else if(!strcmp(name, "Comment"))
	{
		strcpy(codec_data->tag_name, "DESCRIPTION");
	}
	else
	{
		strcpy(codec_data->tag_name, name);
	}
	return codec_data->tag_name;
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(!strcasecmp(codec_data->codec_file_extension, ".ogg"))
	{
		OggVorbis_File vf;
		vorbis_comment * vf_comment;
		char * vf_comment_text = NULL;

		if(!ov_fopen(codec_data->player_filename, &vf))
		{
			vf_comment = ov_comment(&vf, -1);
			if(vf_comment)
			{
				vf_comment_text = vorbis_comment_query(vf_comment, get_tag_name(data, name), 0);
				if(vf_comment_text)
				{
					strcpy(codec_data->codec_tag_buffer, vf_comment_text);
				}
			}
			ov_clear(&vf);
			if(vf_comment_text)
			{
				return codec_data->codec_tag_buffer;
			}
		}
	}
	else if(!strcasecmp(codec_data->codec_file_extension, ".flac"))
	{
		FLAC__StreamMetadata * flac_metadata;
		int entry;
		int i;

		if(FLAC__metadata_get_tags(codec_data->player_filename, &flac_metadata))
		{
			entry = FLAC__metadata_object_vorbiscomment_find_entry_from(flac_metadata, 0, get_tag_name(data, name));
			if(entry >= 0)
			{
				for(i = 0; i < strlen((char *)(flac_metadata->data.vorbis_comment.comments[entry].entry)); i++)
				{
					if(flac_metadata->data.vorbis_comment.comments[entry].entry[i] == '=')
					{
						i++;
						strcpy(codec_data->codec_tag_buffer, (char *)&flac_metadata->data.vorbis_comment.comments[entry].entry[i]);
					}
				}
			}
			FLAC__metadata_object_delete(flac_metadata);
			if(entry >= 0)
			{
				return codec_data->codec_tag_buffer;
			}
		}
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_set_loop(void * data, double loop_start, double loop_end, double fade_time, int loop_count)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(al_set_audio_stream_loop_secs(codec_data->player_stream, loop_start, loop_end))
	{
		if(al_set_audio_stream_playmode(codec_data->player_stream, ALLEGRO_PLAYMODE_LOOP))
		{
			codec_data->loop = true;
			codec_data->loop_start = loop_start;
			codec_data->loop_end = loop_end;
			codec_data->fade_time = fade_time;
			codec_data->loop_count = loop_count;
			return true;
		}
	}

	return false;
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->volume = volume;
	if(codec_data->player_stream)
	{
		al_set_audio_stream_gain(codec_data->player_stream, codec_data->volume);
	}
	return true;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(al_attach_audio_stream_to_mixer(codec_data->player_stream, al_get_default_mixer()))
	{
		return true;
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(al_set_audio_stream_playing(codec_data->player_stream, false))
	{
		return true;
	}
	return false;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(al_set_audio_stream_playing(codec_data->player_stream, true))
	{
		return true;
	}
	return false;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_drain_audio_stream(codec_data->player_stream);
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	int loop_count = 0;

	if(codec_data->loop)
	{
		while(pos > al_get_audio_stream_length_secs(codec_data->player_stream))
		{
			pos -= codec_data->loop_end - codec_data->loop_start;
			loop_count++;
		}
		codec_data->loop_count = loop_count;
	}

	return al_seek_audio_stream_secs(codec_data->player_stream, pos);
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return al_get_audio_stream_position_secs(codec_data->player_stream);
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->loop)
	{
		return codec_data->loop_start + (codec_data->loop_end - codec_data->loop_start) * (double)codec_data->loop_count + codec_data->fade_time;
	}
	return al_get_audio_stream_length_secs(codec_data->player_stream);
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	float volume;

	if(codec_data->player_stream)
	{
		if(codec_data->loop)
		{
			if(al_get_audio_stream_position_secs(codec_data->player_stream) < codec_data->last_pos)
			{
				codec_data->current_loop++;
			}
			codec_data->last_pos = al_get_audio_stream_position_secs(codec_data->player_stream);
			if(codec_data->current_loop >= codec_data->loop_count && !codec_data->fade_out)
			{
				codec_data->fade_start = al_get_time();
				codec_data->fade_out = true;
			}
			if(codec_data->fade_out)
			{
				volume = 1.0 - (al_get_time() - codec_data->fade_start) / codec_data->fade_time;
				al_set_audio_stream_gain(codec_data->player_stream, volume);
				if(al_get_time() - codec_data->fade_start >= codec_data->fade_time)
				{
					return true;
				}
			}
		}
		return !al_get_audio_stream_playing(codec_data->player_stream);
	}
	return false;
}

static const char * codec_get_info(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	ALLEGRO_AUDIO_DEPTH depth;
	ALLEGRO_CHANNEL_CONF channel_conf;
	int freq;
	int bits;
	int channels;

	freq = al_get_audio_stream_frequency(codec_data->player_stream);
	depth = al_get_audio_stream_depth(codec_data->player_stream);
	if(depth & ALLEGRO_AUDIO_DEPTH_INT8)
	{
		bits = 8;
	}
	else if(depth & ALLEGRO_AUDIO_DEPTH_INT16)
	{
		bits = 16;
	}
	else if(depth & ALLEGRO_AUDIO_DEPTH_INT24)
	{
		bits = 24;
	}
	else
	{
		bits = 32;
	}
	channel_conf = al_get_audio_stream_channels(codec_data->player_stream);
	switch(channel_conf)
	{
		case ALLEGRO_CHANNEL_CONF_1:
		{
			channels = 1;
			break;
		}
		case ALLEGRO_CHANNEL_CONF_2:
		{
			channels = 2;
			break;
		}
		default:
		{
			channels = 3;
			break;
		}
	}
	sprintf(codec_data->info, "%dhz %d-Bit %s (Allegro acodec)", freq, bits, channels == 1 ? "Mono" : (channels == 2 ? "Stereo" : "Surround"));
	return codec_data->info;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_allegro_acodec_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
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
	omo_codec_handler_add_type(&codec_handler, ".ogg");
	omo_codec_handler_add_type(&codec_handler, ".flac");
	omo_codec_handler_add_type(&codec_handler, ".wav");
	return &codec_handler;
}
