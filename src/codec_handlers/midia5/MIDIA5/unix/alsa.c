#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "midia5.h"

typedef struct
{

	snd_seq_t * sequencer;
	snd_seq_addr_t addr;
	int in_port[4];
	int out_port[4];
	int command_step;
	int command_type;
	int command_channel;
	int command_data[16];
	int command[16];
	snd_seq_addr_t port;

} MIDIA5_ALSA_DATA;

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
    MIDIA5_ALSA_DATA * cm_data;
	int err;

    cm_data = malloc(sizeof(MIDIA5_ALSA_DATA));
    if(cm_data)
    {
		cm_data->command_step = 0;
		if(snd_seq_open(&cm_data->sequencer, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0)
		{
			fprintf(stderr, "Error opening ALSA sequencer.\n");
			free(cm_data);
			return NULL;
	 	}
		snd_seq_set_client_name(cm_data->sequencer, "MIDIA5");
		snd_seq_parse_address(cm_data->sequencer, &cm_data->addr, "FLUID:0");
    }
    return cm_data;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
    MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;

    free(cm_data);
}

static int get_alsa_pitch_bend_value(int d1, int d2)
{
	return ((d1 & 127) | ((d2 & 127) << 7)) - 0x2000;
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
    MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;
	snd_seq_event_t ev;

	switch(cm_data->command_step)
	{
		case 0:
		{
			cm_data->command_type = data & 0xF0;
			cm_data->command_channel = data & 0x0F;
			switch(cm_data->command_type)
			{
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
				case 0xC0:
				case 0xD0:
				{
					cm_data->command_step = 1;
					break;
				}
			}
			break;
		}
		case 1:
		{
			cm_data->command_data[0] = data;
			switch(cm_data->command_type)
			{
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
				{
					cm_data->command_step = 2;
					break;
				}
				case 0xC0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_pgmchange(&ev, cm_data->command_channel, cm_data->command_data[0]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xD0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_chanpress(&ev, cm_data->command_channel, cm_data->command_data[0]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
			}
			break;
		}
		case 2:
		{
			cm_data->command_data[1] = data;
			switch(cm_data->command_type)
			{
				case 0x80:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_noteoff(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0x90:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_noteon(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xA0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_keypress(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xB0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_controller(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xE0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_pitchbend(&ev, cm_data->command_channel, get_alsa_pitch_bend_value(cm_data->command_data[0], cm_data->command_data[1]));
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
			}
			break;
		}
	}
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
	MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;
	int i, j;

	for(i = 0; i < 0xF; i++)
	{
		for(j = 0; j < 128; j++)
		{
			midia5_send_data(hp, 0x80 | i);
			midia5_send_data(hp, j);
			midia5_send_data(hp, 127);
		}
	}
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
    MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;

	return false;
}
