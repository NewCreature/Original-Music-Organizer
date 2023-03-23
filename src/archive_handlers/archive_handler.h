#ifndef OMO_ARCHIVE_HANDLER_H
#define OMO_ARCHIVE_HANDLER_H

#include "t3f/t3f.h"

#define OMO_ARCHIVE_HANDLER_MAX_TYPES     128
#define OMO_ARCHIVE_HANDLER_MAX_TYPE_SIZE  16

typedef struct
{

  char id[128];
	char type[OMO_ARCHIVE_HANDLER_MAX_TYPES][OMO_ARCHIVE_HANDLER_MAX_TYPE_SIZE];
	int types;

	/* init/exit */
	bool (*init)(void);
	void (*exit)(void);

	/* archive handling functions */
	void * (*open_archive)(const char * fn, ALLEGRO_PATH * temp_path);
	void (*close_archive)(void * data);
	int (*count_files)(void * data);
	const char * (*get_file)(void * data, int index, char * buffer);
	const char * (*extract_file)(void * data, int index, char * buffer);
//	const char * (*get_temp_file)(const char * fn, )

	/* instance data */
	void * data;

} OMO_ARCHIVE_HANDLER;

bool omo_archive_handler_add_type(OMO_ARCHIVE_HANDLER * ap, const char * type);

#endif
