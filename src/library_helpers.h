#ifndef OMO_LIBRARY_HELPERS_H
#define OMO_LIBRARY_HELPERS_H

#include "instance.h"

void omo_cancel_library_setup(APP_INSTANCE * app);
bool omo_build_library_artists_list(APP_INSTANCE * app, OMO_LIBRARY * lp);
void omo_setup_library(APP_INSTANCE * app, const char * file_database_fn, const char * entry_database_fn, ALLEGRO_CONFIG * config);
void omo_setup_library_lists(APP_INSTANCE * app);
const char * omo_get_library_file_id(OMO_LIBRARY * lp, const char * fn, const char * subfn, const char * track);
const char * omo_get_library_file_base_id(OMO_LIBRARY * lp, const char * fn, char * buffer);
int omo_get_library_entry(OMO_LIBRARY * lp, const char * id);
bool omo_split_track(OMO_LIBRARY * lp, const char * basefn, char * split_string);

#endif
