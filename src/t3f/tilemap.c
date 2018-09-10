#include <math.h>
#include "t3f.h"
#include "file.h"
#include "animation.h"
#include "tilemap.h"
#include "view.h"

T3F_TILE * t3f_create_tile(void)
{
	T3F_TILE * tp;

	tp = malloc(sizeof(T3F_TILE));
	if(!tp)
	{
		return NULL;
	}
	memset(tp, 0, sizeof(T3F_TILE));
	return tp;
}

void t3f_destroy_tile(T3F_TILE * tp)
{
	t3f_destroy_animation(tp->ap);
	free(tp);
}

short t3f_get_tile(T3F_TILESET * tsp, int tile, int tick)
{
	if(tsp->tile[tile]->flags & T3F_TILE_FLAG_ANIMATED && tsp->tile[tile]->frame_list_total > 0)
	{
		if(tick >= tsp->tile[tile]->frame_list_total && tsp->tile[tile]->flags & T3F_TILE_FLAG_ONCE)
		{
			return tsp->tile[tile]->frame_list[tsp->tile[tile]->frame_list_total - 1];
		}
		else
		{
			return tsp->tile[tile]->frame_list[tick % tsp->tile[tile]->frame_list_total];
		}
	}
	return tile;
}

T3F_TILESET * t3f_create_tileset(int w, int h)
{
	T3F_TILESET * tsp;

	tsp = malloc(sizeof(T3F_TILESET));
	if(!tsp)
	{
		return NULL;
	}
	tsp->atlas = NULL;
	tsp->tiles = 0;
	tsp->width = w;
	tsp->height = h;
	return tsp;
}

void t3f_destroy_tileset(T3F_TILESET * tsp)
{
	int i;

	for(i = 0; i < tsp->tiles; i++)
	{
		t3f_destroy_tile(tsp->tile[i]);
	}
	if(tsp->atlas)
	{
		t3f_destroy_atlas(tsp->atlas);
	}
	free(tsp);
}

T3F_TILESET * t3f_load_tileset_f(ALLEGRO_FILE * fp, const char * fn)
{
	int i, j;
	T3F_TILESET * tsp;
	char header[16];

	tsp = t3f_create_tileset(0, 0);
	if(!tsp)
	{
		return NULL;
	}
	al_fread(fp, header, 16);
	if(strcmp(header, "T3F_TILESET"))
	{
		t3f_destroy_tileset(tsp);
		return NULL;
	}
	switch(header[15])
	{
		case 0:
		{
			/* read tile data */
			tsp->tiles = al_fread16le(fp);
			for(i = 0; i < tsp->tiles; i++)
			{
				tsp->tile[i] = t3f_create_tile();
				tsp->tile[i]->ap = t3f_load_animation_f(fp, fn);
				if(!tsp->tile[i]->ap)
				{
					printf("load animation failed\n");
					return NULL;
				}
				tsp->tile[i]->flags = al_fread32le(fp);

				/* read user data */
				if(tsp->tile[i]->flags & T3F_TILE_FLAG_USER_DATA)
				{
					for(j = 0; j < T3F_TILE_MAX_DATA; j++)
					{
						tsp->tile[i]->user_data[j] = al_fread32le(fp);
					}
				}

				/* read animation frames */
				tsp->tile[i]->frame_list_total = al_fread32le(fp);
				for(j = 0; j < tsp->tile[i]->frame_list_total; j++)
				{
					tsp->tile[i]->frame_list[j] = al_fread16le(fp);
				}
			}

			/* read tileset data */
			tsp->width = al_fread32le(fp);
			tsp->height = al_fread32le(fp);
			tsp->flags = al_fread32le(fp);

			break;
		}
	}
	return tsp;
}

T3F_TILESET * t3f_load_tileset(const char * fn)
{
	ALLEGRO_FILE * fp;
	T3F_TILESET * tsp;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	tsp = t3f_load_tileset_f(fp, fn);
	al_fclose(fp);
	return tsp;
}

