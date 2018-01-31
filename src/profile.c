#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "constants.h"

const char * omo_get_profile_section(ALLEGRO_CONFIG * cp, char * buffer)
{
	const char * val;

	val = al_get_config_value(cp, "Settings", "Profile");
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

bool omo_setup_profile(const char * name)
{
	ALLEGRO_PATH * path;

	path = al_clone_path(t3f_data_path);
	if(path)
	{
		al_append_path_component(path, "Profiles");
		al_append_path_component(path, name);
		if(al_make_directory(al_path_cstr(path, '/')))
		{
			al_destroy_path(path);
			return true;
		}
		al_destroy_path(path);
	}

	return false;
}

void omo_remove_profile(const char * name)
{
	char buffer[1024];
	char profile_path[1024];
	char buf[64];
	int i, j, c;
	const char * val;

	val = al_get_config_value(t3f_config, "Settings", "profiles");
	if(val)
	{
		c = atoi(val);
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "profile_%d", i);
			val = al_get_config_value(t3f_config, "Settings", buf);
			if(val)
			{
				if(!strcmp(val, name))
				{
					for(j = i + 1; j < c; j++)
					{
						sprintf(buf, "profile_%d", j);
						val = al_get_config_value(t3f_config, "Settings", buf);
						sprintf(buf, "profile_%d", i);
						al_set_config_value(t3f_config, "Settings", buf, val ? val : "");
						i++;
					}
					sprintf(buf, "%d", c - 1 < 0 ? 0 : c - 1);
					al_set_config_value(t3f_config, "Settings", "profiles", buf);
					break;
				}
			}
		}
	}
	sprintf(profile_path, "%s/%s", t3f_get_filename(t3f_data_path, "profiles", buffer, 1024), name);
	t3f_remove_directory(profile_path);
}

const char * omo_get_profile_path(const char * name, const char * fn, char * buffer, int buffer_size)
{
	ALLEGRO_PATH * path;
	const char * ret = NULL;
	char tail[256];

	sprintf(tail, "Profiles/%s", name);
	path = al_clone_path(t3f_data_path);
	if(path)
	{
		al_append_path_component(path, "Profiles");
		al_append_path_component(path, name);
		al_set_path_filename(path, fn);
		if(strlen(al_path_cstr(path, '/')) < buffer_size)
		{
			strcpy(buffer, al_path_cstr(path, '/'));
			ret = buffer;
		}
		al_destroy_path(path);
	}

	return ret;
}

const char * omo_get_profile(void)
{
	const char * val;

	val = al_get_config_value(t3f_config, "Settings", "profile");
	if(!val)
	{
		val = omo_default_profile;
	}
	return val;
}
