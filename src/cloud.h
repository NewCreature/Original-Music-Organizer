#ifndef OMO_CLOUD_H
#define OMO_CLOUD_H

#include "t3net/t3net.h"
#include "library.h"
#include "instance.h"

bool omo_init_cloud(void);
void omo_exit_cloud(void);
T3NET_DATA * omo_get_remote_data(const char * url, const T3NET_ARGUMENTS * arguments);
bool omo_get_tagger_key(const char * name);
bool omo_submit_track_tags(OMO_LIBRARY * lp, const char * id, const char * url, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path);
bool omo_retrieve_track_tags(OMO_LIBRARY * lp, const char * id, const char * url);
bool omo_submit_library_tags(APP_INSTANCE * app, const char * url);
bool omo_retrieve_library_tags(APP_INSTANCE * app, const char * url);

#endif