int t3f_save_tileset_f(T3F_TILESET * tsp, ALLEGRO_FILE * fp)
{
	int i, j;
	char header[16] = {0};
	strcpy(header, "T3F_TILESET");
	header[15] = 0;

	al_fwrite(fp, header, 16);

	/* write tile data */
	al_fwrite16le(fp, tsp->tiles);
	for(i = 0; i < tsp->tiles; i++)
	{
		t3f_save_animation_f(tsp->tile[i]->ap, fp);
		al_fwrite32le(fp, tsp->tile[i]->flags);

		/* write user data */
		if(tsp->tile[i]->flags & T3F_TILE_FLAG_USER_DATA)
		{
			for(j = 0; j < T3F_TILE_MAX_DATA; j++)
			{
				al_fwrite32le(fp, tsp->tile[i]->user_data[j]);
			}
		}

		/* write animation frames */
		al_fwrite32le(fp, tsp->tile[i]->frame_list_total);
		for(j = 0; j < tsp->tile[i]->frame_list_total; j++)
		{
			al_fwrite16le(fp, tsp->tile[i]->frame_list[j]);
		}
	}

	/* write tileset data */
	al_fwrite32le(fp, tsp->width);
	al_fwrite32le(fp, tsp->height);
	al_fwrite32le(fp, tsp->flags);
	return 1;
}

int t3f_save_tileset(T3F_TILESET * tsp, const char * fn)
{
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return 0;
	}
	t3f_save_tileset_f(tsp, fp);
	al_fclose(fp);
	return 1;
}

bool t3f_add_tile(T3F_TILESET * tsp, T3F_ANIMATION * ap)
{
	T3F_TILE * tp;

	tp = t3f_create_tile();
	if(!tp)
	{
		return false;
	}
	tp->ap = ap;
	tsp->tile[tsp->tiles] = tp;
	tsp->tiles++;
	return true;
}

bool t3f_atlas_tileset(T3F_TILESET * tsp)
{
	int tile_sheet_size = 1024; // may want to calculate this from the tile data for an optimization
	int i;
	bool fail = false;

	tsp->atlas = t3f_create_atlas(tile_sheet_size, tile_sheet_size);
	if(!tsp->atlas)
	{
		return false;
	}
	for(i = 0; i < tsp->tiles; i++)
	{
		if(!t3f_add_animation_to_atlas(tsp->atlas, tsp->tile[i]->ap, T3F_ATLAS_TILE))
		{
			printf("sprite sheet failed\n");
			fail = true;
		}
	}
	return true;
}

T3F_TILEMAP_LAYER * t3f_create_tilemap_layer(int w, int h)
{
	T3F_TILEMAP_LAYER * tlp;
	int i, j;

	tlp = malloc(sizeof(T3F_TILEMAP_LAYER));
	if(!tlp)
	{
		return NULL;
	}
	tlp->data = malloc(h * sizeof(short *));
	if(!tlp->data)
	{
		free(tlp);
		return NULL;
	}
	for(i = 0; i < h; i++)
	{
		tlp->data[i] = malloc(w * sizeof(short));
		if(!tlp->data[i])
		{
			return NULL;
		}
	}
	for(i = 0; i < h; i++)
	{
		for(j = 0; j < w; j++)
		{
			tlp->data[i][j] = 0;
		}
	}
	tlp->bitmap = 0;
	tlp->width = w;
	tlp->height = h;
	tlp->x = 0.0;
	tlp->y = 0.0;
	tlp->z = 0.0;
	tlp->scale = 1.0;
	tlp->speed_x = 1.0;
	tlp->speed_y = 1.0;
	tlp->flags = 0;
	return tlp;
}

void t3f_destroy_tilemap_layer(T3F_TILEMAP_LAYER * tlp)
{
	int i;
	for(i = 0; i < tlp->height; i++)
	{
		free(tlp->data[i]);
	}
	free(tlp->data);
	free(tlp);
}

T3F_TILEMAP * t3f_create_tilemap(int w, int h, int layers)
{
	T3F_TILEMAP * tmp;
	int i;

	tmp = malloc(sizeof(T3F_TILEMAP));
	if(!tmp)
	{
		return NULL;
	}
	for(i = 0; i < layers; i++)
	{
		tmp->layer[i] = t3f_create_tilemap_layer(w, h);
	}
	tmp->layers = layers;
	tmp->flags = 0;

	return tmp;
}

void t3f_destroy_tilemap(T3F_TILEMAP * tmp)
{
	int i;

	for(i = 0; i < tmp->layers; i++)
	{
		t3f_destroy_tilemap_layer(tmp->layer[i]);
	}
	free(tmp);
}

