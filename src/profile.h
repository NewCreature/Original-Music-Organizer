#ifndef OMO_PROFILE_H
#define OMO_PROFILE_H

const char * omo_get_profile_section(char * buffer);
bool omo_setup_profile(const char * name);
void omo_remove_profile(const char * name);

#endif
