#ifndef OMO_CODEC_HANDLER_REGISTRY_H
#define OMO_CODEC_HANDLER_REGISTRY_H

#include "codec_handler.h"

#define OMO_MAX_REGISTERED_CODEC_HANDLERS 128

typedef struct
{

    OMO_CODEC_HANDLER codec_handler[OMO_MAX_REGISTERED_CODEC_HANDLERS];
    int codec_handlers;

} OMO_CODEC_HANDLER_REGISTRY;

OMO_CODEC_HANDLER_REGISTRY * omo_create_codec_handler_registry(void);
void omo_destroy_codec_handler_registry(OMO_CODEC_HANDLER_REGISTRY * rp);
bool omo_register_codec_handler(OMO_CODEC_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER * pp);
OMO_CODEC_HANDLER * omo_get_codec_handler(OMO_CODEC_HANDLER_REGISTRY * rp, const char * fn);

#endif
