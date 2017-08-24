#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "../../defines.h"
#include "../../constants.h"
#include "../codec_handler.h"

static AVAudioPlayer * player = NULL;
static char player_filename[1024] = {0};
static double start_time;
static double pause_start;
static double pause_total;

static bool codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

	player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] error:nil];
	if(player)
	{
		strcpy(player_filename, fn);
		[player prepareToPlay];
		return true;
	}
	return false;
}

static void codec_unload_file(void)
{
	[player release];
	player = NULL;
}

static char tag_buffer[1024] = {0};

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

static const char * codec_get_tag(const char * name)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSString * path = [NSString stringWithUTF8String:player_filename];
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
			if(!strcmp(name, tag_name[i]))
			{
				break;
			}
		}
		NSArray *metadata = [asset metadata];
		for(AVMetadataItem* item in metadata)
		{
			NSString *key = [item identifier];
			NSString *value = [item stringValue];
			utf8_key = [key UTF8String];
			utf8_val = [value UTF8String];
			if(!strcasecmp(utf8_key, avplayer_tag_name[i]))
			{
				codec_strcpy(tag_buffer, utf8_val, 1024);
				[pool release];
				return tag_buffer;
			}
		}
	}
	[pool release];
	return NULL;
}

static int codec_get_track_count(const char * fn)
{
	return 1;
}

static bool codec_play(void)
{
	[player play];
	start_time = al_get_time();
	pause_total = 0.0;
	return true;
}

static bool codec_pause(void)
{
	[player stop];
	pause_start = al_get_time();
	return true;
}

static bool codec_resume(void)
{
	pause_total += al_get_time() - pause_start;
	[player play];
	return true;
}

static void codec_stop(void)
{
	if(player)
	{
		[player stop];
	}
}

static bool codec_seek(float pos)
{
//	player.currentTime = pos;
	return true;
//	[player currentPosition] = pos;
}

static float codec_get_position(void)
{
	return 0.0;
//	return player.currentTime;
}

static float codec_get_length(void)
{
	return 0.0;
//	return player.duration;
}

static bool codec_done_playing(void)
{
	return al_get_time() - (start_time + pause_total) >= player.duration;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_avplayer_get_codec_handler(void)
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
	codec_handler.seek = codec_seek;
	codec_handler.get_position = codec_get_position;
	codec_handler.get_length = codec_get_length;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".mp2");
	omo_codec_handler_add_type(&codec_handler, ".mp3");
	omo_codec_handler_add_type(&codec_handler, ".mp4");
	omo_codec_handler_add_type(&codec_handler, ".m4a");
	omo_codec_handler_add_type(&codec_handler, ".aac");

	return &codec_handler;
}
