#ifndef OMO_FILE_CHOOSER_H
#define OMO_FILE_CHOOSER_H

#define OMO_FILE_CHOOSER_PLAY_FILES            0
#define OMO_FILE_CHOOSER_QUEUE_FILES           1
#define OMO_FILE_CHOOSER_PLAY_FOLDER           2
#define OMO_FILE_CHOOSER_QUEUE_FOLDER          3
#define OMO_FILE_CHOOSER_ADD_LIBRARY_FOLDER    4
#define OMO_FILE_CHOOSER_LOAD_THEME            5
#define OMO_FILE_CHOOSER_EXPORT_PLAYLIST       6
#define OMO_FILE_CHOOSER_IMPORT_FILE_DATABASE  7
#define OMO_FILE_CHOOSER_IMPORT_ENTRY_DATABASE 8

bool omo_start_file_chooser(void * data, const char * initial, const char * title, const char * types, int mode, bool threaded);
void omo_file_chooser_logic(void * data);

#endif
