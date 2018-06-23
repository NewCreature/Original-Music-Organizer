#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "../../defines.h"
#include "../../constants.h"
#include "../codec_handler.h"

typedef struct
{

	AVAudioPlayer * player;
	char player_filename[1024];
	double start_time;
	double pause_start;
	double pause_total;
	double start_pos;
	char tag_buffer[1024];

} CODEC_DATA;

static void * codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] error:nil];
		if(data->player)
		{
			strcpy(data->player_filename, fn);
		}
		else
		{
			free(data);
			data = NULL;
		}
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	[codec_data->player release];
	codec_data->player = NULL;
	free(data);
}

static void codec_strcpy(char * dest, const char * src, int limit)
{
	int i;

	for(i = 0; i < strlen(src) && i < limit - 1; i++)
	{
		if(src[i] == '\r' || src[i] == '\n')
		{
			dest[i] = ' ';
		}
		else
		{
			dest[i] = src[i];
		}
	}
	dest[i] = 0;
	dest[limit - 1] = 0;
}

static const char * codec_get_tag(void * data, const char * name)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	NSString * path = [NSString stringWithUTF8String:codec_data->player_filename];
	const char * utf8_key;
	const char * utf8_val;
	const char * tag_name[OMO_MAX_TAG_TYPES] = {"Album Artist", "Artist", "Album", "Disc", "Track", "Title", "Genre", "Year", "Copyright", "Comment"};
	const char * avplayer_tag_name[OMO_MAX_TAG_TYPES] = {"id3/TPE2", "id3/TPE1", "id3/TALB", "id3/TPOS", "id3/TRCK", "id3/TIT2", "id3/TCON", "id3/TYER", "id3/TCOP", "id3/COMM"};

	int i;

	AVAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:path] options:nil];
	if(asset)
	{
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			if(tag_name[i] && !strcmp(name, tag_name[i]))
			{
				break;
			}
		}
		if(i < OMO_MAX_TAG_TYPES)
		{
			NSArray *metadata = [asset metadata];
			for(AVMetadataItem* item in metadata)
			{
				NSString *key = [item identifier];
				NSString *value = [item stringValue];
				utf8_key = [key UTF8String];
				utf8_val = [value UTF8String];
				if(!strcasecmp(utf8_key, avplayer_tag_name[i]))
				{
					codec_strcpy(codec_data->tag_buffer, utf8_val, 1024);
					[pool release];
					return codec_data->tag_buffer;
				}
			}
		}
	}
	[pool release];
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->player.volume = volume;
	return true;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	[codec_data->player prepareToPlay];
	[codec_data->player play];
	codec_data->start_time = al_get_time();
	codec_data->start_pos = 0.0;
	codec_data->pause_total = 0.0;
	return true;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	[codec_data->player stop];
	codec_data->pause_start = al_get_time();
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->pause_total += al_get_time() - codec_data->pause_start;
	[codec_data->player play];
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->player)
	{
		[codec_data->player stop];
	}
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->player.currentTime = pos;
	codec_data->start_pos = pos;
	codec_data->start_time = al_get_time();
	codec_data->pause_start = codec_data->start_time;
	codec_data->pause_total = 0.0;
	return true;
//	[player currentPosition] = pos;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->player.currentTime;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->player.duration;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return al_get_time() - (codec_data->start_time + codec_data->pause_total) >= codec_data->player.duration - codec_data->start_pos;
}

static const char * codec_get_info(void * data)
{
	return "Apple AVPlayer";
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_avplayer_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	strcpy(codec_handler.id, "Apple AVPlayer");
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
	codec_handler.unload_file = codec_unload_file;
	codec_handler.get_tag = codec_get_tag;
	codec_handler.get_track_count = codec_get_track_count;
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
	omo_codec_handler_add_type(&codec_handler, ".mp2");
	omo_codec_handler_add_type(&codec_handler, ".mp3");
	omo_codec_handler_add_type(&codec_handler, ".mp4");
	omo_codec_handler_add_type(&codec_handler, ".m4a");
	omo_codec_handler_add_type(&codec_handler, ".aac");

	return &codec_handler;
}
