#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "player.h"

static AVAudioPlayer * player = NULL;

static bool codec_load_file(const char * fn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

	player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] error:nil];
	[player prepareToPlay];

	return true;
}

static bool codec_play(void)
{
	[player play];
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
		[player play];
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

static OMO_PLAYER codec_player;

OMO_PLAYER * omo_codec_avplayer_get_player(void)
{
	codec_player.initialize = NULL;
	codec_player.load_file = codec_load_file;
	codec_player.play = codec_play;
	codec_player.pause = codec_pause;
	codec_player.stop = codec_stop;
	codec_player.seek = codec_seek;
	codec_player.get_position = codec_get_position;
	codec_player.get_length = codec_get_length;
	codec_player.types = 0;
	omo_player_add_type(&codec_player, ".mp2");
	omo_player_add_type(&codec_player, ".mp3");
	omo_player_add_type(&codec_player, ".mp4");
	omo_player_add_type(&codec_player, ".m4a");
	omo_player_add_type(&codec_player, ".aac");

	return &codec_player;
}
