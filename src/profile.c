#include "t3f/t3f.h"

const char * omo_get_profile_section(char * buffer)
{
	const char * val;

	val = al_get_config_value(t3f_config, "Settings", "Profile");
	if(val)
	{
		sprintf(buffer, "Profile %s", val);
	}
	else
	{
		strcpy(buffer, "Profile Default");
	}
	return buffer;
}
