#include "codec_handler.h"

bool omo_codec_handler_add_type(OMO_CODEC_HANDLER * pp, const char * type)
{
	if(pp->types < OMO_CODEC_HANDLER_MAX_TYPES)
	{
		strcpy(pp->type[pp->types], type);
		pp->types++;
		return true;
	}
	return false;
}
