#ifndef T3F_BITMAP_H
#define T3F_BITMAP_H

#include "async.h"

#define T3F_BITMAP_FLAG_PAD_LEFT   (1 << 0)
#define T3F_BITMAP_FLAG_PAD_RIGHT  (1 << 1)
#define T3F_BITMAP_FLAG_PAD_TOP    (1 << 2)
#define T3F_BITMAP_FLAG_PAD_BOTTOM (1 << 3)
#define T3F_BITMAP_FLAG_PADDED (T3F_BITMAP_FLAG_PAD_LEFT | T3F_BITMAP_FLAG_PAD_RIGHT | T3F_BITMAP_FLAG_PAD_TOP | T3F_BITMAP_FLAG_PAD_BOTTOM)
#define T3F_BITMAP_FLAG_DIRECT_LOAD (1 << 4)

#define T3F_DRAW_V_FLIP       ALLEGRO_FLIP_VERTICAL
#define T3F_DRAW_H_FLIP       ALLEGRO_FLIP_HORIZONTAL
#define T3F_DRAW_INTEGER_SNAP (1 << 2)

typedef struct
{

  T3F_OBJECT_LOADER * object_loader;
  ALLEGRO_BITMAP * bitmap;
  ALLEGRO_BITMAP * loading_bitmap;
  int flags;

  /* how big the bitmap should appear on screen */
  float target_width;
  float target_height;
  float target_scale_x;
  float target_scale_y;

  /* cache data for optimization */
  float pad_left;
  float pad_top;
  float pad_right;
  float pad_bottom;
  float adjust_left;
  float adjust_right;
  float adjust_top;
  float adjust_bottom;

} T3F_BITMAP;

bool _t3f_init_bitmap_system(void);
void _t3f_exit_bitmap_system(void);

/* tools for ALLEGRO_BITMAPs */
ALLEGRO_BITMAP * _t3f_load_allegro_bitmap_padded(const char * fn, int flags);
bool t3f_resize_bitmap(ALLEGRO_BITMAP ** bp, int w, int h, bool hq, int flags);
bool t3f_squeeze_bitmap(ALLEGRO_BITMAP ** bp, int * ow, int * oh);
ALLEGRO_BITMAP * t3f_load_allegro_bitmap_f(ALLEGRO_FILE * fp, int flags);
bool t3f_save_allegro_bitmap_f(ALLEGRO_FILE * fp, ALLEGRO_BITMAP * bp);

T3F_BITMAP * t3f_create_bitmap(int w, int h, float target_width, float target_height, int flags);
T3F_BITMAP * t3f_load_bitmap_f(ALLEGRO_FILE * fp, const char * fm, int flags);
T3F_BITMAP * t3f_load_bitmap(const char * fn, int flags, bool threaded);
T3F_BITMAP * t3f_clone_bitmap(T3F_BITMAP * bp);
T3F_BITMAP * t3f_encapsulate_bitmap(ALLEGRO_BITMAP * bp);
void t3f_reset_bitmap_target_size(T3F_BITMAP * bp);
void t3f_destroy_bitmap(T3F_BITMAP * bp);
int t3f_get_bitmap_width(T3F_BITMAP * bp);
int t3f_get_bitmap_height(T3F_BITMAP * bp);

bool t3f_update_bitmap(T3F_BITMAP * bp);

void t3f_draw_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, int flags);
void t3f_draw_bitmap_region(T3F_BITMAP * bp, ALLEGRO_COLOR color, float sx, float sy, float sw, float sh, float dx, float dy, float dz, int flags);
void t3f_draw_scaled_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, float w, float h, int flags);
void t3f_draw_rotated_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, int flags);
void t3f_draw_scaled_rotated_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, float scale_x, float scale_y, int flags);
void t3f_draw_scaled_rotated_bitmap_region(T3F_BITMAP * bp, float sx, float sy, float sw, float sh, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float scale, float angle, int flags);

#endif
