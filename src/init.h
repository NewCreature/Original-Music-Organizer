#ifndef OMO_INIT_H
#define OMO_INIT_H

void omo_set_window_constraints(APP_INSTANCE * app);
void omo_configure_codec_handlers(APP_INSTANCE * app);
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[]);
void omo_exit(APP_INSTANCE * app);

#endif
