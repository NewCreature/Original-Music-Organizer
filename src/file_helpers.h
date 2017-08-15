#ifndef OMO_FILE_HELPERS_H
#define OMO_FILE_HELPERS_H

void omo_reset_file_count(void);
bool omo_count_file(const char * fn, void * data);
unsigned long omo_get_file_count(void);
bool omo_add_file(const char * fn, void * data);

bool omo_queue_file(const char * fn, void * data);

#endif
