#ifndef OMO_QUEUE_H
#define OMO_QUEUE_H

#include "t3f/t3f.h"

typedef struct
{

    char * file;
    char * sub_file;

} OMO_QUEUE_ENTRY;

typedef struct
{

    OMO_QUEUE_ENTRY ** entry;
    int entry_size;
    int entry_count;

} OMO_QUEUE;

OMO_QUEUE * omo_create_queue(int files);
void omo_destroy_queue(OMO_QUEUE * qp);
bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn, const char * subfn);
void omo_delete_queue_item(OMO_QUEUE * qp, int index);

#endif
