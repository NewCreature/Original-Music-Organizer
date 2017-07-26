#ifndef OMO_CODEC_HANDLER_REGISTRY_H
#define OMO_CODEC_HANDLER_REGISTRY_H

#include "codec_handler.h"

#define OMO_MAX_REGISTERED_PLAYERS 128

typedef struct
{

    OMO_CODEC_HANDLER player[OMO_MAX_REGISTERED_PLAYERS];
    int players;

} OMO_CODEC_HANDLER_REGISTRY;

OMO_CODEC_HANDLER_REGISTRY * omo_create_player_registry(void);
void omo_destroy_player_registry(OMO_CODEC_HANDLER_REGISTRY * rp);
bool omo_register_player(OMO_CODEC_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER * pp);
OMO_CODEC_HANDLER * omo_get_player(OMO_CODEC_HANDLER_REGISTRY * rp, const char * fn);

#endif
