#ifndef OMO_QUEUE_HELPERS_H
#define OMO_QUEUE_HELPERS_H

#include "queue.h"
#include "library.h"

bool omo_get_queue_entry_tags(OMO_QUEUE * qp, int i, OMO_LIBRARY * lp);
void omo_get_queue_tags(OMO_QUEUE * qp, OMO_LIBRARY * lp, void * data);
void omo_sort_queue(OMO_QUEUE * qp, OMO_LIBRARY * lp, int mode, int start_index, int count);
bool omo_export_queue_to_playlist(OMO_QUEUE * qp, const char * fn);

#endif
