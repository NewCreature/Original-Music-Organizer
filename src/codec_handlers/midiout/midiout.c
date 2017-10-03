#include "t3f/t3f.h"
#include "../../rtk/io_allegro.h"
#include "../../rtk/midi.h"

#include <windows.h>
#include <mmsystem.h>

#include "../codec_handler.h"

typedef struct
{

	HMIDIOUT output_device;
	MIDIOUTCAPS output_device_caps;
	RTK_MIDI * midi;
	bool paused;
	double elapsed_time;
	double end_time;
	ALLEGRO_THREAD * thread;
	char tag_buffer[1024];

} CODEC_DATA;

static bool codec_init(void)
{
	rtk_io_set_allegro_driver();
	return true;
}

static void * codec_load_file(const char * fn, const char * subfn)
{
	CODEC_DATA * data;
	int i;

	data = malloc(sizeof(CODEC_DATA));
	if(data)
	{
		memset(data, 0, sizeof(CODEC_DATA));
		data->midi = rtk_load_midi(fn);
		if(!data->midi)
		{
			free(data);
			return NULL;
		}
		data->end_time = 0.0;
		for(i = 0; i < data->midi->tracks; i++)
		{
			if(data->midi->track[i]->event[data->midi->track[i]->events - 1]->pos_sec > data->end_time)
			{
				data->end_time = data->midi->track[i]->event[data->midi->track[i]->events - 1]->pos_sec;
			}
		}
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	rtk_destroy_midi(codec_data->midi);
	free(data);
}

static const char * codec_get_tag(void * data, const char * name)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	int i, j;

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
	return NULL;
}

static int codec_get_track_count(void * data, const char * fn)
{
	return 1;
}

static void send_data(HMIDIOUT midi_out, int midi_data)
{
	int message_length[8] = {3, 3, 3, 3, 2, 2, 3, 0};
	static int midi_message;
	static int midi_message_length;
	static int midi_message_pos;

	if(midi_data >= 0x80)
	{
		midi_message_length = message_length[(midi_data >> 4) & 0x07];
		midi_message = 0;
		midi_message_pos = 0;
	}
	if(midi_message_length > 0)
	{
		midi_message |= ((unsigned long)midi_data) << (midi_message_pos * 8);
		midi_message_pos++;
		if(midi_message_pos == midi_message_length)
		{
			midiOutShortMsg(midi_out, midi_message);
		}
	}
}

static unsigned long get_next_event_tick(RTK_MIDI * mp, unsigned long current_tick, int * track_event, double * event_time)
{
	int i, j;
	unsigned long track_tick[32] = {0};
	double track_time[32] = {0.0};
	unsigned long largest_tick = 0;
	unsigned long shortest_tick;
	double shortest_time = 10000.0;

	for(i = 0; i < mp->tracks; i++)
	{
		for(j = track_event[i]; j < mp->track[i]->events; j++)
		{
			if(mp->track[i]->event[j]->tick > current_tick)
			{
				track_tick[i] = mp->track[i]->event[j]->tick;
				track_time[i] = mp->track[i]->event[j]->pos_sec;
				if(track_tick[i] > largest_tick)
				{
					largest_tick = track_tick[i];
				}
				break;
			}
		}
	}
	shortest_tick = largest_tick;
	for(i = 0; i < mp->tracks; i++)
	{
		if(track_event[i] < mp->track[i]->events && track_tick[i] <= shortest_tick)
		{
			shortest_tick = track_tick[i];
			shortest_time = track_time[i];
		}
	}
	*event_time = shortest_time;
	return shortest_tick;
}

static void send_midi_event_data(HMIDIOUT midi_out, RTK_MIDI * mp, unsigned long tick, int * track_event)
{
	int i;

	for(i = 0; i < mp->tracks; i++)
	{
		while(track_event[i] < mp->track[i]->events && mp->track[i]->event[track_event[i]]->tick == tick)
		{
			switch(mp->track[i]->event[track_event[i]]->type)
			{
				case RTK_MIDI_EVENT_TYPE_NOTE_OFF:
				case RTK_MIDI_EVENT_TYPE_NOTE_ON:
				case RTK_MIDI_EVENT_TYPE_KEY_AFTER_TOUCH:
				case RTK_MIDI_EVENT_TYPE_CONTROLLER_CHANGE:
				case RTK_MIDI_EVENT_TYPE_PITCH_WHEEL_CHANGE:
				{
					send_data(midi_out, mp->track[i]->event[track_event[i]]->type + mp->track[i]->event[track_event[i]]->channel);
					send_data(midi_out, mp->track[i]->event[track_event[i]]->data_i[0]);
					send_data(midi_out, mp->track[i]->event[track_event[i]]->data_i[1]);
					break;
				}
				case RTK_MIDI_EVENT_TYPE_PROGRAM_CHANGE:
				case RTK_MIDI_EVENT_TYPE_CHANNEL_AFTER_TOUCH:
				{
					send_data(midi_out, mp->track[i]->event[track_event[i]]->type + mp->track[i]->event[track_event[i]]->channel);
					send_data(midi_out, mp->track[i]->event[track_event[i]]->data_i[0]);
					break;
				}
			}
			track_event[i]++;
		}
	}
}

static void * codec_thread_proc(ALLEGRO_THREAD * thread, void * arg)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_TIMER * timer;
	ALLEGRO_EVENT event;
	unsigned long current_tick = 0;
	double tick_time = 1.0 / 1000.0;
	double event_time = 0;
	int midi_event[32] = {0};

	queue = al_create_event_queue();
	if(!queue)
	{
		return NULL;
	}
	timer = al_create_timer(tick_time);
	if(!timer)
	{
		al_destroy_event_queue(queue);
		return NULL;
	}
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);
	send_midi_event_data(codec_data->output_device, codec_data->midi, current_tick, midi_event);
	current_tick = get_next_event_tick(codec_data->midi, current_tick, midi_event, &event_time);
	while(!al_get_thread_should_stop(thread) && codec_data->elapsed_time < codec_data->end_time)
	{
		al_wait_for_event(queue, &event);
		if(!codec_data->paused)
		{
			codec_data->elapsed_time += tick_time;
			if(codec_data->elapsed_time >= event_time)
			{
				send_midi_event_data(codec_data->output_device, codec_data->midi, current_tick, midi_event);
				current_tick = get_next_event_tick(codec_data->midi, current_tick, midi_event, &event_time);
			}
		}
	}
	al_unregister_event_source(queue, al_get_timer_event_source(timer));
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	return NULL;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(midiOutOpen(&codec_data->output_device, MIDI_MAPPER, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
	{
		midiOutReset(codec_data->output_device);
		codec_data->thread = al_create_thread(codec_thread_proc, codec_data);
		if(codec_data->thread)
		{
			al_start_thread(codec_data->thread);
		}
		return true;
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->paused = true;
	midiOutReset(codec_data->output_device);
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	codec_data->paused = false;
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_join_thread(codec_data->thread, NULL);
	midiOutClose(codec_data->output_device);
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->elapsed_time;
}

static bool codec_done_playing(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	if(codec_data->elapsed_time >= codec_data->end_time)
	{
		return true;
	}
	return false;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_midiout_get_codec_handler(void)
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
	codec_handler.seek = NULL;
	codec_handler.get_position = codec_get_position;
	codec_handler.get_length = NULL;
	codec_handler.done_playing = codec_done_playing;
	codec_handler.types = 0;
	omo_codec_handler_add_type(&codec_handler, ".mid");
	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}
