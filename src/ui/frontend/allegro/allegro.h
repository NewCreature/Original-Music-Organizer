#ifndef _OMO_FRONTEND_ALLEGRO_H
#define _OMO_FRONTEND_ALLEGRO_H

#include "../frontend.h"

#define OMO_ALLEGRO_UI_FLAG_LIBRARY_VIEW (1 << 0)

OMO_FRONTEND * omo_get_allegro_frontend(void * app_instance, int flags);

#endif
