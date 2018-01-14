#include "t3f/t3f.h"
#include "database.h"

OMO_DATABASE * omo_create_database(const char * fn)
{
	OMO_DATABASE * dp;

	dp = malloc(sizeof(OMO_DATABASE));
	if(dp)
	{
		memset(dp, 0, sizeof(OMO_DATABASE));
		dp->filename = malloc(strlen(fn) + 1);
		if(!dp->filename)
		{
			goto fail;
		}
		strcpy(dp->filename, fn);

		dp->config = al_load_config_file(dp->filename);
		if(!dp->config)
		{
			dp->empty = true;
			dp->config = al_create_config();
			if(!dp->config)
			{
				goto fail;
			}
		}

		dp->mutex = al_create_mutex();
		if(!dp->mutex)
		{
			goto fail;
		}
		return dp;
	}

	fail:
	{
		if(dp)
		{
			if(dp->mutex)
			{
				al_destroy_mutex(dp->mutex);
			}
			if(dp->config)
			{
				al_destroy_config(dp->config);
			}
			if(dp->filename)
			{
				free(dp->filename);
			}
			free(dp);
		}
	}
	return NULL;
}

void omo_destroy_database(OMO_DATABASE * dp)
{
	al_destroy_mutex(dp->mutex);
	al_destroy_config(dp->config);
	free(dp->filename);
	free(dp);
}

bool omo_save_database(OMO_DATABASE * dp)
{
	return al_save_config_file(dp->filename, dp->config);
}

void omo_set_database_value(OMO_DATABASE * dp, const char * section, const char * key, const char * val)
{
	al_lock_mutex(dp->mutex);
	al_set_config_value(dp->config, section, key, val);
	al_unlock_mutex(dp->mutex);
}

const char * omo_get_database_value(OMO_DATABASE * dp, const char * section, const char * key)
{
	const char * retval;

	al_lock_mutex(dp->mutex);
	retval = al_get_config_value(dp->config, section, key);
	al_unlock_mutex(dp->mutex);

	return retval;
}

void omo_remove_database_key(OMO_DATABASE * dp, const char * section, const char * key)
{
	al_lock_mutex(dp->mutex);
	al_remove_config_key(dp->config, section, key);
	al_unlock_mutex(dp->mutex);
}
