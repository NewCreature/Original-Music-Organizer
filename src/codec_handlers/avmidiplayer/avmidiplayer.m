#include "t3f/t3f.h"
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>
#include "../../rtk/io_allegro.h"
#include "../../rtk/midi.h"

#include "../codec_handler.h"

typedef struct
{

	RTK_MIDI * midi;
	AVMIDIPlayer * player;
	double start_time;
	double pause_start;
	double pause_total;
	char tag_buffer[1024];

} CODEC_DATA;

static bool codec_init(void)
{
	rtk_io_set_allegro_driver();
	return true;
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	NSString * fnstring = [NSString stringWithUTF8String:fn];
	CODEC_DATA * data;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->player = [[AVMIDIPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:fnstring] soundBankURL:nil error:nil];
		if(data->player)
		{
			data->midi = rtk_load_midi(fn);
		}
		else
		{
			return NULL;
		}
	}

	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->midi)
	{
		rtk_destroy_midi(codec_data->midi);
	}
	[codec_data->player release];
	codec_data->player = NULL;
	free(data);
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	int i, j;

	if(codec_data->midi)
	{
		if(!strcmp(name, "Copyright"))
		{
			i = 0;
			for(j = 0; j < codec_data->midi->track[i]->events; j++)
			{
				if(codec_data->midi->track[i]->event[j]->type == RTK_MIDI_EVENT_TYPE_META && codec_data->midi->track[i]->event[j]->meta_type == RTK_MIDI_EVENT_META_TYPE_COPYRIGHT && codec_data->midi->track[i]->event[j]->text)
				{
					strcpy(codec_data->tag_buffer, codec_data->midi->track[i]->event[j]->text);
					return codec_data->tag_buffer;
				}
			}
		}
		else if(!strcmp(name, "Title"))
		{
			i = 0;
			for(j = 0; j < codec_data->midi->track[i]->events; j++)
			{
				if(codec_data->midi->track[i]->event[j]->type == RTK_MIDI_EVENT_TYPE_META && codec_data->midi->track[i]->event[j]->meta_type == RTK_MIDI_EVENT_META_TYPE_TRACK_NAME && codec_data->midi->track[i]->event[j]->text)
				{
					strcpy(codec_data->tag_buffer, codec_data->midi->track[i]->event[j]->text);
					return codec_data->tag_buffer;
				}
			}
		}
	}
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	[codec_data->player prepareToPlay];
	[codec_data->player play:nil];
	codec_data->start_time = al_get_time();
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
	[codec_data->player play:nil];
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

	codec_data->player.currentPosition = pos;
	return true;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->player.currentPosition;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->player.duration;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return al_get_time() - (codec_data->start_time + codec_data->pause_total) >= codec_data->player.duration;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_avmidiplayer_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	codec_handler.initialize = codec_init;
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
	omo_codec_handler_add_type(&codec_handler, ".mid");

	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}
