#ifndef OMO_LIBRARY_HELPERS_H
#define OMO_LIBRARY_HELPERS_H

void omo_cancel_library_setup(APP_INSTANCE * app);
void omo_setup_library(APP_INSTANCE * app, const char * file_database_fn, const char * entry_database_fn, ALLEGRO_CONFIG * config);
const char * omo_get_library_file_id(OMO_LIBRARY * lp, const char * fn, const char * subfn, const char * track);

#endif
