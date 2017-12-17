#ifndef OMO_CLOUD_H
#define OMO_CLOUD_H

#include "library.h"

bool omo_submit_tags(OMO_LIBRARY * lp, const char * id, const char * url, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path);

#endif
