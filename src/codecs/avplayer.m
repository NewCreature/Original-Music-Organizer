#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "player.h"

static AVAudioPlayer * player = NULL;

bool omo_codec_av_load_file(const char * fn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

	player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] error:nil];
	[player prepareToPlay];

	return true;
}

bool omo_codec_av_play(void)
{
	[player play];
	return true;
}

bool omo_codec_av_pause(bool paused)
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

void omo_codec_av_stop(void)
{
	if(player)
	{
		[player stop];
		[player release];
		player = NULL;
	}
}

bool omo_codec_av_seek(float pos)
{
//	player.currentTime = pos;
	return true;
//	[player currentPosition] = pos;
}

float omo_codec_av_get_position(void)
{
	return 0.0;
//	return player.currentTime;
}

float omo_codec_av_get_length(void)
{
	return 0.0;
//	return player.duration;
}

static OMO_PLAYER omo_av_player;

OMO_PLAYER * omo_codec_avplayer_get_player(void)
{
	omo_av_player.initialize = NULL;
	omo_av_player.load_file = omo_codec_av_load_file;
	omo_av_player.play = omo_codec_av_play;
	omo_av_player.pause = omo_codec_av_pause;
	omo_av_player.stop = omo_codec_av_stop;
	omo_av_player.seek = omo_codec_av_seek;
	omo_av_player.get_position = omo_codec_av_get_position;
	omo_av_player.get_length = omo_codec_av_get_length;
	omo_av_player.types = 0;
	omo_player_add_type(&omo_av_player, ".mp2");
	omo_player_add_type(&omo_av_player, ".mp3");
	omo_player_add_type(&omo_av_player, ".mp4");
	omo_player_add_type(&omo_av_player, ".m4a");
	omo_player_add_type(&omo_av_player, ".aac");

	return &omo_av_player;
}
