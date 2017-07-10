#ifndef T3F_ATLAS_H
#define T3F_ATLAS_H

#define T3F_ATLAS_TILE     0
#define T3F_ATLAS_SPRITE   1
#define T3F_ATLAS_MAX_BITMAPS 1024
#define T3F_MAX_ATLASES   32

typedef struct
{

	ALLEGRO_BITMAP * page;
	int x, y;
	int width, height;
	int line_height;
	ALLEGRO_BITMAP ** bitmap[T3F_ATLAS_MAX_BITMAPS];
	int bitmap_type[T3F_ATLAS_MAX_BITMAPS];
	int bitmaps;

} T3F_ATLAS;

T3F_ATLAS * t3f_create_atlas(int w, int h);
void t3f_destroy_atlas(T3F_ATLAS * ap);
bool t3f_add_bitmap_to_atlas(T3F_ATLAS * ap, ALLEGRO_BITMAP ** bp, int type);
void t3f_unload_atlases(void);
bool t3f_rebuild_atlases(void);

#endif
