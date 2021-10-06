#ifndef OMO_FILE_HELPERS_H
#define OMO_FILE_HELPERS_H

#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "library.h"
#include "queue.h"

typedef struct
{

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry;
	OMO_LIBRARY * library;
	OMO_QUEUE * queue;
	unsigned long file_count;
	bool cancel_scan;
	bool scan_done;
	ALLEGRO_PATH * temp_path;
	const char * filter;
	void * user_data;

} OMO_FILE_HELPER_DATA;

time_t omo_get_folder_mtime(const char * path);
void omo_setup_file_helper_data(OMO_FILE_HELPER_DATA * fhdp, OMO_ARCHIVE_HANDLER_REGISTRY * ahrp, OMO_CODEC_HANDLER_REGISTRY * chrp, const char * filter, OMO_LIBRARY * lp, OMO_QUEUE * qp, ALLEGRO_PATH * temp_path, void * user_data);
bool omo_count_file(const char * fn, bool isfolder, void * data);
bool omo_add_file(const char * fn, bool isfolder, void * data);

bool omo_queue_file(const char * fn, bool isfolder, void * data);

void omo_clear_library_cache(void);

bool omo_get_full_filename(const char * fn, const char * subfn, const char * track, char * buffer, int buffer_size);

#endif
