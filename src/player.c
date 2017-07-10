#include "player.h"

bool omo_player_add_type(OMO_PLAYER * pp, const char * type)
{
    if(pp->types < OMO_PLAYER_MAX_TYPES)
    {
        strcpy(pp->type[pp->types], type);
        pp->types++;
        return true;
    }
    return false;
}
