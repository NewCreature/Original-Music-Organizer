#ifndef OMO_PROFILE_H
#define OMO_PROFILE_H

#include "t3f/t3f.h"

/* basic profile functions */
int omo_get_profile_count(void);
const char * omo_get_profile(int index);
bool omo_add_profile(const char * name);
void omo_delete_profile(int index);

/* high level profile functions */
const char * omo_get_profile_section(ALLEGRO_CONFIG * cp, const char * name, char * buffer);
bool omo_setup_profile(const char * name);
void omo_remove_profile(const char * name);
const char * omo_get_profile_path(const char * name, const char * fn, char * buffer, int buffer_size);
int omo_get_current_profile(void);
void omo_set_current_profile(int index);

#endif
