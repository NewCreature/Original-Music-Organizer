#ifndef T3F_RESOURCE_H
#define T3F_RESOURCE_H

#include <allegro5/allegro.h>

#define T3F_MAX_RESOURCES        1024

#define T3F_RESOURCE_MAX_PATH     256

typedef struct
{

	bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
	char filename[T3F_RESOURCE_MAX_PATH];
	unsigned long offset;
	int option, flags;
	const ALLEGRO_FILE_INTERFACE * fi;
	void ** ptr;

} T3F_RESOURCE;

/* resource handlers */
bool t3f_bitmap_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_font_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_bitmap_font_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_t3f_font_gen_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);
bool t3f_t3f_font_load_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy);

void * t3f_load_resource(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), const char * filename, int option, int flags, unsigned long offset);
void * t3f_load_resource_f(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), ALLEGRO_FILE * fp, const char * filename, int option, int flags);
int t3f_unload_resource(void * ptr);
bool t3f_destroy_resource(void * ptr);
void t3f_unload_resources(void);
void t3f_reload_resources(void);
void * t3f_clone_resource(void ** dest, void * ptr);
void t3f_show_resources(void);

#endif
