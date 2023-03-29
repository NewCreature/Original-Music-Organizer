#include "t3f/t3f.h"
#include "../../rtk/io_allegro.h"
#include "../../rtk/midi.h"
#include "MIDIA5/midia5.h"

#include "../codec_handler.h"

typedef struct
{

	MIDIA5_OUTPUT_HANDLE * midia5_output;
	int device;
	char codec_info[1024];
	RTK_MIDI * midi;
	bool paused;
	double elapsed_time;
	double end_time;
	char tag_buffer[1024];
	ALLEGRO_MUTEX * mutex;
	ALLEGRO_THREAD * thread;
	unsigned long current_tick;
	double event_time;
	int midi_event[32];
	float volume;
	bool fluidsynth_started; // did we start FluidSynth?

} CODEC_DATA;

#ifdef ALLEGRO_UNIX
	#ifndef ALLEGRO_MACOSX
		static int get_fluidsynth_device(void)
		{
			int device_count;
			int i;

			device_count = midia5_get_output_device_count();
			for(i = 0; i < device_count; i++)
			{
				if(!memcmp(midia5_get_output_device_name(i), "FLUID", 5))
				{
					return i;
				}
			}
			return -1;
		}

		static char * get_helper_script_path(char * buf, int buf_size)
		{
			ALLEGRO_PATH * path;

			path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
			if(path)
			{
				al_set_path_filename(path, "omo-helper.sh");
				strcpy(buf, al_path_cstr(path, '/'));
				al_destroy_path(path);
				return buf;
			}
			return NULL;
		}

		static int start_fluidsynth(void)
		{
			char command[1024];

			if(get_helper_script_path(command, 1024))
			{
				strcat(command, " start");
				return system(command);
			}
			return 0;
		}

		static int stop_fluidsynth(void)
		{
			char command[1024];

			if(get_helper_script_path(command, 1024))
			{
				strcat(command, " stop");
				return system(command);
			}
			return 0;
		}

		static void enable_fluidsynth(CODEC_DATA * data)
		{
			int try = 0;
			int i;

			if(get_fluidsynth_device() < 0)
			{
				for(i = 0; i < 5; i++)
				{
					start_fluidsynth();
					al_rest(0.5);
					while(get_fluidsynth_device() < 0 && try < 5)
					{
						printf("FluidSynth device not found. Retrying...\n");
						al_rest(0.5);
						try++;
					}
					if(try < 5)
					{
						data->fluidsynth_started = true;
						break;
					}
					else
					{
						stop_fluidsynth();
						try = 0;
					}
				}
			}
			else
			{
				data->fluidsynth_started = false;
			}
		}
	#endif
#endif

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
		data->mutex = al_create_mutex();
		if(!data->mutex)
		{
			rtk_destroy_midi(data->midi);
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
		data->volume = 1.0;
	}
	return data;
}

static void codec_unload_file(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_destroy_mutex(codec_data->mutex);
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

static bool codec_set_volume(void * data, float volume)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	al_lock_mutex(codec_data->mutex);
	codec_data->volume = volume;
	al_unlock_mutex(codec_data->mutex);
	return true;
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

static void send_midi_event_data(MIDIA5_OUTPUT_HANDLE * midi_out, RTK_MIDI * mp, unsigned long tick, int * track_event, float volume)
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
					midia5_send_data(midi_out, mp->track[i]->event[track_event[i]]->type + mp->track[i]->event[track_event[i]]->channel);
					midia5_send_data(midi_out, mp->track[i]->event[track_event[i]]->data_i[0]);
					midia5_send_data(midi_out, (float)mp->track[i]->event[track_event[i]]->data_i[1] * volume);
					break;
				}
				case RTK_MIDI_EVENT_TYPE_PROGRAM_CHANGE:
				case RTK_MIDI_EVENT_TYPE_CHANNEL_AFTER_TOUCH:
				{
					midia5_send_data(midi_out, mp->track[i]->event[track_event[i]]->type + mp->track[i]->event[track_event[i]]->channel);
					midia5_send_data(midi_out, mp->track[i]->event[track_event[i]]->data_i[0]);
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
	double tick_time = 1.0 / 1000.0;

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
	send_midi_event_data(codec_data->midia5_output, codec_data->midi, codec_data->current_tick, codec_data->midi_event, codec_data->volume);
	codec_data->current_tick = get_next_event_tick(codec_data->midi, codec_data->current_tick, codec_data->midi_event, &codec_data->event_time);
	while(!al_get_thread_should_stop(thread) && codec_data->elapsed_time < codec_data->end_time)
	{
		al_wait_for_event(queue, &event);
		if(!codec_data->paused)
		{
			al_lock_mutex(codec_data->mutex);
			codec_data->elapsed_time += tick_time;
			if(codec_data->elapsed_time >= codec_data->event_time)
			{
				send_midi_event_data(codec_data->midia5_output, codec_data->midi, codec_data->current_tick, codec_data->midi_event, codec_data->volume);
				codec_data->current_tick = get_next_event_tick(codec_data->midi, codec_data->current_tick, codec_data->midi_event, &codec_data->event_time);
			}
			al_unlock_mutex(codec_data->mutex);
		}
	}
	al_unregister_event_source(queue, al_get_timer_event_source(timer));
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	midia5_reset_output_device(codec_data->midia5_output);
	return NULL;
}

