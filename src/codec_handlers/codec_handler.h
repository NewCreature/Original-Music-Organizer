#ifndef OMO_CODEC_HANDLER_H
#define OMO_CODEC_HANDLER_H

#include "t3f/t3f.h"

#define OMO_CODEC_HANDLER_MAX_TYPES     128
#define OMO_CODEC_HANDLER_MAX_TYPE_SIZE  16

typedef struct
{

    char type[OMO_CODEC_HANDLER_MAX_TYPES][OMO_CODEC_HANDLER_MAX_TYPE_SIZE];
    int types;

    /* playback functions */
    bool (*initialize)(void);
    bool (*load_file)(const char * fn, const char * subfn);
    int (*get_track_count)(const char * fn);
    bool (*play)(void);
    bool (*pause)(bool paused);
    void (*stop)(void);
    bool (*seek)(float pos);
    float (*get_position)(void);
    float (*get_length)(void);
    bool (*done_playing)(void);

} OMO_CODEC_HANDLER;

bool omo_codec_handler_add_type(OMO_CODEC_HANDLER * pp, const char * type);

#endif
