#ifndef OMO_CODEC_HANDLER_H
#define OMO_CODEC_HANDLER_H

#include "t3f/t3f.h"

#define OMO_CODEC_HANDLER_MAX_TYPES     128
#define OMO_CODEC_HANDLER_MAX_TYPE_SIZE  16

typedef struct
{

	char type[OMO_CODEC_HANDLER_MAX_TYPES][OMO_CODEC_HANDLER_MAX_TYPE_SIZE];
	int types;

	/* init/exit routines */
	bool (*initialize)(void);
	void (*exit)(void);

	/* file operations */
	void * (*load_file)(const char * fn, const char * subfn);
	void (*unload_file)(void * data);
	int (*get_track_count)(void * data, const char * fn);
	const char * (*get_tag)(void * data, const char * name);

	/* playback functions */
	bool (*set_loop)(void * data, double loop_start, double loop_end, double fade_time, int loop_count);
	bool (*set_volume)(void * data, float volume);
	bool (*play)(void * data);
	bool (*pause)(void * data);
	bool (*resume)(void * data);
	void (*stop)(void * data);
	bool (*seek)(void * data, double pos);
	double (*get_position)(void * data);
	double (*get_length)(void * data);
	bool (*done_playing)(void * data);
	const char * (*get_info)(void * data);

	/* instance data */
	void * data;

} OMO_CODEC_HANDLER;

bool omo_codec_handler_add_type(OMO_CODEC_HANDLER * pp, const char * type);

#endif
