#include "t3f/t3f.h"

#include <vorbis/vorbisfile.h>
#include <FLAC/metadata.h>

#include "../codec_handler.h"

static ALLEGRO_AUDIO_STREAM * player_stream = NULL;
static char player_filename[1024] = {0};
static OMO_CODEC_HANDLER codec_handler;
static char codec_file_extension[16] = {0};
static char codec_tag_buffer[1024] = {0};

static bool codec_load_file(const char * fn, const char * subfn)
{
	ALLEGRO_PATH * path;

	player_stream = al_load_audio_stream(fn, 4, 1024);
	if(player_stream)
	{
		strcpy(codec_file_extension, "");
		path = al_create_path(fn);
		if(path)
		{
			strcpy(codec_file_extension, al_get_path_extension(path));
			al_destroy_path(path);
		}
		strcpy(player_filename, fn);
		return true;
	}
	return true;
}

static char tag_name[32];

static const char * get_tag_name(const char * name)
{
	if(!strcmp(name, "Disc"))
	{
		strcpy(tag_name, "DISCNUMBER");
	}
	else if(!strcmp(name, "Track"))
	{
		strcpy(tag_name, "TRACKNUMBER");
	}
	else if(!strcmp(name, "Year"))
	{
		strcpy(tag_name, "DATE");
	}
	else if(!strcmp(name, "Comment"))
	{
		strcpy(tag_name, "DESCRIPTION");
	}
	else
	{
		strcpy(tag_name, name);
	}
	return tag_name;
}

static const char * codec_get_tag(const char * name)
{
	if(!strcasecmp(codec_file_extension, ".ogg"))
	{
		OggVorbis_File vf;
		vorbis_comment * vf_comment;
		char * vf_comment_text = NULL;

		if(!ov_fopen(player_filename, &vf))
		{
			vf_comment = ov_comment(&vf, -1);
			if(vf_comment)
			{
				vf_comment_text = vorbis_comment_query(vf_comment, get_tag_name(name), 0);
				if(vf_comment_text)
				{
					strcpy(codec_tag_buffer, vf_comment_text);
				}
			}
			ov_clear(&vf);
			if(vf_comment_text)
			{
				return codec_tag_buffer;
			}
		}
	}
	else if(!strcasecmp(codec_file_extension, ".flac"))
	{
		FLAC__StreamMetadata * flac_metadata;
		int entry;
		int i;

		if(FLAC__metadata_get_tags(player_filename, &flac_metadata))
		{
			entry = FLAC__metadata_object_vorbiscomment_find_entry_from(flac_metadata, 0, get_tag_name(name));
			if(entry >= 0)
			{
				for(i = 0; i < strlen((char *)(flac_metadata->data.vorbis_comment.comments[entry].entry)); i++)
				{
					if(flac_metadata->data.vorbis_comment.comments[entry].entry[i] == '=')
					{
						i++;
						strcpy(codec_tag_buffer, (char *)&flac_metadata->data.vorbis_comment.comments[entry].entry[i]);
					}
				}
				printf("%s\n", codec_tag_buffer);
			}
			FLAC__metadata_object_delete(flac_metadata);
			if(entry >= 0)
			{
				return codec_tag_buffer;
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
	if(al_attach_audio_stream_to_mixer(player_stream, al_get_default_mixer()))
	{
		return true;
	}
	return false;
}

static bool codec_pause(void)
{
	if(al_set_audio_stream_playing(player_stream, false))
	{
		return true;
	}
	return false;
}

static bool codec_resume(void)
{
	if(al_set_audio_stream_playing(player_stream, true))
	{
		return true;
	}
	return false;
}

static void codec_stop(void)
{
	al_drain_audio_stream(player_stream);
	al_destroy_audio_stream(player_stream);
	player_stream = false;
}

static double codec_get_position(void)
{
	return al_get_audio_stream_position_secs(player_stream);
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
