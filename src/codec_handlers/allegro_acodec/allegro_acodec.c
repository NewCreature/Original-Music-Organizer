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
				printf("%s\n", codec_data->codec_tag_buffer);
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

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return al_get_audio_stream_position_secs(codec_data->player_stream);
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->player_stream)
	{
		return !al_get_audio_stream_playing(codec_data->player_stream);
	}
	return false;
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
	codec_handler.play = codec_play;
	codec_handler.pause = codec_pause;
	codec_handler.resume = codec_resume;
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
