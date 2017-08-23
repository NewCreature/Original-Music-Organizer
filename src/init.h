#ifndef OMO_INIT_H
#define OMO_INIT_H

void omo_library_setup_update_proc(const char * fn, void * data);
bool omo_setup_library(APP_INSTANCE * app, void (*update_proc)(const char * fn, void * data));
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[]);
void omo_exit(APP_INSTANCE * app);

#endif
