#ifndef OMO_PLAYER_REGISTRY_H
#define OMO_PLAYER_REGISTRY_H

#include "player.h"

#define OMO_MAX_REGISTERED_PLAYERS 128

typedef struct
{

    OMO_PLAYER player[OMO_MAX_REGISTERED_PLAYERS];
    int players;

} OMO_PLAYER_REGISTRY;

OMO_PLAYER_REGISTRY * omo_create_player_registry(void);
void omo_destroy_player_registry(OMO_PLAYER_REGISTRY * rp);
bool omo_register_player(OMO_PLAYER_REGISTRY * rp, OMO_PLAYER * pp);
OMO_PLAYER * omo_get_player(OMO_PLAYER_REGISTRY * rp, const char * fn);

#endif
