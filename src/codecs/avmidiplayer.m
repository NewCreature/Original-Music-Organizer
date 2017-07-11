#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

#include "player.h"

static AVMIDIPlayer * player = NULL;

bool omo_codec_midi_load_file(const char * fn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];

//	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:@"data/test.mid"] soundBankURL:nil error:nil];
	player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] soundBankURL:nil error:nil];
	[player prepareToPlay];

	return true;
}

bool omo_codec_midi_play(void)
{
	[player play:nil];
	return true;
}

bool omo_codec_midi_pause(bool paused)
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

void omo_codec_midi_stop(void)
{
	if(player)
	{
		[player stop];
		[player release];
		player = NULL;
	}
}

bool omo_codec_midi_seek(float pos)
{
	player.currentPosition = pos;
	return true;
//	[player currentPosition] = pos;
}

float omo_codec_midi_get_position(void)
{
	return player.currentPosition;
}

float omo_codec_midi_get_length(void)
{
	return player.duration;
}

static OMO_PLAYER omo_midi_player;

OMO_PLAYER * omo_codec_avmidiplayer_get_player(void)
{
	omo_midi_player.initialize = NULL;
	omo_midi_player.load_file = omo_codec_midi_load_file;
	omo_midi_player.play = omo_codec_midi_play;
	omo_midi_player.pause = omo_codec_midi_pause;
	omo_midi_player.stop = omo_codec_midi_stop;
	omo_midi_player.seek = omo_codec_midi_seek;
	omo_midi_player.get_position = omo_codec_midi_get_position;
	omo_midi_player.get_length = omo_codec_midi_get_length;
	omo_midi_player.types = 0;
	omo_player_add_type(&omo_midi_player, ".mid");

	return &omo_midi_player;
}
