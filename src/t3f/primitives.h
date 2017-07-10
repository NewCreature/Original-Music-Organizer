#ifndef T3F_PRIMITIVES_H
#define T3F_PRIMITIVES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

typedef struct
{

	int max_vertices;
	ALLEGRO_VERTEX * vertex;
	int vertices;

} T3F_PRIMITIVES_CACHE;

T3F_PRIMITIVES_CACHE * t3f_create_primitives_cache(int max);
void t3f_destroy_primitives_cache(T3F_PRIMITIVES_CACHE * cp);
bool t3f_cache_primitive(T3F_PRIMITIVES_CACHE * cp, ALLEGRO_VERTEX v[], int vc);
bool t3f_cache_vertex(T3F_PRIMITIVES_CACHE * cp, double x, double y, double z, ALLEGRO_COLOR c, double u, double v);
void t3f_flush_cached_primitives(T3F_PRIMITIVES_CACHE * cp, ALLEGRO_BITMAP * bp, ALLEGRO_PRIM_TYPE ptype);

#endif
