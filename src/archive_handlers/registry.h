#ifndef OMO_ARCHIVE_HANDLER_REGISTRY_H
#define OMO_ARCHIVE_HANDLER_REGISTRY_H

#include "archive_handler.h"

#define OMO_MAX_REGISTERED_ARCHIVE_HANDLERS 128

typedef struct
{

	OMO_ARCHIVE_HANDLER archive_handler[OMO_MAX_REGISTERED_ARCHIVE_HANDLERS];
	int archive_handlers;

} OMO_ARCHIVE_HANDLER_REGISTRY;

OMO_ARCHIVE_HANDLER_REGISTRY * omo_create_archive_handler_registry(void);
void omo_destroy_archive_handler_registry(OMO_ARCHIVE_HANDLER_REGISTRY * rp);
bool omo_register_archive_handler(OMO_ARCHIVE_HANDLER_REGISTRY * rp, OMO_ARCHIVE_HANDLER * ap);
OMO_ARCHIVE_HANDLER * omo_get_archive_handler(OMO_ARCHIVE_HANDLER_REGISTRY * rp, const char * fn);

#endif
