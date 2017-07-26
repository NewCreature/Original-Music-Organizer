#include "registry.h"
#include "codec_handler.h"

OMO_CODEC_HANDLER_REGISTRY * omo_create_player_registry(void)
{
    OMO_CODEC_HANDLER_REGISTRY * rp;

    rp = malloc(sizeof(OMO_CODEC_HANDLER_REGISTRY));
    if(rp)
    {
        memset(rp, 0, sizeof(OMO_CODEC_HANDLER_REGISTRY));
    }
    return rp;
}

void omo_destroy_player_registry(OMO_CODEC_HANDLER_REGISTRY * rp)
{
    free(rp);
}

bool omo_register_player(OMO_CODEC_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER * pp)
{
    if(rp->players < OMO_MAX_REGISTERED_PLAYERS)
    {
        memcpy(&rp->player[rp->players], pp, sizeof(OMO_CODEC_HANDLER));
        rp->players++;
        return true;
    }
    return false;
}

OMO_CODEC_HANDLER * omo_get_player(OMO_CODEC_HANDLER_REGISTRY * rp, const char * fn)
{
    ALLEGRO_PATH * path;
    const char * extension;
    int i, j;
    OMO_CODEC_HANDLER * player = NULL;

    path = al_create_path(fn);
    if(path)
    {
        extension = al_get_path_extension(path);
        for(i = 0; i < rp->players; i++)
        {
            for(j = 0; j < rp->player[i].types; j++)
            {
                if(!strcasecmp(extension, rp->player[i].type[j]))
                {
                    player = &rp->player[i];
                    break;
                }
            }
        }
        al_destroy_path(path);
    }
    return player;
}
