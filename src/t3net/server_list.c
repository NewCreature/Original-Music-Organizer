#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t3net.h"
#include "server_list.h"
#include "internal.h"

static char t3net_server_key[1024] = {0};

T3NET_SERVER_LIST * t3net_get_server_list(char * url, char * game, char * version)
{
	T3NET_SERVER_LIST * lp = NULL;

	lp = malloc(sizeof(T3NET_SERVER_LIST));
	if(!lp)
	{
		goto fail;
	}
	lp->entries = 0;
	strcpy(lp->url, url);
	strcpy(lp->game, game);
	strcpy(lp->version, version);
	if(!t3net_update_server_list_2(lp))
	{
		free(lp);
		return NULL;
	}
	return lp;

	fail:
	{
		if(lp)
		{
			free(lp);
		}
		return NULL;
	}
}

int t3net_update_server_list_2(T3NET_SERVER_LIST * lp)
{
	T3NET_ARGUMENTS * args = NULL;
	T3NET_DATA * data = NULL;
	const char * val;
	int i;

	if(!lp)
	{
		goto fail;
	}

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "game", lp->game))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "version", lp->version))
	{
		goto fail;
	}
	data = t3net_get_data(lp->url, args);
	if(!data)
	{
		goto fail;
	}

	for(i = 0; i < data->entries; i++)
	{
		val = t3net_get_data_entry_field(data, i, "name");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->name, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "ip");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->address, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "port");
		if(val)
		{
			lp->entry[i]->port = atoi(val);
		}
		val = t3net_get_data_entry_field(data, i, "capacity");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->capacity, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "private");
		if(val)
		{
			lp->entry[i]->private = 0;
			if(!strcmp(val, "true"))
			{
				lp->entry[i]->private = 1;
			}
		}
	}

	lp->entries = data->entries;
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);
	return 1;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return 0;
	}
}

void t3net_clear_server_list(T3NET_SERVER_LIST * lp)
{
	int i;

	for(i = 0; i < lp->entries; i++)
	{
		free(lp->entry[i]);
	}
	lp->entries = 0;
}

void t3net_destroy_server_list(T3NET_SERVER_LIST * lp)
{
	t3net_clear_server_list(lp);
	free(lp);
}

char * t3net_register_server(char * url, int port, char * game, char * version, char * name, char * password, int permanent)
{
	T3NET_DATA * data = NULL;
	T3NET_ARGUMENTS * args = NULL;
	char tport[64] = {0};
	const char * val;

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	sprintf(tport, "%d", port);
	if(!t3net_add_argument(args, "addServer", "1"))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "port", tport))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "game", game))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "version", version))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "name", name))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "password", password ? password : ""))
	{
		goto fail;
	}
	if(permanent)
	{
		if(!t3net_add_argument(args, "no_poll", "1"))
		{
			goto fail;
		}
	}

	data = t3net_get_data(url, args);
	if(!data)
	{
		goto fail;
	}

	/* see if we got a key */
	t3net_server_key[0] = 0;
	if(data->entries > 0)
	{
		val = t3net_get_data_entry_field(data, 0, "key");
		if(val)
		{
			t3net_strcpy(t3net_server_key, val, 1024);
		}
	}
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);
    return t3net_server_key;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return NULL;
	}
}

int t3net_update_server(char * url, int port, char * key, char * capacity)
{
	T3NET_DATA * data = NULL;
	T3NET_ARGUMENTS * args = NULL;
	char tport[256] = {0};

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	sprintf(tport, "%d", port);
	if(!t3net_add_argument(args, "pollServer", "1"))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "port", tport))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "key", key))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "capacity", capacity))
	{
		goto fail;
	}
	data = t3net_get_data(url, args);
	if(!data)
	{
		goto fail;
	}
	t3net_destroy_arguments(args);
    t3net_destroy_data(data);
    return 1;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return -1;
	}
}

int t3net_unregister_server(char * url, int port, char * key)
{
	T3NET_DATA * data = NULL;
	T3NET_ARGUMENTS * args = NULL;
	char tport[256] = {0};

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	sprintf(tport, "%d", port);
	if(!t3net_add_argument(args, "removeServer", "1"))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "port", tport))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "key", key))
	{
		goto fail;
	}
	data = t3net_get_data(url, args);
	if(!data)
	{
		goto fail;
	}
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);
    return 1;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return -1;
	}
}
