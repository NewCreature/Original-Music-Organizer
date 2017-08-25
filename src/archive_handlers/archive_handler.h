#ifndef OMO_ARCHIVE_HANDLER_H
#define OMO_ARCHIVE_HANDLER_H

#include "t3f/t3f.h"

#define OMO_ARCHIVE_HANDLER_MAX_TYPES     128
#define OMO_ARCHIVE_HANDLER_MAX_TYPE_SIZE  16

typedef struct
{

    char type[OMO_ARCHIVE_HANDLER_MAX_TYPES][OMO_ARCHIVE_HANDLER_MAX_TYPE_SIZE];
    int types;

    /* archive handling functions */
    int (*count_files)(const char * fn);
    const char * (*get_file)(const char * fn, int index, char * buffer);
    const char * (*extract_file)(const char * fn, int index, char * buffer);
//    const char * (*get_temp_file)(const char * fn, )

    /* instance data */
    void * data;

} OMO_ARCHIVE_HANDLER;

bool omo_archive_handler_add_type(OMO_ARCHIVE_HANDLER * ap, const char * type);

#endif
