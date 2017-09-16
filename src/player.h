#ifndef OMO_PLAYER_H
#define OMO_PLAYER_H

#include "queue.h"
#include "codec_handlers/codec_handler.h"

#define OMO_PLAYER_STATE_STOPPED 0
#define OMO_PLAYER_STATE_PLAYING 1
#define OMO_PLAYER_STATE_PAUSED  2

typedef struct
{

    OMO_QUEUE * queue;
    int queue_pos;
    char extracted_filename[1024];
    OMO_CODEC_HANDLER * codec_handler;
    void * codec_data;
    int state;

} OMO_PLAYER;

OMO_PLAYER * omo_create_player(void);
void omo_destroy_player(OMO_PLAYER * pp);

bool omo_start_player(OMO_PLAYER * pp);
void omo_stop_player(OMO_PLAYER * pp);
void omo_pause_player(OMO_PLAYER * pp);
void omo_resume_player(OMO_PLAYER * pp);
bool omo_play_previous_song(OMO_PLAYER * pp);
bool omo_play_next_song(OMO_PLAYER * pp);

void omo_player_logic(OMO_PLAYER * pp, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path);

#endif
