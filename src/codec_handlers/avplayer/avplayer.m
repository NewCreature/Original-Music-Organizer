#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "../codec_handler.h"

static AVAudioPlayer * player = NULL;
static double start_time;
static double pause_start;
static double pause_total;

static bool codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

	player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] error:nil];
	[player prepareToPlay];

	return true;
}

static void codec_unload_file(void)
{
	[player release];
	player = NULL;
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
