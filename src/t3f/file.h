#ifndef T3F_FILE_H
#define T3F_FILE_H

#include "t3f.h"

size_t t3f_file_size(const char * fn);
time_t t3f_get_file_mtime(const char * fn);

float t3f_fread_float(ALLEGRO_FILE * fp);
bool t3f_fwrite_float(ALLEGRO_FILE * fp, float f);
char * t3f_load_string_f(ALLEGRO_FILE * fp);
bool t3f_save_string_f(ALLEGRO_FILE * fp, const char * sp);

#endif
