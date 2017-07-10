#include "primitives.h"

T3F_PRIMITIVES_CACHE * t3f_create_primitives_cache(int max)
{
	T3F_PRIMITIVES_CACHE * cp;
	
	cp = malloc(sizeof(T3F_PRIMITIVES_CACHE));
	if(cp)
	{
		cp->vertex = malloc(sizeof(ALLEGRO_VERTEX) * max);
		if(cp->vertex)
		{
			cp->max_vertices = max;
			cp->vertices = 0;
		}
		else
		{
			free(cp);
			cp = NULL;
		}
	}
	return cp;
}

void t3f_destroy_primitives_cache(T3F_PRIMITIVES_CACHE * cp)
{
	free(cp->vertex);
	free(cp);
}

bool t3f_cache_primitive(T3F_PRIMITIVES_CACHE * cp, ALLEGRO_VERTEX v[], int vc)
{
	int i;
	
	if(cp->vertices + vc < cp->max_vertices)
	{
		for(i = 0; i < vc; i++)
		{
			memcpy(&cp->vertex[cp->vertices], &v[i], sizeof(ALLEGRO_VERTEX));
			cp->vertices++;
		}
	}
	return false;
}

bool t3f_cache_vertex(T3F_PRIMITIVES_CACHE * cp, double x, double y, double z, ALLEGRO_COLOR c, double u, double v)
{
	if(cp->vertices < cp->max_vertices)
	{
		cp->vertex[cp->vertices].x = x;
		cp->vertex[cp->vertices].y = y;
		cp->vertex[cp->vertices].z = z;
		cp->vertex[cp->vertices].color = c;
		cp->vertex[cp->vertices].u = u;
		cp->vertex[cp->vertices].v = v;
		return true;
	}
	return false;
}

void t3f_flush_cached_primitives(T3F_PRIMITIVES_CACHE * cp, ALLEGRO_BITMAP * bp, ALLEGRO_PRIM_TYPE ptype)
{
	al_draw_prim(cp->vertex, NULL, bp, 0, cp->vertices, ptype);
	cp->vertices = 0;
}