static int get_midi_device(void)
{
	const char * val;
	int val_i;
	int device_count;
	int i;

	device_count = midia5_get_output_device_count();
	val = al_get_config_value(t3f_config, "Settings", "midi_device");
	if(val)
	{
		val_i = atoi(val);
		if(val_i < device_count)
		{
			return val_i;
		}
	}
	else
	{
		/* look for FLUID Synth */
		for(i = 0; i < device_count; i++)
		{
			if(!memcmp(midia5_get_output_device_name(i), "FLUID", 5))
			{
				return i;
			}
		}
	}
	if(device_count < 0)
	{
		return -1;
	}
	return 0;
}

static bool codec_play(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	#ifdef ALLEGRO_UNIX
		#ifndef ALLEGRO_MACOSX
			enable_fluidsynth(codec_data);
		#endif
	#endif
	codec_data->device = get_midi_device();
	if(codec_data->device >= 0)
	{
		try_device:
		{
			codec_data->midia5_output = midia5_create_output_handle(codec_data->device);
			if(codec_data->midia5_output)
			{
				midia5_reset_output_device(codec_data->midia5_output);
				codec_data->thread = al_create_thread(codec_thread_proc, codec_data);
				if(codec_data->thread)
				{
					al_start_thread(codec_data->thread);
				}
				sprintf(codec_data->codec_info, "MIDIA5: %s", midia5_get_output_device_name(codec_data->device));
				return true;
			}
			else if(codec_data->device != 0)
			{
				printf("Failed to open MIDI device %d. Retry with device 0.", codec_data->device);
				codec_data->device = 0;
				goto try_device;
			}
		}
	}
	return false;
}

static bool codec_pause(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	#ifdef ALLEGRO_UNIX
		#ifndef ALLEGRO_MACOSX
			if(codec_data->fluidsynth_started)
			{
				stop_fluidsynth();
				codec_data->fluidsynth_started = false;
			}
		#endif
	#endif
	codec_data->paused = true;
	midia5_reset_output_device(codec_data->midia5_output);
	return true;
}

static bool codec_resume(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	#ifdef ALLEGRO_UNIX
		#ifndef ALLEGRO_MACOSX
			enable_fluidsynth(codec_data);
		#endif
	#endif
	codec_data->paused = false;
	return true;
}

static void codec_stop(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	#ifdef ALLEGRO_UNIX
		#ifndef ALLEGRO_MACOSX
			if(codec_data->fluidsynth_started)
			{
				stop_fluidsynth();
				codec_data->fluidsynth_started = false;
			}
		#endif
	#endif
	al_join_thread(codec_data->thread, NULL);
	midia5_destroy_output_handle(codec_data->midia5_output);
}

static bool codec_seek(void * data, double pos)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;
	unsigned long current_tick;
	unsigned long current_event;
	double current_time;
	unsigned long target_tick = ~0;
	double target_time = 100000.0;
	int track_program_change[128][16];
	int i, j;

	for(i = 0; i < 128; i++)
	{
		for(j = 0; j < 16; j++)
		{
			track_program_change[i][j] = -1;
		}
	}
	al_lock_mutex(codec_data->mutex);
	midia5_reset_output_device(codec_data->midia5_output);
	for(i = 0; i < codec_data->midi->tracks; i++)
	{
		current_tick = 0;
		current_event = 0;
		current_time = 0;
		for(j = 0; j < codec_data->midi->track[i]->events; j++)
		{
			/* make note of latest program change */
			if(codec_data->midi->track[i]->event[j]->type == RTK_MIDI_EVENT_TYPE_PROGRAM_CHANGE)
			{
				track_program_change[i][codec_data->midi->track[i]->event[j]->channel] = j;
			}
			if(codec_data->midi->track[i]->event[j]->pos_sec >= pos)
			{
				codec_data->midi_event[i] = current_event;
				if(current_tick < target_tick)
				{
					target_tick = current_tick;
					target_time = current_time;
				}
				break;
			}
			if(codec_data->midi->track[i]->event[j]->tick > current_tick)
			{
				current_tick = codec_data->midi->track[i]->event[j]->tick;
				current_event = j;
				current_time = codec_data->midi->track[i]->event[j]->pos_sec;
			}
		}
	}

	/* send program changes for all tracks and channels */
	for(i = 0; i < codec_data->midi->tracks; i++)
	{
		for(j = 0; j < 16; j++)
		{
			if(track_program_change[i][j] >= 0)
			{
				midia5_send_data(codec_data->midia5_output, codec_data->midi->track[i]->event[track_program_change[i][j]]->type + j);
				midia5_send_data(codec_data->midia5_output, codec_data->midi->track[i]->event[track_program_change[i][j]]->data_i[0]);
			}
		}
	}
	codec_data->current_tick = target_tick;
	codec_data->event_time = target_time;
	codec_data->elapsed_time = pos;
	al_unlock_mutex(codec_data->mutex);

	return true;
}

static double codec_get_position(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->elapsed_time;
}

static double codec_get_length(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->end_time;
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

static const char * codec_get_info(void * data)
{
	CODEC_DATA * codec_data = (CODEC_DATA *)data;

	return codec_data->codec_info;
}

static OMO_CODEC_HANDLER codec_handler;

OMO_CODEC_HANDLER * omo_codec_midia5_get_codec_handler(void)
{
	memset(&codec_handler, 0, sizeof(OMO_CODEC_HANDLER));
	strcpy(codec_handler.id, "MIDIA5");
	codec_handler.initialize = codec_init;
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
	omo_codec_handler_add_type(&codec_handler, ".mid");
	omo_codec_handler_add_type(&codec_handler, ".rmi");
	if(codec_handler.initialize())
	{
		return &codec_handler;
	}
	return NULL;
}
