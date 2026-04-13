#ifndef T3F_RESOURCE_H
#define T3F_RESOURCE_H

#include <allegro5/allegro.h>

typedef struct
{

	bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
	char * filename;
	unsigned long offset;
	int option, flags;
	const ALLEGRO_FILE_INTERFACE * fi;
	void ** ptr;
	void * extra;

} T3F_RESOURCE;

bool _t3f_initialize_resource_manager(int max_resources);
void _t3f_uninitialize_resource_manager(void);

/* resource handlers */
bool t3f_bitmap_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_create_bitmap_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_font_t3f_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_font_allegro_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_bitmap_font_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_t3f_font_gen_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_t3f_font_load_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);

/* main API */
void * t3f_load_resource(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), const char * filename, int option, int flags, unsigned long offset);
void * t3f_load_resource_f(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), ALLEGRO_FILE * fp, const char * filename, int option, int flags);
bool t3f_destroy_resource(void * ptr);
void t3f_remap_resource(void ** original_ptr, void ** new_ptw);
void t3f_unload_resources(void);
void t3f_reload_resources(void);
void * t3f_clone_resource(void ** dest, void ** original_ptr);
void t3f_show_resources(void);

#endif
