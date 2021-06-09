#ifndef T3F_FILE_UTILS_H
#define T3F_FILE_UTILS_H

bool t3f_scan_files(const char * path, bool (*process_file)(const char * fn, bool isfolder, void * data), bool subdir, void * data);
bool t3f_remove_directory(const char * path);
const char * t3f_get_path_filename(const char * path);
const char * t3f_get_path_extension(const char * path);

#endif
