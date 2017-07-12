#ifndef OMO_QUEUE_H
#define OMO_QUEUE_H

#include "t3f/t3f.h"

typedef struct
{

    char ** file;
    int file_size;
    int file_count;

} OMO_QUEUE;

OMO_QUEUE * omo_create_queue(int files);
void omo_destroy_queue(OMO_QUEUE * qp);
bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn);

#endif
