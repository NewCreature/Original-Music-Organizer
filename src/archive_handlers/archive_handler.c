#include "archive_handler.h"

bool omo_archive_handler_add_type(OMO_ARCHIVE_HANDLER * ap, const char * type)
{
	if(ap->types < OMO_ARCHIVE_HANDLER_MAX_TYPES)
	{
		strcpy(ap->type[ap->types], type);
		ap->types++;
		return true;
	}
	return false;
}
