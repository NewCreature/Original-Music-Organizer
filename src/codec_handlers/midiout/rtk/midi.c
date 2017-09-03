#include <stdlib.h>
#include <memory.h>
#include "midi.h"
#include "io.h"

static unsigned long rtk_parse_var_len(unsigned char * data, unsigned long pos, unsigned long * bytes_used)
{	//bytes_used is set to the number of bytes long the variable length value is, the value itself is returned

	int cpos = pos;
	unsigned long val = *(&data[cpos]) & 0x7F;

	if(!data || !bytes_used)
	{
		return 0;
	}

	while(data[cpos] & 0x80)
	{
		cpos++;
		(*bytes_used)++;
		val <<= 7;
		val += (data[cpos] & 0x7F);
	}

	(*bytes_used)++;
	return val;
}

static int rtk_parse_midi(RTK_MIDI * mp, int pass)
{
	int midi_pos, midi_event, midi_event_type, midi_meta_event, last_midi_event;
	int i, j, track_pos, delta, track_event;
	unsigned long bytes_used;
	int d1, d2, d3, d4;

	/* allocate empty tracks on first pass */
	if(pass == 0)
	{
		mp->tracks = mp->raw_data->tracks;
		mp->track = malloc(sizeof(RTK_MIDI_TRACK *) * mp->tracks);
		if(mp->track)
		{
			for(i = 0; i < mp->tracks; i++)
			{
				mp->track[i] = malloc(sizeof(RTK_MIDI_TRACK));
				if(!mp->track[i])
				{
					return 0;
				}
				memset(mp->track[i], 0, sizeof(RTK_MIDI_TRACK));
			}
		}
	}

	/* allocate space for events on second pass */
	else if(pass == 1)
	{
		for(i = 0; i < mp->tracks; i++)
		{
			mp->track[i]->event = malloc(sizeof(RTK_MIDI_EVENT *) * mp->track[i]->events);
			if(mp->track[i]->event)
			{
				for(j = 0; j < mp->track[i]->events; j++)
				{
					mp->track[i]->event[j] = malloc(sizeof(RTK_MIDI_EVENT));
					if(mp->track[i]->event[j])
					{
						memset(mp->track[i]->event[j], 0, sizeof(RTK_MIDI_EVENT));
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				return 0;
			}
		}
	}

	for(i = 0; i < mp->raw_data->tracks; i++)
	{	//For each imported track
		last_midi_event = 0;	//Running status resets at beginning of each track
		midi_pos = 0;
		midi_event = 0;
		midi_event_type = 0;
		midi_meta_event = 0;

		track_pos = 0;
		track_event = 0;
		while(track_pos < mp->raw_data->track[i].len)
		{	//While the byte index of this MIDI track hasn't reached the end of the track data
			/* read delta */

			bytes_used = 0;
			delta = rtk_parse_var_len(mp->raw_data->track[i].data, track_pos, &bytes_used);
			midi_pos += delta;
			track_pos += bytes_used;

			/* read event type */
			if((midi_event_type >= 0x80) && (midi_event_type < 0xF0))
			{	//If the last loop iteration's event was normal
				last_midi_event = midi_event;	//Store it (including the original channel number)
			}
			midi_event = mp->raw_data->track[i].data[track_pos];

			if((midi_event & 0xF0) < 0x80)	//If this event is a running status event
			{
				midi_event = last_midi_event;	//Recall the previous normal event
			}
			else
			{
				track_pos++;	//Increment buffer pointer past the status byte
			}
			midi_event_type = midi_event & 0xF0;

			/* don't used filtered data for meta events */
			if(midi_event_type == 0xF0)
			{
				midi_event_type = midi_event;
			}

			if(pass == 1)
			{
				mp->track[i]->event[track_event]->type = midi_event_type;
				mp->track[i]->event[track_event]->channel = midi_event & 0x0F;
				mp->track[i]->event[track_event]->tick = midi_pos;
			}
			switch(midi_event_type)
			{
				case RTK_MIDI_EVENT_TYPE_NOTE_OFF:
				case RTK_MIDI_EVENT_TYPE_NOTE_ON:
				case RTK_MIDI_EVENT_TYPE_KEY_AFTER_TOUCH:
				case RTK_MIDI_EVENT_TYPE_CONTROLLER_CHANGE:
				case RTK_MIDI_EVENT_TYPE_PITCH_WHEEL_CHANGE:
				{
					d1 = mp->raw_data->track[i].data[track_pos++];
					d2 = mp->raw_data->track[i].data[track_pos++];
					if(pass == 1)
					{
						mp->track[i]->event[track_event]->data_i[0] = d1;
						mp->track[i]->event[track_event]->data_i[1] = d2;
					}
					break;
				}
				case RTK_MIDI_EVENT_TYPE_PROGRAM_CHANGE:
				case RTK_MIDI_EVENT_TYPE_CHANNEL_AFTER_TOUCH:
				{
					d1 = mp->raw_data->track[i].data[track_pos++];
					if(pass == 1)
					{
						mp->track[i]->event[track_event]->data_i[0] = d1;
					}
					break;
				}
				case RTK_MIDI_EVENT_TYPE_SYSEX_1:
				case RTK_MIDI_EVENT_TYPE_SYSEX_2:
				{
					bytes_used = 0;
					d3 = rtk_parse_var_len(mp->raw_data->track[i].data, track_pos, &bytes_used);
					track_pos += bytes_used;
					if(pass == 1)
					{
						memcpy(mp->track[i]->event[track_event]->data, &(mp->raw_data->track[i].data[track_pos]), d3);
					}
					track_pos += d3;
					break;
				}
				case RTK_MIDI_EVENT_TYPE_META:
				{
					midi_meta_event = mp->raw_data->track[i].data[track_pos];
					track_pos++;
					if(pass == 1)
					{
						mp->track[i]->event[track_event]->meta_type = midi_meta_event;
					}
					switch(midi_meta_event)
					{
						case RTK_MIDI_EVENT_META_TYPE_SEQUENCE_NUMBER:
						{
							d1 = mp->raw_data->track[i].data[track_pos++];
							d2 = mp->raw_data->track[i].data[track_pos++];
							d3 = mp->raw_data->track[i].data[track_pos++];
							if(pass == 1)
							{
								mp->track[i]->event[track_event]->data_i[0] = d1;
								mp->track[i]->event[track_event]->data_i[1] = (d2 << 8) + d3;
							}
							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_TEXT:
						case RTK_MIDI_EVENT_META_TYPE_COPYRIGHT:
						case RTK_MIDI_EVENT_META_TYPE_TRACK_NAME:
						case RTK_MIDI_EVENT_META_TYPE_INSTRUMENT_NAME:
						case RTK_MIDI_EVENT_META_TYPE_LYRIC:
						case RTK_MIDI_EVENT_META_TYPE_MARKER:
						case RTK_MIDI_EVENT_META_TYPE_CUE_POINT:
						case RTK_MIDI_EVENT_META_TYPE_SEQUENCER_INFO:
						{
							d1 = mp->raw_data->track[i].data[track_pos++];
							if(pass == 1)
							{
								memcpy(mp->track[i]->event[track_event]->text, &mp->raw_data->track[i].data[track_pos], d1);
							}
							track_pos += d1;
							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_CHANNEL_PREFIX:
						{
							track_pos += 2;
							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_END_OF_TRACK:
						{
							if(pass == 1)
							{
								mp->track[i]->event[track_event]->data_i[0] = 0;
							}
							track_pos += 1;
							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_TEMPO:
						{
							track_pos++;
							d1 = (mp->raw_data->track[i].data[track_pos++]);	//MPQN byte 1
							d2 = (mp->raw_data->track[i].data[track_pos++]);	//MPQN byte 2
							d3 = (mp->raw_data->track[i].data[track_pos++]);	//MPQN byte 3
							d4 = (d1 << 16) | (d2 << 8) | (d3);

							if(pass == 1)
							{
								mp->track[i]->event[track_event]->data_i[0] = d4;
							}

							/* count tempo events so we know how much space to allocate later */
							else if(pass == 0)
							{
								mp->tempo_events++;
							}
							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_TIME_SIGNATURE:
						{
							track_pos++;
							d1 = (mp->raw_data->track[i].data[track_pos++]);	//Numerator
							d2 = (mp->raw_data->track[i].data[track_pos++]);	//Denominator
							d3 = (mp->raw_data->track[i].data[track_pos++]);	//Metronome
							d4 = (mp->raw_data->track[i].data[track_pos++]);	//32nds

							if(pass == 1)
							{
								mp->track[i]->event[track_event]->data_i[0] = d1;
								mp->track[i]->event[track_event]->data_i[1] = d2;
								mp->track[i]->event[track_event]->data_i[2] = d3;
								mp->track[i]->event[track_event]->data_i[3] = d4;
							}

							break;
						}

						case RTK_MIDI_EVENT_META_TYPE_KEY_SIGNATURE:
						{
							char key;
							track_pos++;
							key = (mp->raw_data->track[i].data[track_pos++]);	//Key
							d2 = (mp->raw_data->track[i].data[track_pos++]);	//Scale
							if(pass == 1)
							{
								mp->track[i]->event[track_event]->data_i[0] = key;
								mp->track[i]->event[track_event]->data_i[1] = d2;
							}
							break;
						}

						default:
						{
							track_pos++;
							break;
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}//switch(current_event_hi)
			track_event++;
		}//While the byte index of this MIDI track hasn't reached the end of the track data
		if(pass == 0)
		{
			mp->track[i]->events = track_event;
		}
	}//For each imported track
	return 1;
}

static int rtk_tempo_map_qsort_callback(const void * e1, const void * e2)
{
	RTK_MIDI_EVENT ** thing1 = (RTK_MIDI_EVENT **)e1;
	RTK_MIDI_EVENT ** thing2 = (RTK_MIDI_EVENT **)e2;

	if((*thing1)->tick < (*thing2)->tick)
	{
		return -1;
	}
	if((*thing1)->tick > (*thing2)->tick)
	{
		return 1;
	}
	return 0;
}

/* convert ticks to real time */
static double rtk_tick_to_real_time(int division, double bpm, unsigned long ticks)
{
	return ticks / (double)division * ((double)60.0 / (bpm));
}

/* convert MIDI PPQN to BPM */
double rtk_ppqn_to_bpm(unsigned long ppqn)
{
	return 60000000.0 / (double)ppqn;
}

static void rtk_build_tempo_map(RTK_MIDI * mp)
{
	int i, j;
	int current_event = 0;
	double current_bpm = 120.0;
	unsigned long current_tick = 0;
	double current_time = 0.0;

	/* allocate storage for tempo events */
	mp->tempo_event = malloc(sizeof(RTK_MIDI_EVENT *) * mp->tempo_events);
	if(mp->tempo_event)
	{
		for(i = 0; i < mp->tempo_events; i++)
		{
			mp->tempo_event[i] = malloc(sizeof(RTK_MIDI_EVENT));
			if(mp->tempo_event[i])
			{
				memset(mp->tempo_event[i], 0, sizeof(RTK_MIDI_EVENT));
			}
			else
			{
				return;
			}
		}

		/* iterate through all events, copying any tempo change found */
		for(i = 0; i < mp->tracks; i++)
		{
			for(j = 0; j < mp->track[i]->events; j++)
			{
				if(mp->track[i]->event[j]->meta_type == RTK_MIDI_EVENT_META_TYPE_TEMPO)
				{
					memcpy(mp->tempo_event[current_event], mp->track[i]->event[j], sizeof(RTK_MIDI_EVENT));
					current_event++;
				}
			}
		}
		qsort(mp->tempo_event, mp->tempo_events, sizeof(RTK_MIDI_EVENT *), rtk_tempo_map_qsort_callback);

		/* generate real time data for tempo map */
		for(i = 0; i < mp->tempo_events; i++)
		{
			current_time += rtk_tick_to_real_time(mp->raw_data->divisions, current_bpm, mp->tempo_event[i]->tick - current_tick);
			mp->tempo_event[i]->pos_sec = current_time;
			mp->tempo_event[i]->pos_msec = mp->tempo_event[i]->pos_sec * 1000.0 + 0.5;
			current_bpm = rtk_ppqn_to_bpm(mp->tempo_event[i]->data_i[0]);
			current_tick = mp->tempo_event[i]->tick;
		}
	}
}

/* convert MIDI tick to real time (seconds), must be called after rtk_build_tempo_map() */
static double rtk_get_real_time(RTK_MIDI * mp, unsigned long tick)
{
	double current_bpm = 120.0; // default to 120 BPM (MIDI standard)
	int current_tempo_event = 0;
	double current_time = 0.0;
	int current_tick = 0;
	int i;

	if(mp->tempo_events)
	{
		for(i = 0; i < mp->tempo_events; i++)
		{
			if(mp->tempo_event[i]->tick >= tick)
			{
				break;
			}
		}
		if(i > 0)
		{
			current_tempo_event = i - 1;
		}
		if(tick >= mp->tempo_event[current_tempo_event]->tick)
		{
			current_bpm = rtk_ppqn_to_bpm(mp->tempo_event[current_tempo_event]->data_i[0]);
			current_time = mp->tempo_event[current_tempo_event]->pos_sec;
			current_tick = mp->tempo_event[current_tempo_event]->tick;
		}
	}

	/* add the remaining time from the tempo change to the desired tick */
	current_time += rtk_tick_to_real_time(mp->raw_data->divisions, current_bpm, tick - current_tick);

	return current_time;
}

/* iterate through all events and get real time data */
static void rtk_get_event_real_times(RTK_MIDI * mp)
{
	int i, j;

	/* build the tempo map so we can calculate the real times */
	rtk_build_tempo_map(mp);

	for(i = 0; i < mp->tracks; i++)
	{
		for(j = 0; j < mp->track[i]->events; j++)
		{
			mp->track[i]->event[j]->pos_sec = rtk_get_real_time(mp, mp->track[i]->event[j]->tick);
			mp->track[i]->event[j]->pos_msec = mp->track[i]->event[j]->pos_sec * 1000.0 + 0.5;
		}
	}
}

static void rtk_get_track_names(RTK_MIDI * mp)
{
	int i, j;

	for(i = 0; i < mp->tracks; i++)
	{
		for(j = 0; j < mp->track[i]->events; j++)
		{
			if(mp->track[i]->event[j]->meta_type == RTK_MIDI_EVENT_META_TYPE_TRACK_NAME)
			{
				strcpy(mp->track[i]->name, mp->track[i]->event[j]->text);
			}
		}
	}
}

RTK_MIDI * rtk_load_midi(const char * fn)
{
	int c;
	char buf[4];
	long data;
	void * fp;
	RTK_MIDI *midi;

	fp = rtk_io_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}

	midi = malloc(sizeof(RTK_MIDI));
	if(!midi)
	{
		rtk_io_fclose(fp);
		return NULL;
	}
	memset(midi, 0, sizeof(RTK_MIDI));

	midi->raw_data = malloc(sizeof(RTK_MIDI_DATA));
	if(!midi->raw_data)
	{
		free(midi);
		rtk_io_fclose(fp);
		return NULL;
	}

	rtk_io_fread(fp, buf, 4); /* read midi header */

	if(memcmp(buf, "MThd", 4))
	{
		goto err;
	}

	rtk_io_mgetl(fp);                           /* skip header chunk length */

	data = rtk_io_mgetw(fp);                    /* MIDI file type */
	if ((data != 0) && (data != 1))
	{
		goto err;
	}

	midi->raw_data->tracks = rtk_io_mgetw(fp);              /* number of tracks */
	if((midi->raw_data->tracks < 1) || (midi->raw_data->tracks > RTK_MAX_MIDI_TRACKS))
	{
		goto err;
	}

	data = rtk_io_mgetw(fp);                    /* beat divisions */
	midi->raw_data->divisions = abs(data);

	for(c=0; c < midi->raw_data->tracks; c++)
	{            /* read each track */
		rtk_io_fread(fp, buf, 4);                /* read track header */
		if(memcmp(buf, "MTrk", 4))
		{
			goto err;
		}

		data = rtk_io_mgetl(fp);                 /* length of track chunk */
		midi->raw_data->track[c].len = data;

		midi->raw_data->track[c].data = malloc(data); /* allocate memory */
		if(!midi->raw_data->track[c].data)
		{
			goto err;
		}

		/* finally, read track data */
		if(rtk_io_fread(fp, midi->raw_data->track[c].data, data) != data)
		{
			goto err;
		}
	}

	rtk_io_fclose(fp);

	/* convert raw MIDI data to something useful */
	rtk_parse_midi(midi, 0);
	rtk_parse_midi(midi, 1);
	rtk_get_event_real_times(midi);
	rtk_get_track_names(midi);
	return midi;

	/* oh dear... */
	err:
	{
		rtk_io_fclose(fp);
		rtk_destroy_midi(midi);
	}
	return NULL;
}

void rtk_destroy_midi(RTK_MIDI * mp)
{
	int i;

	for(i = 0; i < mp->tracks; i++)
	{
		free(mp->track[i]);
	}
	free(mp->track);
	free(mp->raw_data);
}

static double rtk_get_tick_sec(RTK_MIDI * mp, double bpm)
{
	return 1.0 / (mp->raw_data->divisions * bpm) / 60.0;
}

int rtk_sec_to_tick(RTK_MIDI * mp, float sec)
{
	double cur_sec = 0.0;
	double tick_sec = rtk_get_tick_sec(mp, 120.0);
	int tick = 0;
	int tempo_change = 0;
	int i;

	while(cur_sec < sec)
	{
		if(tempo_change < mp->tempo_events)
		{
			if(tick >= mp->tempo_event[tempo_change]->tick)
			{
				tick_sec = rtk_get_tick_sec(mp, rtk_ppqn_to_bpm(mp->tempo_event[tempo_change]->data_i[0]));
				cur_sec = mp->tempo_event[tempo_change]->pos_sec;
				tempo_change++;
			}
		}
		cur_sec += tick_sec;
		if(cur_sec >= sec)
		{
			return tick;
		}
		tick++;
	}
	return 0;
}

int rtk_add_midi_event_sec(RTK_MIDI * mp, int track, float sec, int type, int meta, int channel, char * data, int data_size)
{
	RTK_MIDI_EVENT ** event = NULL;
	int i;

	/* expand event */
	event = mp->track[track]->event;
	event = malloc(sizeof(RTK_MIDI_EVENT *) * mp->track[track]->events + 1);
	if(event)
	{
		for(i = 0; i < mp->track[track]->events; i++)
		{
			event[i] = mp->track[track]->event[i];
		}
		event[mp->track[track]->events] = malloc(sizeof(RTK_MIDI_EVENT));
		if(event[mp->track[track]->events])
		{
			memset(event[mp->track[track]->events], 0, sizeof(RTK_MIDI_EVENT));
			event[mp->track[track]->events]->tick = rtk_sec_to_tick(mp, sec);
			event[mp->track[track]->events]->type = type;
			event[mp->track[track]->events]->meta_type = meta;
			event[mp->track[track]->events]->channel = channel;
			if(data)
			{
				for(i = 0; i < data_size; i++)
				{
					event[mp->track[track]->events]->data[i] = data[i];
				}
			}
			free(mp->track[track]->event);
			mp->track[track]->event = event;
			return 1;
		}
	}

	if(event)
	{
		free(event);
	}
	return 0;
}

void rtk_delete_midi_event(RTK_MIDI * mp, int track, int event)
{
	int i;

	if(track < mp->tracks)
	{
		if(event < mp->track[track]->events)
		{
			free(mp->track[track]->event[event]);
			for(i = event; i < mp->track[track]->events - 1; i++)
			{
				mp->track[track]->event[i] = mp->track[track]->event[i + 1];
			}
			mp->track[track]->events--;
		}
	}
}
