#ifndef OMO_PROFILE_H
#define OMO_PROFILE_H

#include "t3f/t3f.h"

const char * omo_get_profile_section(ALLEGRO_CONFIG * cp, char * buffer);
bool omo_setup_profile(const char * name);
void omo_remove_profile(const char * name);

#endif
