#include "registry.h"
#include "archive_handler.h"

OMO_ARCHIVE_HANDLER_REGISTRY * omo_create_archive_handler_registry(void)
{
	OMO_ARCHIVE_HANDLER_REGISTRY * rp;

	rp = malloc(sizeof(OMO_ARCHIVE_HANDLER_REGISTRY));
	if(rp)
	{
		memset(rp, 0, sizeof(OMO_ARCHIVE_HANDLER_REGISTRY));
	}
	return rp;
}

void omo_destroy_archive_handler_registry(OMO_ARCHIVE_HANDLER_REGISTRY * rp)
{
	int i;

	for(i = 0; i < rp->archive_handlers; i++)
	{
		if(rp->archive_handler[i].exit)
		{
			rp->archive_handler[i].exit();
		}
	}
	free(rp);
}

bool omo_register_archive_handler(OMO_ARCHIVE_HANDLER_REGISTRY * rp, OMO_ARCHIVE_HANDLER * ap)
{
	bool ret = true;

	if(rp->archive_handlers < OMO_MAX_REGISTERED_ARCHIVE_HANDLERS)
	{
		if(ap->init)
		{
			ret = ap->init();			
		}
		if(ret)
		{
			memcpy(&rp->archive_handler[rp->archive_handlers], ap, sizeof(OMO_ARCHIVE_HANDLER));
			rp->archive_handlers++;
			return true;
		}
	}
	return false;
}

OMO_ARCHIVE_HANDLER * omo_get_archive_handler(OMO_ARCHIVE_HANDLER_REGISTRY * rp, const char * fn)
{
	ALLEGRO_PATH * path;
	const char * extension;
	int i, j;
	OMO_ARCHIVE_HANDLER * archive_handler = NULL;

	path = al_create_path(fn);
	if(path)
	{
		extension = al_get_path_extension(path);
		for(i = 0; i < rp->archive_handlers; i++)
		{
			for(j = 0; j < rp->archive_handler[i].types; j++)
			{
				if(!strcasecmp(extension, rp->archive_handler[i].type[j]))
				{
					archive_handler = &rp->archive_handler[i];
					break;
				}
			}
		}
		al_destroy_path(path);
	}
	return archive_handler;
}
