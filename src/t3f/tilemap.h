#ifndef T3F_TILEMAP_H
#define T3F_TILEMAP_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include "animation.h"

#define T3F_MAX_TILES         1024
#define T3F_MAX_LAYERS          32
#define T3F_TILE_MAX_DATA       16
#define T3F_MAX_TILE_SHEETS     16
#define T3F_MAX_TILESET_BITMAPS 16

#define T3F_TILE_FLAG_ANIMATED   1
#define T3F_TILE_FLAG_ONCE       2
#define T3F_TILE_FLAG_USER_DATA  4

/* layer flags */
#define T3F_TILEMAP_LAYER_STATIC      1
#define T3F_TILEMAP_LAYER_SOLID       2

#define T3F_TILEMAP_CAMERA_FLAG_NO_TRANSFORM 1

typedef struct
{

	T3F_ANIMATION * ap;
	int user_data[T3F_TILE_MAX_DATA];
	int flags;

	/* animated tiles (tiles which change to other tiles) */
	short frame_list[1024];
	short frame_list_total;

} T3F_TILE;

typedef struct
{

	T3F_ATLAS * atlas;

	T3F_TILE * tile[T3F_MAX_TILES];
	int tiles;
	int width;
	int height;
	int flags;

} T3F_TILESET;

typedef struct
{

	/* map data */
	short ** data;
	int width;
	int height;
	int bitmap;

	/* position of layer plane in 3D space */
	float x;
	float y;
	float z;

	/* scaling attributes */
	float scale;
	float speed_x;
	float speed_y;

	int flags;

} T3F_TILEMAP_LAYER;

typedef struct
{

	T3F_TILEMAP_LAYER * layer[T3F_MAX_LAYERS];
	int layers;

	int flags;

} T3F_TILEMAP;

T3F_TILE * t3f_create_tile(void);
void t3f_destroy_tile(T3F_TILE * tp);
short t3f_get_tile(T3F_TILESET * tsp, int tile, int tick);
T3F_TILESET * t3f_create_tileset(int w, int h);
void t3f_destroy_tileset(T3F_TILESET * tsp);
T3F_TILESET * t3f_load_tileset_f(ALLEGRO_FILE * fp, const char * fn);
T3F_TILESET * t3f_load_tileset(const char * fn);
int t3f_save_tileset_f(T3F_TILESET * tsp, ALLEGRO_FILE * fp);
int t3f_save_tileset(T3F_TILESET * tsp, const char * fn);
bool t3f_add_tile(T3F_TILESET * tsp, T3F_ANIMATION * ap);
bool t3f_atlas_tileset(T3F_TILESET * tsp);

T3F_TILEMAP_LAYER * t3f_create_tilemap_layer(int w, int h);
void t3f_destroy_tilemap_layer(T3F_TILEMAP_LAYER * tlp);

T3F_TILEMAP * t3f_create_tilemap(int w, int h, int layers);
void t3f_destroy_tilemap(T3F_TILEMAP * tmp);
T3F_TILEMAP * t3f_load_tilemap_f(ALLEGRO_FILE * fp);
T3F_TILEMAP * t3f_load_tilemap(const char * fn);
int t3f_save_tilemap_f(T3F_TILEMAP * tmp, ALLEGRO_FILE * fp);
int t3f_save_tilemap(T3F_TILEMAP * tmp, const char * fn);

void t3f_render_tilemap(T3F_TILEMAP * tmp, T3F_TILESET * tsp, int layer, int tick, float ox, float oy, float oz, ALLEGRO_COLOR color);

#ifdef __cplusplus
	}
#endif

#endif
