#ifndef LIBRTK_MIDI_H
#define LIBRTK_MIDI_H

#define RTK_MAX_MIDI_TRACKS               32
#define RTK_MAX_MIDI_EVENT_DATA_LENGTH   256
#define RTK_MAX_MIDI_EVENT_DATA           16
#define RTK_MAX_MIDI_TRACK_NAME_LENGTH    64
#define RTK_MAX_MIDI_EVENTS            65536

/* event types */
#define RTK_MIDI_EVENT_TYPE_NOTE_OFF             0x80
#define RTK_MIDI_EVENT_TYPE_NOTE_ON              0x90
#define RTK_MIDI_EVENT_TYPE_KEY_AFTER_TOUCH      0xA0
#define RTK_MIDI_EVENT_TYPE_CONTROLLER_CHANGE    0xB0
#define RTK_MIDI_EVENT_TYPE_PITCH_WHEEL_CHANGE   0xE0
#define RTK_MIDI_EVENT_TYPE_PROGRAM_CHANGE       0xC0
#define RTK_MIDI_EVENT_TYPE_CHANNEL_AFTER_TOUCH  0xD0
#define RTK_MIDI_EVENT_TYPE_META                 0xFF
#define RTK_MIDI_EVENT_TYPE_SYSEX_1              0xF0
#define RTK_MIDI_EVENT_TYPE_SYSEX_2              0xF7

/* meta event types */
#define RTK_MIDI_EVENT_META_TYPE_SEQUENCE_NUMBER 0x00
#define RTK_MIDI_EVENT_META_TYPE_TEXT            0x01
#define RTK_MIDI_EVENT_META_TYPE_COPYRIGHT       0x02
#define RTK_MIDI_EVENT_META_TYPE_TRACK_NAME      0x03
#define RTK_MIDI_EVENT_META_TYPE_INSTRUMENT_NAME 0x04
#define RTK_MIDI_EVENT_META_TYPE_LYRIC           0x05
#define RTK_MIDI_EVENT_META_TYPE_MARKER          0x06
#define RTK_MIDI_EVENT_META_TYPE_CUE_POINT       0x07
#define RTK_MIDI_EVENT_META_TYPE_CHANNEL_PREFIX  0x20
#define RTK_MIDI_EVENT_META_TYPE_SEQUENCER_INFO  0x7F
#define RTK_MIDI_EVENT_META_TYPE_END_OF_TRACK    0x2F
#define RTK_MIDI_EVENT_META_TYPE_TEMPO           0x51
#define RTK_MIDI_EVENT_META_TYPE_TIME_SIGNATURE  0x58
#define RTK_MIDI_EVENT_META_TYPE_KEY_SIGNATURE   0x59

/* structure to hold data for a single MIDI track */
typedef struct
{

	unsigned char * data;            /* MIDI message stream */
	int len;                         /* length of the track data */

} RTK_RAW_MIDI_TRACK;

/* structure to hold raw MIDI data */
typedef struct
{

	int divisions;                      /* number of ticks per quarter note */
	int tracks;
	RTK_RAW_MIDI_TRACK track[RTK_MAX_MIDI_TRACKS];

} RTK_MIDI_DATA;

/* a parsed MIDI event */
typedef struct
{

	int type, channel, meta_type;

	int tick; /* absolute MIDI tick */
	char raw_data[RTK_MAX_MIDI_EVENT_DATA_LENGTH]; /* event data as provided through MIDI */

	/* real time position */
	double pos_sec;
	unsigned long pos_msec;

	char text[RTK_MAX_MIDI_EVENT_DATA_LENGTH];
	char data[RTK_MAX_MIDI_EVENT_DATA_LENGTH];
	unsigned long data_i[RTK_MAX_MIDI_EVENT_DATA]; /* data such as note number or velocity, depends on event type */

	int flags;

} RTK_MIDI_EVENT;

/* a parsed MIDI track */
typedef struct
{

	char name[RTK_MAX_MIDI_TRACK_NAME_LENGTH];

	RTK_MIDI_EVENT ** event; /* dynamically allocated array of events */
	int events;

} RTK_MIDI_TRACK;

/* this is the main structure we will operate on, it will store a copy of the
 * raw MIDI data and parsed MIDI data */
typedef struct
{

	RTK_MIDI_DATA * raw_data;

	/* tracks */
	RTK_MIDI_TRACK ** track;
	int tracks;

	/* store copy of all tempo change events here */
	RTK_MIDI_EVENT ** tempo_event;
	int tempo_events;

} RTK_MIDI;

/* MIDI functions */
RTK_MIDI * rtk_load_midi(const char * fn);
int rtk_save_midi(RTK_MIDI * mp, const char * fn);
void rtk_destroy_midi(RTK_MIDI * mp);

/* utility functions */
double rtk_ppqn_to_bpm(unsigned long ppqn);
int rtk_sec_to_tick(RTK_MIDI * mp, float sec);

/* editing functions */
int rtk_add_midi_event_sec(RTK_MIDI * mp, int track, float sec, int type, int meta, int channel, char * data, int data_size);
void rtk_delete_midi_event(RTK_MIDI * mp, int track, int event);

#endif
