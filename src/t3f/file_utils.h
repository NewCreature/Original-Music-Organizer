#ifndef T3F_FILE_UTILS_H
#define T3F_FILE_UTILS_H

bool t3f_scan_files(const char * path, bool (*process_file)(const char * fn, void * data), bool subdir, void (*update_proc)(const char * fn, void * data), void * data);

#endif
