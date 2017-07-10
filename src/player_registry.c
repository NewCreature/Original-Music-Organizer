#include "player_registry.h"
#include "player.h"

bool omo_register_player(OMO_PLAYER_REGISTRY * rp, OMO_PLAYER * pp)
{
    if(rp->players < OMO_MAX_REGISTERED_PLAYERS)
    {
        memcpy(&rp->player[rp->players], pp, sizeof(OMO_PLAYER));
        rp->players++;
        return true;
    }
    return false;
}
