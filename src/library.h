#ifndef OMO_LIBRARY_H
#define OMO_LIBRARY_H

typedef struct
{

    char * filename;
    char * id;

} OMO_LIBRARY_ENTRY;

typedef struct
{

    OMO_LIBRARY_ENTRY ** entry;
    unsigned long entry_size;
    unsigned long entry_count;

    /* two databases, one for storing file paths and one for storing song
       information that the file database references by id */
    char * file_database_fn;
    ALLEGRO_CONFIG * file_database;
    char * entry_database_fn;
    ALLEGRO_CONFIG * entry_database;

} OMO_LIBRARY;

OMO_LIBRARY * omo_create_library(const char * file_db_fn, const char * entry_db_fn, int total_files);
void omo_destroy_library(OMO_LIBRARY * lp);
bool omo_save_library(OMO_LIBRARY * lp);
bool omo_add_file_to_library(OMO_LIBRARY * lp, const char * fn, const char * subfn);

#endif
