#ifndef OMO_FILE_CHOOSER_H
#define OMO_FILE_CHOOSER_H

bool omo_start_file_chooser(void * data, const char * title, const char * types, int mode, bool threaded);
void omo_file_chooser_logic(void * data);

#endif