T3F_TILEMAP * t3f_load_tilemap_f(ALLEGRO_FILE * fp)
{
	int i, j, k, w, h;
	T3F_TILEMAP * tmp;
	char header[16];

	al_fread(fp, header, 16);
	if(strcmp(header, "T3F_TILEMAP"))
	{
		return NULL;
	}
	tmp = malloc(sizeof(T3F_TILEMAP));
	if(!tmp)
	{
		return NULL;
	}
	switch(header[15])
	{
		case 0:
		{
			tmp->layers = al_fread16le(fp);
			for(i = 0; i < tmp->layers; i++)
			{
				tmp->layer[i] = malloc(sizeof(T3F_TILEMAP_LAYER));
				w = al_fread16le(fp);
				h = al_fread16le(fp);
				tmp->layer[i] = t3f_create_tilemap_layer(w, h);
				for(j = 0; j < tmp->layer[i]->height; j++)
				{
					for(k = 0; k < tmp->layer[i]->width; k++)
					{
						tmp->layer[i]->data[j][k] = al_fread16le(fp);
					}
				}
				tmp->layer[i]->x = t3f_fread_float(fp);
				tmp->layer[i]->y = t3f_fread_float(fp);
				tmp->layer[i]->z = t3f_fread_float(fp);
				tmp->layer[i]->scale = t3f_fread_float(fp);
				tmp->layer[i]->speed_x = t3f_fread_float(fp);
				tmp->layer[i]->speed_y = t3f_fread_float(fp);
				tmp->layer[i]->flags = al_fread32le(fp);
			}
			tmp->flags = al_fread32le(fp);
			break;
		}
	}
	return tmp;
}

T3F_TILEMAP * t3f_load_tilemap(const char * fn)
{
	ALLEGRO_FILE * fp;
	T3F_TILEMAP * tmp;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	tmp = t3f_load_tilemap_f(fp);
	al_fclose(fp);
	return tmp;
}

int t3f_save_tilemap_f(T3F_TILEMAP * tmp, ALLEGRO_FILE * fp)
{
	int i, j, k;
	char header[16] = {0};
	strcpy(header, "T3F_TILEMAP");
	header[15] = 0;

	al_fwrite(fp, header, 16);
	al_fwrite16le(fp, tmp->layers);
	for(i = 0; i < tmp->layers; i++)
	{
		al_fwrite16le(fp, tmp->layer[i]->width);
		al_fwrite16le(fp, tmp->layer[i]->height);
		for(j = 0; j < tmp->layer[i]->height; j++)
		{
			for(k = 0; k < tmp->layer[i]->width; k++)
			{
				al_fwrite16le(fp, tmp->layer[i]->data[j][k]);
			}
		}
		t3f_fwrite_float(fp, tmp->layer[i]->x);
		t3f_fwrite_float(fp, tmp->layer[i]->y);
		t3f_fwrite_float(fp, tmp->layer[i]->z);
		t3f_fwrite_float(fp, tmp->layer[i]->scale);
		t3f_fwrite_float(fp, tmp->layer[i]->speed_x);
		t3f_fwrite_float(fp, tmp->layer[i]->speed_y);
		al_fwrite32le(fp, tmp->layer[i]->flags);
	}
	al_fwrite32le(fp, tmp->flags);
	return 1;
}

int t3f_save_tilemap(T3F_TILEMAP * tmp, const char * fn)
{
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return 0;
	}
	t3f_save_tilemap_f(tmp, fp);
	al_fclose(fp);
	return 1;
}

static float t3f_get_speed(T3F_TILEMAP * tmp, int layer, float oz)
{
	return (t3f_project_x(1.0, tmp->layer[layer]->z - oz) - t3f_project_x(0.0, tmp->layer[layer]->z - oz));
}

static void t3f_render_static_tilemap(T3F_TILEMAP * tmp, T3F_TILESET * tsp, int layer, int tick, float ox, float oy, float oz, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE old_blender;
	int i, j;
	bool held;

	held = al_is_bitmap_drawing_held();
	al_store_state(&old_blender, ALLEGRO_STATE_BLENDER);
	if(tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_SOLID)
	{
		if(held)
		{
			al_hold_bitmap_drawing(false);
		}
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	}
	al_hold_bitmap_drawing(true);
	for(i = 0; i < (t3f_virtual_display_height / tsp->height) + 1; i++)
	{
		for(j = 0; j < (t3f_virtual_display_width / tsp->width) + 1; j++)
		{
			t3f_draw_scaled_animation(tsp->tile[t3f_get_tile(tsp, tmp->layer[layer]->data[i][j], tick)]->ap, color, tick, (float)(j * tsp->width) * tmp->layer[layer]->scale, (float)(i * tsp->height) * tmp->layer[layer]->scale, 0, tmp->layer[layer]->scale, 0);
		}
	}
	if(tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_SOLID)
	{
		al_hold_bitmap_drawing(false);
	}
	al_hold_bitmap_drawing(held);
	al_restore_state(&old_blender);
}

