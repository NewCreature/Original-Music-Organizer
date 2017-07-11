#ifndef OMO_PLAYER_H
#define OMO_PLAYER_H

#include "t3f/t3f.h"

#define OMO_PLAYER_MAX_TYPES     128
#define OMO_PLAYER_MAX_TYPE_SIZE  16

typedef struct
{

    char type[OMO_PLAYER_MAX_TYPES][OMO_PLAYER_MAX_TYPE_SIZE];
    int types;

    /* playback functions */
    bool (*initialize)(void);
    bool (*load_file)(const char * fn);
    bool (*play)(void);
    bool (*pause)(bool paused);
    void (*stop)(void);
    bool (*seek)(float pos);
    float (*get_position)(void);
    float (*get_length)(void);

} OMO_PLAYER;

bool omo_player_add_type(OMO_PLAYER * pp, const char * type);

#endif
