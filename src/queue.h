#ifndef OMO_QUEUE_H
#define OMO_QUEUE_H

#include "t3f/t3f.h"
#include "library.h"

typedef struct
{

	char artist[256];
	char album[256];
	char title[256];
	char track[8];
	double length;

} OMO_QUEUE_TAGS;

typedef struct
{

	char * file;
	char * sub_file;
	char * track;

	OMO_QUEUE_TAGS tags;
	bool tags_retrieved;
	bool skip_scan; // flag to skip scanning if we got song from library

} OMO_QUEUE_ENTRY;

typedef struct
{

	OMO_QUEUE_ENTRY ** entry;
	int entry_size;
	int entry_count;

	ALLEGRO_THREAD * thread; // tags scanner thread
	bool thread_done;

} OMO_QUEUE;

OMO_QUEUE * omo_create_queue(int files);
bool omo_save_queue(OMO_QUEUE * qp, const char * fn);
OMO_QUEUE * omo_load_queue(const char * fn);
void omo_destroy_queue(OMO_QUEUE * qp);
bool omo_resize_queue(OMO_QUEUE ** qp, int files);
bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn, const char * subfn, const char * track, bool skip_scan);
void omo_delete_queue_item(OMO_QUEUE * qp, int index);
bool omo_copy_queue_item(OMO_QUEUE_ENTRY * ep, OMO_QUEUE * qp);

#endif
