#ifndef OMO_INIT_H
#define OMO_INIT_H

void omo_library_setup_update_proc(const char * fn, void * data);
void omo_setup_library(APP_INSTANCE * app, const char * file_database_fn, const char * entry_database_fn, ALLEGRO_CONFIG * config);
void omo_cancel_library_setup(APP_INSTANCE * app);
void omo_set_window_constraints(APP_INSTANCE * app);
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[]);
void omo_exit(APP_INSTANCE * app);

#endif
