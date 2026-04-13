#ifndef OCD_ANIMATION_H
#define OCD_ANIMATION_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include "atlas.h"
#include "bitmap.h"
#include "async.h"

#define T3F_ANIMATION_MAX_BITMAPS  256
#define T3F_ANIMATION_REVISION       2 // change to 1 after we fix image loading to use memfiles

#define T3F_ANIMATION_FLAG_ONCE             1
#define T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS 2

typedef struct
{

	int bitmap;
	float x;
	float y;
	float z;
	float width;
	float height;
	float angle;
	int ticks;
	int flags;

} T3F_ANIMATION_FRAME;

typedef struct
{

    T3F_BITMAP * bitmap[T3F_ANIMATION_MAX_BITMAPS];
    int count;

} T3F_ANIMATION_BITMAPS;

typedef struct
{
	T3F_ANIMATION_BITMAPS * bitmaps;

	T3F_ANIMATION_FRAME ** frame;
	int frames;

	int * frame_list;
	int frame_list_total;
	int loop_point;

	int flags;

} T3F_ANIMATION_DATA;

typedef struct
{
	/* loading helpers */
	T3F_OBJECT_LOADER * object_loader;
	float target_width;
	float target_height;

	T3F_ANIMATION_DATA * data;

} T3F_ANIMATION;

bool _t3f_init_animation_system(void);
void _t3f_exit_animation_system(void);

/* memory management */
T3F_ANIMATION * t3f_create_animation(void);
T3F_ANIMATION * t3f_clone_animation(T3F_ANIMATION * ap);
void t3f_destroy_animation(T3F_ANIMATION * ap);
T3F_ANIMATION * t3f_load_animation_f(ALLEGRO_FILE * fp, const char * fn, int flags);
T3F_ANIMATION * t3f_load_animation(const char * fn, int flags, bool threaded);
int t3f_save_animation_f(T3F_ANIMATION * ap, ALLEGRO_FILE * fp);
int t3f_save_animation(T3F_ANIMATION * ap, const char * fn);

bool t3f_update_animation(T3F_ANIMATION * ap);

/* utilities */
int t3f_animation_add_bitmap(T3F_ANIMATION * ap, T3F_BITMAP * bp);
int t3f_animation_delete_bitmap(T3F_ANIMATION * ap, int bitmap);
int t3f_animation_add_frame(T3F_ANIMATION * ap, int bitmap, float x, float y, float z, float w, float h, float angle, int ticks, int flags);
int t3f_animation_delete_frame(T3F_ANIMATION * ap, int frame);
int t3f_animation_build_frame_list(T3F_ANIMATION * ap);
bool t3f_add_animation_to_atlas(T3F_ATLAS * sap, T3F_ANIMATION * ap, int flags);

/* in-game */
T3F_BITMAP * t3f_animation_get_bitmap(T3F_ANIMATION * ap, int tick);
T3F_ANIMATION_FRAME * t3f_animation_get_frame(T3F_ANIMATION * ap, int tick);
void t3f_draw_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, int flags);
void t3f_draw_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, float scale, int flags);
void t3f_draw_rotated_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, int flags);
void t3f_draw_rotated_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, float scale, int flags);
void t3f_draw_scaled_rotated_animation_region(T3F_ANIMATION * ap, float sx, float sy, float sw, float sh, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float scale, float angle, int flags);

#ifdef __cplusplus
   }
#endif

#endif
