#ifndef _OMO_FRONTEND_GALAXY_OF_MUSIC_H
#define _OMO_FRONTEND_GALAXY_OF_MUSIC_H

#include "../frontend.h"

#define GOM_MAX_BITMAPS 256
#define GOM_BITMAP_STAR_FAR        0
#define OMO_BITMAP_STAR_TRANSITION 1
#define OMO_BITMAP_STAR_CLOSE      2
#define GOM_BITMAP_PLANET          3
#define GOM_BITMAP_ORBIT           4

OMO_FRONTEND * omo_get_galaxy_of_music_frontend(void * app_instance, int flags);

#endif