static void t3f_render_normal_tilemap(T3F_TILEMAP * tmp, T3F_TILESET * tsp, int layer, int tick, float ox, float oy, float oz, ALLEGRO_COLOR color)
{
	int startx, ostartx;
	int starty, ostarty;
	float fox, foy;
	float tw;
	float th;
	int tx, px;
	int ty, py;
	float zsp = t3f_get_speed(tmp, layer, oz);
	float ziw = (float)tsp->width * tmp->layer[layer]->scale;
	float zih = (float)tsp->height * tmp->layer[layer]->scale;
	float ztp = zsp * ziw;
	float zhp = zsp * zih;
	float sw;
	float sh;
	float cx = (ox * tmp->layer[layer]->speed_x) - tmp->layer[layer]->x;
	float cy = (oy * tmp->layer[layer]->speed_y) - tmp->layer[layer]->y;
	ALLEGRO_STATE old_blender;
	bool held;

	sw = t3f_virtual_display_width;
	sh = t3f_virtual_display_height;

	/* calculate total visible tiles */
	tw = sw / ztp; // width of screen divided by total width of tile in pixels
	th = sh / zhp;

	/* calculate first visible horizontal tile */
	fox = (cx * zsp) / ztp - ((t3f_project_x(0.0, tmp->layer[layer]->z - oz)) / ztp);
	ostartx = fox;
	if(fox < 0.0)
	{
		ostartx--;
	}
	ostartx--;
	startx = ostartx;
	while(startx < 0)
	{
		startx += tmp->layer[layer]->width;
	}
	while(startx >= tmp->layer[layer]->width)
	{
		startx -= tmp->layer[layer]->width;
	}

	/* calculate first visible vertical tile */
	foy = (cy * zsp) / zhp - ((t3f_project_y(0.0, tmp->layer[layer]->z - oz)) / zhp);
	ostarty = foy;
	if(foy < 0.0)
	{
		ostarty--;
	}
	ostarty--;
	starty = ostarty;
	while(starty < 0)
	{
		starty += tmp->layer[layer]->height;
	}
	while(starty >= tmp->layer[layer]->height)
	{
		starty -= tmp->layer[layer]->height;
	}

	/* render the tiles */
	ty = ostarty;
	py = starty;

	held = al_is_bitmap_drawing_held();
	al_store_state(&old_blender, ALLEGRO_STATE_BLENDER);
	if(tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_SOLID)
	{
		if(held)
		{
			al_hold_bitmap_drawing(false);
		}
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	}
	al_hold_bitmap_drawing(true);
	while(ty < ostarty + (int)th + 3)
	{
		tx = ostartx;
		px = startx;
		while(tx < ostartx + (int)tw + 3)
		{
			if(tmp->layer[layer]->data[py][px] != 0 || (tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_SOLID))
			{
				t3f_draw_scaled_animation(tsp->tile[t3f_get_tile(tsp, tmp->layer[layer]->data[py][px], tick)]->ap, color, tick, tmp->layer[layer]->x + (float)tx * ziw - ox * tmp->layer[layer]->speed_x, tmp->layer[layer]->y + (float)ty * zih - oy * tmp->layer[layer]->speed_y, tmp->layer[layer]->z - oz, tmp->layer[layer]->scale, 0);
			}
			tx++;
			px++;
			if(px >= tmp->layer[layer]->width)
			{
				px = 0;
			}
		}
		ty++;
		py++;
		if(py >= tmp->layer[layer]->height)
		{
			py = 0;
		}
	}
	if(tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_SOLID)
	{
		al_hold_bitmap_drawing(false);
	}
	al_hold_bitmap_drawing(held);
	al_restore_state(&old_blender);
}

/* figure the upper left tile (ostartx, ostarty)
   make sure the tile that is scrolling off the screen is included
   figure the dimensions (in tiles) of the screen (including partially visible tiles) */
void t3f_render_tilemap(T3F_TILEMAP * tmp, T3F_TILESET * tsp, int layer, int tick, float ox, float oy, float oz, ALLEGRO_COLOR color)
{
	if(tmp->layer[layer]->flags & T3F_TILEMAP_LAYER_STATIC)
	{
		t3f_render_static_tilemap(tmp, tsp, layer, tick, ox, oy, oz, color);
	}
	else
	{
		t3f_render_normal_tilemap(tmp, tsp, layer, tick, ox, oy, oz, color);
	}
}
