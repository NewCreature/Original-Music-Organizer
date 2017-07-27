#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "../codec_handler.h"

static AVMIDIPlayer * player = NULL;
static OMO_CODEC_HANDLER codec_handler;
static double start_time;
static double pause_start;
static double pause_total;

static bool codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

//	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:@"data/test.mid"] soundBankURL:nil error:nil];
	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] soundBankURL:nil error:nil];
	[player prepareToPlay];

	return true;
}

static int codec_get_track_count(const char * fn)
{
	return 1;
}

static bool codec_play(void)
{
	[player play:nil];
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
	[player play:nil];
	return true;
}

static void codec_stop(void)
{
	if(player)
	{
		[player stop];
		[player release];
		player = NULL;
	}
}

static bool codec_seek(float pos)
{
	player.currentPosition = pos;
	return true;
//	[player currentPosition] = pos;
}

static float codec_get_position(void)
{
	return player.currentPosition;
}

static float codec_get_length(void)
{
	return player.duration;
}

static bool codec_done_playing(void)
{
	return al_get_time() - (start_time + pause_total) >= player.duration;
}

OMO_CODEC_HANDLER * omo_codec_avmidiplayer_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = NULL;
	codec_handler.load_file = codec_load_file;
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
	omo_codec_handler_add_type(&codec_handler, ".mid");

	return &codec_handler;
}
