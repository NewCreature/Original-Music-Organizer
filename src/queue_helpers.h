#ifndef OMO_QUEUE_HELPERS_H
#define OMO_QUEUE_HELPERS_H

#include "t3gui/t3gui.h"
#include "queue.h"
#include "library.h"

bool omo_get_queue_entry_tags(OMO_QUEUE * qp, int i, OMO_LIBRARY * lp);
void omo_get_queue_tags(OMO_QUEUE * qp, OMO_LIBRARY * lp, void * data);
void omo_sort_queue(OMO_QUEUE * qp, OMO_LIBRARY * lp, int mode, int start_index, int count);
bool omo_export_queue_to_playlist(OMO_QUEUE * qp, const char * fn);
const char * omo_get_queue_entry_id(OMO_QUEUE * qp, int entry, OMO_LIBRARY * lp);
bool omo_queue_item_selected(T3GUI_ELEMENT * d, int entry);
int omo_queue_items_selected(T3GUI_ELEMENT * d, int max);

#endif
