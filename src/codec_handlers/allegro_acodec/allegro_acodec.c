#include "t3f/t3f.h"
#include "t3f/music.h"

#include <vorbis/vorbisfile.h>

#include "../codec_handler.h"

static char player_filename[1024] = {0};
static OMO_CODEC_HANDLER codec_handler;
static char codec_file_extension[16] = {0};
static char codec_tag_buffer[1024] = {0};

static bool codec_load_file(const char * fn, const char * subfn)
{
	ALLEGRO_PATH * path;

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

static const char * codec_get_tag(const char * name)
{
	if(!strcasecmp(codec_file_extension, ".ogg"))
	{
		OggVorbis_File vf;
		vorbis_comment * vf_comment;
		char * vf_comment_text = NULL;
		char tag_name[32] = {0};

		if(!ov_fopen(player_filename, &vf))
		{
			vf_comment = ov_comment(&vf, -1);
			if(vf_comment)
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
				vf_comment_text = vorbis_comment_query(vf_comment, tag_name, 0);
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
	return NULL;
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

static bool codec_pause(void)
{
	t3f_pause_music();
	return true;
}

static bool codec_resume(void)
{
	t3f_resume_music();
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
