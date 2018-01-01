#ifndef OMO_LIBRARY_H
#define OMO_LIBRARY_H

#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "database.h"

typedef struct
{

	char * filename;
	char * sub_filename;
	char * track;
	const char * id;

} OMO_LIBRARY_ENTRY;

typedef struct
{

	/* two databases, one for storing file paths and one for storing song
	   information that the file database references by id */
	OMO_DATABASE * file_database;
	OMO_DATABASE * entry_database;

	OMO_LIBRARY_ENTRY ** entry;
	unsigned long entry_size;
	unsigned long entry_count;

	char ** artist_entry;
	int artist_entry_size;
	int artist_entry_count;
	char last_artist_name[256];

	char ** album_entry;
	int album_entry_size;
	int album_entry_count;
	char last_album_name[256];

	/* current list of songs for song list pane */
	unsigned long * song_entry;
	int song_entry_size;
	int song_entry_count;

	/* store the newest modified time from library folders */
	time_t modified_time;

	/* flag to tell library to ignore caches (set when we change library folder settings) */
	bool modified;
	bool loaded;

} OMO_LIBRARY;

OMO_LIBRARY * omo_create_library(const char * file_db_fn, const char * entry_db_fn);
bool omo_allocate_library(OMO_LIBRARY * lp, int total_files);
void omo_destroy_library(OMO_LIBRARY * lp);
bool omo_save_library(OMO_LIBRARY * lp);
int omo_add_file_to_library(OMO_LIBRARY * lp, const char * fn, const char * subfn, const char * track, OMO_ARCHIVE_HANDLER_REGISTRY * rp, OMO_CODEC_HANDLER_REGISTRY * crp, ALLEGRO_PATH * temp_path);
bool omo_add_artist_to_library(OMO_LIBRARY * lp, const char * name);
bool omo_add_album_to_library(OMO_LIBRARY * lp, const char * name);
bool omo_get_library_album_list(OMO_LIBRARY * lp, const char * artist);
bool omo_get_library_song_list(OMO_LIBRARY * lp, const char * artist, const char * album);
void omo_start_library_sort(void);
void omo_cancel_library_sort(void);

#endif
