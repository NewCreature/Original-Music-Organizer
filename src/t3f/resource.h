#ifndef T3F_RESOURCE_H
#define T3F_RESOURCE_H

#include <allegro5/allegro.h>

#define T3F_MAX_RESOURCES        1024
#define T3F_MAX_RESOURCE_HANDLERS  16

#define T3F_RESOURCE_TYPE_BITMAP        0
#define T3F_RESOURCE_TYPE_FONT          1
#define T3F_RESOURCE_TYPE_BITMAP_FONT   2
#define T3F_RESOURCE_TYPE_T3F_FONT_GEN  3
#define T3F_RESOURCE_TYPE_T3F_FONT_LOAD 4
#define T3F_RESOURCE_TYPE_USER          8

#define T3F_RESOURCE_MAX_PATH     256

typedef struct
{

	void * (*proc)(ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset);
	void (*destroy_proc)(void * ptr);
	int flags;

} T3F_RESOURCE_HANDLER;

typedef struct
{

	int type;
	char filename[T3F_RESOURCE_MAX_PATH];
	unsigned long offset;
	int option, flags;
	const ALLEGRO_FILE_INTERFACE * fi;
	void ** ptr;

} T3F_RESOURCE;

void t3f_init_resource_handlers(void);
void t3f_register_resource_handler(int type, void * (*proc)(ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset), void (*destroy_proc)(void * ptr));
void * t3f_load_resource(void ** ptr, int type, const char * filename, int option, int flags, unsigned long offset);
void * t3f_load_resource_f(void ** ptr, int type, ALLEGRO_FILE * fp, const char * filename, int option, int flags);
int t3f_unload_resource(void * ptr);
bool t3f_destroy_resource(void * ptr);
void t3f_unload_resources(void);
void t3f_reload_resources(void);
void * t3f_clone_resource(void ** dest, void * ptr);
void t3f_show_resources(void);

#endif
