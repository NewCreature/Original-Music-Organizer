#include "registry.h"
#include "codec_handler.h"

OMO_CODEC_HANDLER_REGISTRY * omo_create_codec_handler_registry(void)
{
	OMO_CODEC_HANDLER_REGISTRY * rp;

	rp = malloc(sizeof(OMO_CODEC_HANDLER_REGISTRY));
	if(rp)
	{
		memset(rp, 0, sizeof(OMO_CODEC_HANDLER_REGISTRY));
	}
	return rp;
}

void omo_destroy_codec_handler_registry(OMO_CODEC_HANDLER_REGISTRY * rp)
{
	int i;

	for(i = 0; i < rp->codec_handlers; i++)
	{
		if(rp->codec_handler[i].exit)
		{
			rp->codec_handler[i].exit();
		}
	}
	free(rp);
}

bool omo_register_codec_handler(OMO_CODEC_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER * pp)
{
	if(rp->codec_handlers < OMO_MAX_REGISTERED_CODEC_HANDLERS)
	{
		memcpy(&rp->codec_handler[rp->codec_handlers], pp, sizeof(OMO_CODEC_HANDLER));
		rp->codec_handlers++;
		return true;
	}
	return false;
}

static bool check_filter(const char * extension, const char * filter)
{
	char * filter_copy;
	char type[256][OMO_CODEC_HANDLER_MAX_TYPE_SIZE];
	int types = 0;
	char * token;
	int i;

	if(filter)
	{
		filter_copy = malloc(strlen(filter) + 1);
		if(filter_copy)
		{
			strcpy(filter_copy, filter);
			token = strtok(filter_copy, "; ");
			while(token != NULL)
			{
				strcpy(type[types], token);
				types++;
				token = strtok(NULL, "; ");
			}
			free(filter_copy);
			for(i = 0; i < types; i++)
			{
				if(!strcasecmp(&extension[1], type[i]))
				{
					break;
				}
			}
			if(i >= types)
			{
				return false;
			}
		}
	}
	return true;
}

OMO_CODEC_HANDLER * omo_get_codec_handler(OMO_CODEC_HANDLER_REGISTRY * rp, const char * fn, const char * filter)
{
	ALLEGRO_PATH * path;
	const char * extension;
	int i, j;
	OMO_CODEC_HANDLER * codec_handler = NULL;

	/* filter data */


	path = al_create_path(fn);
	if(path)
	{
		extension = al_get_path_extension(path);

		/* use filter if one is supplied */
		if(!check_filter(extension, filter))
		{
			return NULL;
		}

		/* find codec handler */
		for(i = 0; i < rp->codec_handlers; i++)
		{
			for(j = 0; j < rp->codec_handler[i].types; j++)
			{
				if(!strcasecmp(extension, rp->codec_handler[i].type[j]))
				{
					codec_handler = &rp->codec_handler[i];
					i = rp->codec_handlers;
					break;
				}
			}
		}
		al_destroy_path(path);
	}
	return codec_handler;
}
