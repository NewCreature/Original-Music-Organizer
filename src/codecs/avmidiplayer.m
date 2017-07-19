#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "player.h"

static AVMIDIPlayer * player = NULL;
static OMO_PLAYER codec_player;
static double start_time;

static bool codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

//	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:@"data/test.mid"] soundBankURL:nil error:nil];
	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] soundBankURL:nil error:nil];
	[player prepareToPlay];

	return true;
}

static bool codec_play(void)
{
	[player play:nil];
	start_time = al_get_time();
	return true;
}

static bool codec_pause(bool paused)
{
	if(paused)
	{
		[player stop];
	}
	else
	{
		[player play:nil];
	}
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
	return al_get_time() - start_time >= player.duration;
}

OMO_PLAYER * omo_codec_avmidiplayer_get_player(void)
{
	memset(&codec_player, 0, sizeof(OMO_PLAYER));
	codec_player.initialize = NULL;
	codec_player.load_file = codec_load_file;
	codec_player.play = codec_play;
	codec_player.pause = codec_pause;
	codec_player.stop = codec_stop;
	codec_player.seek = codec_seek;
	codec_player.get_position = codec_get_position;
	codec_player.get_length = codec_get_length;
	codec_player.done_playing = codec_done_playing;
	codec_player.types = 0;
	omo_player_add_type(&codec_player, ".mid");

	return &codec_player;
}
