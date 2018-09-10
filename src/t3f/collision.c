#include <allegro5/allegro5.h>
#ifndef ALLEGRO_MACOSX
	#ifndef ALLEGRO_IPHONE
		#include <malloc.h>
	#endif
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "t3f.h"
#include "file.h"
#include "collision.h"

static void add_collision_point(T3F_COLLISION_LIST * lp, float x, float y)
{
	lp->point[lp->points].x = x;
	lp->point[lp->points].y = y;
	lp->points++;
}

T3F_COLLISION_OBJECT * t3f_create_collision_object(float rx, float ry, float w, float h, int tw, int th, int flags)
{
	T3F_COLLISION_OBJECT * cp;
	float i;

	cp = malloc(sizeof(T3F_COLLISION_OBJECT));
	if(!cp)
	{
		return NULL;
	}
	cp->x = 0.0;
	cp->y = 0.0;
	cp->map.top.points = 0;
	cp->map.bottom.points = 0;
	cp->map.left.points = 0;
	cp->map.right.points = 0;

	/* map top points */
	add_collision_point(&cp->map.top, rx + w / 2.0, ry); // center collision point
	for(i = rx; i < rx + w; i += tw)
	{
		add_collision_point(&cp->map.top, i, ry);
	}
	add_collision_point(&cp->map.top, rx + w - 1.0, ry);

	/* map bottom points */
	add_collision_point(&cp->map.bottom, rx + w / 2.0, ry + h - 1.0); // center collision point
	for(i = rx; i < rx + w; i += tw)
	{
		add_collision_point(&cp->map.bottom, i, ry + h - 1.0);
	}
	add_collision_point(&cp->map.bottom, rx + w - 1.0, ry + h - 1.0);

	/* map left points */
	add_collision_point(&cp->map.left, rx, ry + h / 2.0); // center collision point
	for(i = ry; i < ry + h; i += th)
	{
		add_collision_point(&cp->map.left, rx, i);
	}
	add_collision_point(&cp->map.left, rx, ry + h - 1.0);

	/* map right points */
	add_collision_point(&cp->map.right, rx + w - 1.0, ry + h / 2.0); // center collision point
	for(i = ry; i < ry + h; i += th)
	{
		add_collision_point(&cp->map.right, rx + w - 1.0, i);
	}
	add_collision_point(&cp->map.right, rx + w - 1.0, ry + h - 1.0);

	cp->flags = flags;
	return cp;
}

void t3f_recreate_collision_object(T3F_COLLISION_OBJECT * cp, float rx, float ry, float w, float h, int tw, int th, int flags)
{
	float i;

	cp->x = 0.0;
	cp->y = 0.0;
	cp->map.top.points = 0;
	cp->map.bottom.points = 0;
	cp->map.left.points = 0;
	cp->map.right.points = 0;

	/* map top points */
	add_collision_point(&cp->map.top, (rx + w) / 2.0, ry); // center collision point
	for(i = rx; i < w; i += tw)
	{
		add_collision_point(&cp->map.top, i, ry);
	}
	add_collision_point(&cp->map.top, rx + w - 1.0, ry);

	/* map bottom points */
	add_collision_point(&cp->map.bottom, (rx + w) / 2.0, ry + h - 1.0); // center collision point
	for(i = rx; i < w; i += tw)
	{
		add_collision_point(&cp->map.bottom, i, ry + h - 1.0);
	}
	add_collision_point(&cp->map.bottom, rx + w - 1.0, ry + h - 1.0);

	/* map left points */
	add_collision_point(&cp->map.left, rx, (ry + h) / 2.0); // center collision point
	for(i = ry; i < h; i += th)
	{
		add_collision_point(&cp->map.left, rx, i);
	}
	add_collision_point(&cp->map.left, rx, ry + h - 1.0);

	/* map right points */
	add_collision_point(&cp->map.right, rx + w - 1.0, (ry + h) / 2.0); // center collision point
	for(i = ry; i < h; i += th)
	{
		add_collision_point(&cp->map.right, rx + w - 1.0, i);
	}
	add_collision_point(&cp->map.right, rx + w - 1.0, ry + h - 1.0);

	cp->flags = flags;
}

void t3f_destroy_collision_object(T3F_COLLISION_OBJECT * cp)
{
	free(cp);
}

T3F_COLLISION_OBJECT * t3f_load_collision_object_f(ALLEGRO_FILE * fp, int tw, int th)
{
	T3F_COLLISION_OBJECT * op = NULL;
	char header[16];
	float rx, ry, w, h;
	int flags;

	al_fread(fp, header, 16);
	if(strcmp(header, "T3F_COBJECT"))
	{
		return NULL;
	}
	op = t3f_create_collision_object(0, 0, 32, 32, 32, 32, 0);
	if(!op)
	{
		return NULL;
	}
	switch(header[15])
	{
		case 0:
		{
			rx = t3f_fread_float(fp);
			ry = t3f_fread_float(fp);
			w = t3f_fread_float(fp);
			h = t3f_fread_float(fp);
			flags = al_fread32le(fp);
			t3f_recreate_collision_object(op, rx, ry, w, h, tw, th, flags);
			break;
		}
	}
	return op;
}

T3F_COLLISION_OBJECT * t3f_load_collision_object(const char * fn, int tw, int th)
{
	ALLEGRO_FILE * fp;
	T3F_COLLISION_OBJECT * op = NULL;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	op = t3f_load_collision_object_f(fp, tw, th);
	al_fclose(fp);
	return op;
}

bool t3f_save_collision_object_f(T3F_COLLISION_OBJECT * op, ALLEGRO_FILE * fp)
{
	char header[16];
	float rx, ry, w, h;

	rx = op->map.left.point[0].x;
	ry = op->map.top.point[0].y;
	w = op->map.right.point[0].x - op->map.left.point[0].x;
	h = op->map.bottom.point[0].y - op->map.top.point[0].y;
	strcpy(header, "T3F_COBJECT");
	header[15] = 0;
	al_fwrite(fp, header, 16);
	rx = t3f_fwrite_float(fp, rx);
	ry = t3f_fwrite_float(fp, ry);
	w = t3f_fwrite_float(fp, w);
	h = t3f_fwrite_float(fp, h);
	al_fwrite32le(fp, op->flags);
	return true;
}

bool t3f_save_collision_object(T3F_COLLISION_OBJECT * op, const char * fn)
{
	ALLEGRO_FILE * fp;
	bool ret;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return false;
	}
	ret = t3f_save_collision_object_f(op, fp);
	al_fclose(fp);
	return ret;
}

T3F_COLLISION_TILEMAP * t3f_create_collision_tilemap(int w, int h, int tw, int th)
{
	T3F_COLLISION_TILEMAP * tmp;
	int i, j;

	tmp = malloc(sizeof(T3F_COLLISION_TILEMAP));
	if(!tmp)
	{
		return NULL;
	}
	tmp->data = malloc(h * sizeof(T3F_COLLISION_TILE *));
	if(!tmp->data)
	{
		free(tmp);
		return NULL;
	}
	tmp->width = w;
	tmp->height = h;
	tmp->tile_width = tw;
	tmp->tile_height = th;
	tmp->flags = 0;
	for(i = 0; i < h; i++)
	{
		tmp->data[i] = malloc(w * sizeof(T3F_COLLISION_TILE));
		if(!tmp->data[i])
		{
			return NULL;
		}
	}
	for(i = 0; i < h; i++)
	{
		for(j = 0; j < w; j++)
		{
			tmp->data[i][j].slope = NULL;
			tmp->data[i][j].user_data = NULL;
			tmp->data[i][j].flags = 0;
		}
	}
	return tmp;
}

void t3f_destroy_collision_tilemap(T3F_COLLISION_TILEMAP * tmp)
{
	int i, j;
	for(i = 0; i < tmp->height; i++)
	{
		for(j = 0; j < tmp->width; j++)
		{
			if(tmp->data[i][j].slope)
			{
				free(tmp->data[i][j].slope);
			}
			if(tmp->data[i][j].user_data)
			{
				free(tmp->data[i][j].user_data);
			}
		}
	}
	for(i = 0; i < tmp->height; i++)
	{
		free(tmp->data[i]);
	}
	free(tmp->data);
	free(tmp);
}

T3F_COLLISION_TILEMAP * t3f_load_collision_tilemap_f(ALLEGRO_FILE * fp)
{
	T3F_COLLISION_TILEMAP * tmp = NULL;
	char header[16];
	int j, k, l;

	if(al_fread(fp, header, 16) != 16)
	{
		printf("read failed\n");
	}
	if(strcmp(header, "T3F_CTILEMAP"))
	{
		printf("collision header fail %s\n", header);
		return NULL;
	}
	switch(header[15])
	{
		case 0:
		{
			int w = al_fread16le(fp);
			int h = al_fread16le(fp);
			int tw = al_fread16le(fp);
			int th = al_fread16le(fp);
			char c;
			tmp = t3f_create_collision_tilemap(w, h, tw, th);
			tmp->flags = al_fread32le(fp);
			for(j = 0; j < tmp->height; j++)
			{
				for(k = 0; k < tmp->width; k++)
				{
					if(tmp->flags & T3F_COLLISION_TILEMAP_FLAG_USER_DATA)
					{
						c = al_fgetc(fp);
						if(c > 0)
						{
							tmp->data[j][k].user_data = malloc(sizeof(int) * c);
							for(l = 0; l < c; l++)
							{
								tmp->data[j][k].user_data[l] = al_fread32le(fp);
							}
						}
					}
					if(tmp->flags & T3F_COLLISION_TILEMAP_FLAG_SLOPES)
					{
						c = al_fgetc(fp);
						if(c)
						{
							tmp->data[j][k].slope = malloc(tmp->tile_height > tmp->tile_width ? tmp->tile_height : tmp->tile_width);
							for(l = 0; l < (tmp->tile_height > tmp->tile_width ? tmp->tile_height : tmp->tile_width); l++)
							{
								tmp->data[j][k].slope[l] = al_fgetc(fp);
							}
						}
					}
					tmp->data[j][k].flags = al_fread32le(fp);
				}
			}
			break;
		}
	}
	return tmp;
}

T3F_COLLISION_TILEMAP * t3f_load_collision_tilemap(char * fn)
{
	ALLEGRO_FILE * fp;
	T3F_COLLISION_TILEMAP * tmp;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	tmp = t3f_load_collision_tilemap_f(fp);
	al_fclose(fp);
	return tmp;
}

bool t3f_save_collision_tilemap_f(T3F_COLLISION_TILEMAP * tmp, ALLEGRO_FILE * fp)
{
	char header[16] = {0};
	int j, k, l;
	strcpy(header, "T3F_CTILEMAP");

	al_fwrite(fp, header, 16);
	al_fwrite16le(fp, tmp->width);
	al_fwrite16le(fp, tmp->height);
	al_fwrite16le(fp, tmp->tile_width);
	al_fwrite16le(fp, tmp->tile_height);
	al_fwrite32le(fp, tmp->flags);
	for(j = 0; j < tmp->height; j++)
	{
		for(k = 0; k < tmp->width; k++)
		{
			if(tmp->flags & T3F_COLLISION_TILEMAP_FLAG_USER_DATA)
			{
				al_fputc(fp, sizeof(tmp->data[j][k].user_data) / sizeof(int));
				for(l = 0; l < (int)(sizeof(tmp->data[j][k].user_data) / sizeof(int)); l++)
				{
					al_fwrite32le(fp, tmp->data[j][k].user_data[l]);
				}
			}
			if(tmp->flags & T3F_COLLISION_TILEMAP_FLAG_SLOPES)
			{
				if(tmp->data[j][k].slope)
				{
					al_fputc(fp, 1);
					for(l = 0; l < (tmp->tile_height > tmp->tile_width ? tmp->tile_height : tmp->tile_width); l++)
					{
						tmp->data[j][k].slope[l] = al_fgetc(fp);
					}
				}
				else
				{
					al_fputc(fp, 0);
				}
			}
			al_fwrite32le(fp, tmp->data[j][k].flags);
		}
	}
	return 1;
}

bool t3f_save_collision_tilemap(T3F_COLLISION_TILEMAP * tmp, char * fn)
{
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return 0;
	}
	t3f_save_collision_tilemap_f(tmp, fp);
	al_fclose(fp);
	return 1;
}

void t3f_move_collision_object_x(T3F_COLLISION_OBJECT * cp, float x)
{
	cp->ox = cp->x;
	cp->x = x;
	cp->vx = cp->x - cp->ox;

//	cp->ox = cp->x;
//	cp->x = x;
//	cp->vx = cp->x - cp->ox;
}

void t3f_move_collision_object_y(T3F_COLLISION_OBJECT * cp, float y)
{
	cp->oy = cp->y;
	cp->y = y;
	cp->vy = cp->y - cp->oy;

//	cp->oy = cp->y;
//	cp->y = y;
//	cp->vy = cp->y - cp->oy;
}

/* you will want to move x and y separately unless you don't need to use the collision
   response functions (get_object_collision_*) */
void t3f_move_collision_object_xy(T3F_COLLISION_OBJECT * cp, float x, float y)
{
	t3f_move_collision_object_x(cp, x);
	t3f_move_collision_object_y(cp, y);
}

float t3f_get_collision_object_left_x(T3F_COLLISION_OBJECT * cp)
{
	return cp->x + cp->map.left.point[0].x;
}

float t3f_get_collision_object_right_x(T3F_COLLISION_OBJECT * cp)
{
	return cp->x + cp->map.right.point[0].x;
}

float t3f_get_collision_object_top_x(T3F_COLLISION_OBJECT * cp)
{
	return cp->y + cp->map.top.point[0].y;
}

float t3f_get_collision_object_bottom_x(T3F_COLLISION_OBJECT * cp)
{
	return cp->y + cp->map.bottom.point[0].y;
}

/* see if cp1 overlaps cp2 */
int t3f_check_object_collision(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	return ((cp1->y + cp1->map.top.point[0].y <= cp2->y + cp2->map.bottom.point[0].y) && (cp2->y + cp2->map.top.point[0].y <= cp1->y + cp1->map.bottom.point[0].y) && (cp1->x + cp1->map.left.point[0].x <= cp2->x + cp2->map.right.point[0].x) && (cp2->x + cp2->map.left.point[0].x <= cp1->x + cp1->map.right.point[0].x));
}

float t3f_get_object_left_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	return cp2->x + cp2->map.left.point[0].x - (cp1->map.left.point[0].x + (cp1->map.right.point[0].x - cp1->map.left.point[0].x + 1.0));
}

float t3f_get_object_right_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	return cp2->x + cp2->map.right.point[0].x - cp1->map.left.point[0].x + 1.0;
}

float t3f_get_object_collision_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	if(cp1->vx < 0.0)
	{
		return t3f_get_object_right_x(cp1, cp2);
	}
	else if(cp1->vx > 0.0)
	{
		return t3f_get_object_left_x(cp1, cp2);
	}
	return cp1->x;
}

float t3f_get_object_top_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	return cp2->y + cp2->map.top.point[0].y - (cp1->map.top.point[0].y + (cp1->map.bottom.point[0].y - cp1->map.top.point[0].y + 1.0));
}

float t3f_get_object_bottom_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	return cp2->y + cp2->map.bottom.point[0].y - cp1->map.top.point[0].y + 1.0;
}

/* assuming cp1 has just overlapped cp2, return the y position that will place cp1
   just at the edge of cp2 */
float t3f_get_object_collision_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2)
{
	if(cp1->vy < 0.0)
	{
		return t3f_get_object_bottom_y(cp1, cp2);
	}
	else if(cp1->vy > 0.0)
	{
		return t3f_get_object_top_y(cp1, cp2);
	}
	return cp1->y;
}

int t3f_get_collision_tile_x(T3F_COLLISION_TILEMAP * tmp, float x)
{
	int ctx;
	float tx = x;
	float total_width = tmp->width * tmp->tile_width;

	/* handle map looping */
	while(tx < 0)
	{
		tx += total_width;
	}
	while(tx >= total_width)
	{
		tx -= total_width;
	}

	/* figure out tile index */
	ctx = (int)tx / tmp->tile_width;

	return ctx;
}

int t3f_get_collision_tile_y(T3F_COLLISION_TILEMAP * tmp, float y)
{
	int cty;
	float ty = y;
	float total_height = tmp->height * tmp->tile_height;

	/* handle map looping */
	while(ty < 0)
	{
		ty += total_height;
	}
	while(ty >= total_height)
	{
		ty -= total_height;
	}

	/* figure out tile index */
	cty = (int)ty / tmp->tile_height;

	return cty;
}

T3F_COLLISION_TILE * t3f_get_collision_tile(T3F_COLLISION_TILEMAP * tmp, float x, float y)
{
	return &tmp->data[t3f_get_collision_tile_y(tmp, y)][t3f_get_collision_tile_x(tmp, x)];
}

int t3f_get_collision_tilemap_flag(T3F_COLLISION_TILEMAP * tmp, float x, float y, int flags)
{
	return tmp->data[t3f_get_collision_tile_y(tmp, y)][t3f_get_collision_tile_x(tmp, x)].flags & flags;
}

int t3f_get_collision_tilemap_data(T3F_COLLISION_TILEMAP * tmp, float x, float y, int i)
{
	return tmp->data[t3f_get_collision_tile_y(tmp, y)][t3f_get_collision_tile_x(tmp, x)].user_data[i];
}

int t3f_check_collision_tilemap_flag(T3F_COLLISION_TILEMAP * tmp, float x, float y, int inflags, int exflags)
{
	int ctx = t3f_get_collision_tile_x(tmp, x);
	int cty = t3f_get_collision_tile_y(tmp, y);
    if((tmp->data[cty][ctx].flags & inflags) && !(tmp->data[cty][ctx].flags & exflags))
    {
       	return 1;
    }
    return 0;
}

int t3f_check_tilemap_collision_top(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int i;

    if(cp->map.top.points > 0)
    {
	    if(cp->vy < 0.0)
	    {
	       	/* check the points */
        	for(i = 0; i < cp->map.top.points; i++)
        	{
				/* see if we need to check collision */
		    	if(fmodf(cp->y + cp->map.top.point[i].y, tmp->tile_height) > fmodf(cp->oy + cp->map.top.point[i].y, tmp->tile_height))
		    	{
	                if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.top.point[i].x, cp->y + cp->map.top.point[i].y, T3F_COLLISION_FLAG_SOLID_BOTTOM))
                	{
	                	return 1;
                	}
            	}
        	}
        }
    }
    return 0;
}

int t3f_check_tilemap_collision_bottom(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int i, f;
	int hit = 0;
	int inslope = 0;
	int inpslope = 0;
	int inpxslope = 0;
	int pinslope = 1;
//	int pxinslope = 0;
	bool crossed = false;

    if(cp->vy > 0.0)
    {
    	if(cp->map.bottom.points > 0)
    	{
       		/* check the points */
       		for(i = 0; i < cp->map.bottom.points; i++)
       		{
				/* see if we need to check collision */
       			if(fmodf(cp->y + cp->map.bottom.point[i].y, tmp->tile_height) < fmodf(cp->oy + cp->map.bottom.point[i].y, tmp->tile_height))
	    		{
	                if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[i].x, cp->y + cp->map.bottom.point[i].y, T3F_COLLISION_FLAG_SOLID_TOP))
                	{
	                	return 1;
                	}
		    		crossed = true;
		    		f = t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[i].x, cp->y + cp->map.bottom.point[i].y, T3F_COLLISION_FLAG_SLOPE_TOP);
//	                if((f & T3F_COLLISION_FLAG_SOLID_TOP) && !(f & T3F_COLLISION_FLAG_SLOPE_TOP))
	                if(f & T3F_COLLISION_FLAG_SOLID_TOP)
               		{
	                	hit = 1;
               		}
           		}

    		}
			/* if we crossed a tile border, check previous tile for slope */
			if(crossed && t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->oy + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP))
			{
				inpslope = 1;
			}
			/* check for slope */
			if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP))
			{
				inslope = 1;
			}
			if(t3f_get_collision_tilemap_flag(tmp, cp->ox + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP))
			{
				inpxslope = 1;
				printf("pxslope\n");
			}
        }
        if(inpslope || inslope || inpxslope)
        {
	    	T3F_COLLISION_TILE * tp = &tmp->data[t3f_get_collision_tile_y(tmp, cp->y + cp->map.bottom.point[0].y)][t3f_get_collision_tile_x(tmp, cp->x + cp->map.bottom.point[0].x)];
	    	T3F_COLLISION_TILE * pp = &tmp->data[t3f_get_collision_tile_y(tmp, cp->oy + cp->map.bottom.point[0].y)][t3f_get_collision_tile_x(tmp, cp->ox + cp->map.bottom.point[0].x)];
	    	T3F_COLLISION_TILE * pxp = &tmp->data[t3f_get_collision_tile_y(tmp, cp->y + cp->map.bottom.point[0].y)][t3f_get_collision_tile_x(tmp, cp->ox + cp->map.bottom.point[0].x)];
	    	T3F_COLLISION_TILE * pyp = &tmp->data[t3f_get_collision_tile_y(tmp, cp->oy + cp->map.bottom.point[0].y)][t3f_get_collision_tile_x(tmp, cp->x + cp->map.bottom.point[0].x)];
	    	int bpy = cp->y + cp->map.bottom.point[0].y;
	    	int bpoy = cp->oy + cp->map.bottom.point[0].y;

			if(tp->slope)
			{
				if((cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
				{
					printf("slope overlap\n");
				}
			}
			if(fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width) < fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width))
			{
				printf("xcross\n");
			}

			/* first see if sprite is moving within a tile */
	    	if(tp == pp)
	    	{
		    	if(tp->slope && (cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
		    	{
			    	printf("test 1\n");
			    	return 1;
		    	}
	    	}

	    	/* now see if sprite has crossed into the next tile */
	    	if(crossed && tp != pyp)
	    	{
		    	if(pyp->slope && (cp->oy + cp->map.bottom.point[0].y) <= (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
			    	printf("test 2\n");
			    	return 1;
	    		}
		    	if(tp->slope && (cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
			    	printf("test 2b\n");
			    	return 1;
	    		}
	    	}

	    	/* see if x movement caused sprite to cross the slope */
	    	if(tp == pxp)
	    	{
		    	if(pxp->slope && (cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->oy + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
			    	printf("test 3\n");
			    	return 1;
	    		}
	    	}

	    	/* see if we passed through the gap between two tiles */
	    	if(tp != pp)
	    	{
		    	printf("pregap\n");
		    	if(tp->slope)
		    	{
			    	if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    			{
				    	printf("test 4\n");
			    		return 1;
	    			}
		    	}
		    	if(pp->slope)
		    	{
			    	printf("would 5\n");
			    	if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpoy / tmp->tile_height) * tmp->tile_height + pp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpoy / tmp->tile_height) * tmp->tile_height + pp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    			{
				    	printf("test 5\n");
			    		return 1;
	    			}
		    	}
		    	if(pyp->slope)
		    	{
			    	printf("would 8\n");
		    		if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->y + cp->map.bottom.point[0].y) > (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
		    		{
			    		printf("test 8\n");
		    			return 1;
	    			}
    			}
		    	if(pxp->slope && tp->slope)
		    	{
			    	printf("would 9\n");
		    		if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]) && (cp->oy + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    			{
				    	printf("test 9\n");
			    		return 1;
	    			}
    			}
	    	}

	    	return hit;

	    	if(tp->slope)
	    	{
		    	if(pyp->slope)
		    	{
			    	if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
		    		{
				    	pinslope = 0;
		    		}
		    		if(!pp->slope)
		    		{
			    		printf("third\n");
			    		pinslope = 0;
		    		}
		    	}
		    	else if(pxp->slope)
		    	{
			    	if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]))
		    		{
			    		printf("fourth\n");
				    	pinslope = 0;
//				    	pxinslope = 1;
		    		}
		    	}
		    	else if(pp->slope)
		    	{
			    	if((cp->oy + cp->map.bottom.point[0].y) <= (float)((bpoy / tmp->tile_height) * tmp->tile_height + pp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]))
		    		{
				    	pinslope = 0;
		    		}
		    	}
		    	else
		    	{
			    	pinslope = 0;
		    	}
		    	if(!pinslope && (cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
				{
					return 1;
				}
	    		printf("first (%f, %f)\n", (cp->y + cp->map.bottom.point[0].y), (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]));
	    	}
	    	else if(pyp->slope)
	    	{
		    	printf("edge case\n");
		    	if(!pp->slope || (cp->y + cp->map.bottom.point[0].y) > (float)((bpoy / tmp->tile_height) * tmp->tile_height + pyp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
			    	printf("edge case confirm\n");
		    		return 1;
	    		}
	    	}
	    	else if(pxp->slope)
	    	{
		    	if((cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + pxp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
		    		printf("second\n");
		    		return 1;
	    		}
	    	}
	    	else if(pp->slope)
	    	{
		    	if((cp->y + cp->map.bottom.point[0].y) > (float)((bpoy / tmp->tile_height) * tmp->tile_height + pp->slope[(int)fmodf(cp->ox + cp->map.bottom.point[0].x, tmp->tile_width)]))
	    		{
		    		return 1;
	    		}
	    	}
	    	if((pxp->slope || pyp->slope) && !pp->slope)
	    	{
		    	if(tp->slope && (cp->y + cp->map.bottom.point[0].y) > (float)((bpy / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)]))
				{
			    	printf("last chance\n");
					return 1;
				}
	    	}
//	    	printf("nothing (%d, %d, %d, %d)\n", tp->slope, pp->slope, pxp->slope, pyp->slope);
        }
        return hit;
	}
	return 0;
}

int t3f_check_tilemap_collision_left(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int i;

    if(cp->map.left.points > 0)
    {
	    if(cp->vx < 0.0)
	    {
	       	/* check the points */
        	for(i = 0; i < cp->map.left.points; i++)
        	{
				/* see if we need to check collision */
		    	if(fmodf(cp->x + cp->map.left.point[i].x, tmp->tile_width) > fmodf(cp->ox + cp->map.left.point[i].x, tmp->tile_width))
		    	{
	                if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.left.point[i].x, cp->y + cp->map.left.point[i].y, T3F_COLLISION_FLAG_SOLID_RIGHT))
                	{
	                	return 1;
                	}
            	}
        	}
        }
    }
    return 0;
}

int t3f_check_tilemap_collision_right(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int i;

    if(cp->map.right.points > 0)
    {
	    if(cp->vx > 0.0)
	    {
	       	/* check the points */
        	for(i = 0; i < cp->map.right.points; i++)
        	{
				/* see if we need to check collision */
        		if(fmodf(cp->x + cp->map.right.point[i].x, tmp->tile_width) < fmodf(cp->ox + cp->map.right.point[i].x, tmp->tile_width))
		    	{
	                if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.right.point[i].x, cp->y + cp->map.right.point[i].y, T3F_COLLISION_FLAG_SOLID_LEFT))
                	{
	                	return 1;
                	}
            	}
        	}
        }
    }
    return 0;
}

int t3f_check_tilemap_collision_slope(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	if(cp->vy > 0.0)
	{
		if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.top.point[0].x, cp->y + cp->map.top.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_BOTTOM) == (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_BOTTOM))
		{
			return 1;
		}
	}
	else if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP) == (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP))
	{
		return 1;
	}
	else if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.left.point[0].x, cp->y + cp->map.left.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_RIGHT) == (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_RIGHT))
	{
		return 1;
	}
	else if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.right.point[0].x, cp->y + cp->map.right.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_LEFT) == (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_LEFT))
	{
		return 1;
	}
	return 0;
}

/* -see if cp overlaps any solid area in tmp
   -edges that have just crossed tile borders need to check the tile flags to
    see whether a collision has occurred
   -for sloped/curved surfaces, check if center point lies within the sloped/curved
    tile, if so, ignore straight edge collisions and see if the point crosses the
    slope */
int t3f_check_tilemap_collision(T3F_COLLISION_TILEMAP * tmp, T3F_COLLISION_OBJECT * cp)
{
	return 0;
}

/* gets sprite solid edge alignment value (use after collision) */
float t3f_get_tilemap_collision_x(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int tw = tmp->tile_width;
	float rx;
	float tx;

    /* if sprite was moving left */
    if(cp->x < cp->ox)
    {
		tx = (((int)(cp->x + cp->map.left.point[0].x) / tw) * tw) + tw;
	    rx =  tx- (int)(cp->map.left.point[0].x);
	    if(tx < 0)
	    {
		    rx -= tw;
	    }
        return rx;
    }

    /* if sprite was moving right */
    else if(cp->x > cp->ox)
    {
	    rx = (((int)(cp->x + cp->map.right.point[0].x) / tw) * tw) - (cp->map.right.point[0].x - cp->map.left.point[0].x) - 1 - (int)cp->map.left.point[0].x;
	    if(rx < 0)
	    {
		    rx -= tw;
	    }
        return rx;
    }

    /* if sprite wasn't moving */
    else
    {
        return cp->x;
    }
}

/* gets sprite solid edge alignment value (use after collision) */
float t3f_get_tilemap_collision_y(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int th = tmp->tile_height;
	float ry;
	float ty;

    /* if sprite was moving up */
    if(cp->y < cp->oy)
    {
		ty = (((int)(cp->y + cp->map.top.point[0].y) / th) * th) + th;
	    ry =  ty - (int)(cp->map.top.point[0].y);
	    if(ty < 0)
	    {
		    ry -= th;
	    }
        return ry;
    }

    /* if sprite was moving down */
    else if(cp->y > cp->oy)
    {
	    if(t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP) || t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->oy + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP))
	    {
		    return t3f_get_tilemap_walk_position(cp, tmp, T3F_COLLISION_FLAG_SOLID_TOP);
	    }
	    else
	    {
        	return (((int)(cp->y + cp->map.bottom.point[0].y) / th) * th) - (int)(cp->map.bottom.point[0].y - cp->map.top.point[0].y) - 1 - (int)(cp->map.top.point[0].y);
    	}
    }

    /* if sprite wasn't moving */
    else
    {
        return cp->y;
    }
}

float t3f_get_tilemap_slope_x(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	T3F_COLLISION_TILE * tp;

	tp = t3f_get_collision_tile(tmp, cp->x + cp->map.left.point[0].x, cp->y + cp->map.left.point[0].y);
	if(tp->flags & (T3F_COLLISION_FLAG_SOLID_RIGHT | T3F_COLLISION_FLAG_SLOPE_TOP))
	{
		return ((int)(cp->x + cp->map.left.point[0].x) / tmp->tile_width) * tmp->tile_width + tp->slope[(int)fmodf(cp->y + cp->map.left.point[0].y, tmp->tile_height)];
	}
	tp = t3f_get_collision_tile(tmp, cp->x + cp->map.right.point[0].x, cp->y + cp->map.right.point[0].y);
	if(tp->flags & (T3F_COLLISION_FLAG_SOLID_LEFT | T3F_COLLISION_FLAG_SLOPE_TOP))
	{
		return ((int)(cp->x + cp->map.right.point[0].x) / tmp->tile_width) * tmp->tile_width + tp->slope[(int)fmodf(cp->y + cp->map.right.point[0].y, tmp->tile_height)];
	}
	return cp->x;
}

float t3f_get_tilemap_slope_y(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	T3F_COLLISION_TILE * tp;

	tp = t3f_get_collision_tile(tmp, cp->x + cp->map.top.point[0].x, cp->y + cp->map.top.point[0].y);
	if(tp->flags & (T3F_COLLISION_FLAG_SOLID_BOTTOM | T3F_COLLISION_FLAG_SLOPE_TOP))
	{
		return ((int)(cp->y + cp->map.top.point[0].y) / tmp->tile_height) * tmp->tile_height + tp->slope[(int)fmodf(cp->x + cp->map.top.point[0].x, tmp->tile_width)];
	}
	tp = t3f_get_collision_tile(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y);
	if(tp->flags & (T3F_COLLISION_FLAG_SOLID_TOP | T3F_COLLISION_FLAG_SLOPE_TOP))
	{
		return ((int)(cp->y + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)];
	}
	return cp->y;
}

float t3f_find_edge_top(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	return cp->y;
}

float t3f_find_edge_bottom(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	int flags = t3f_get_collision_tilemap_flag(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y, T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP);
	T3F_COLLISION_TILE * tp = t3f_get_collision_tile(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y);
	if(flags & T3F_COLLISION_FLAG_SOLID_TOP)
	{
		if(flags & T3F_COLLISION_FLAG_SLOPE_TOP)
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + tp->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)];
		}
		else
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (int)(cp->map.bottom.point[0].y - cp->map.top.point[0].y);
		}
	}
	return cp->y;
}

float t3f_find_edge_left(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	return cp->x;
}

float t3f_find_edge_right(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp)
{
	return cp->x;
}

/* use this function when your sprite is "walking" on solid tiles
   your program is responsible for knowing when the sprite is "walking" */
float t3f_get_tilemap_walk_position(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp, int flags)
{
	int tflags, bflags, aflags;
	T3F_COLLISION_TILE * current_tile, * below_tile, * above_tile;

	if(flags & T3F_COLLISION_FLAG_SOLID_TOP)
	{
		current_tile = t3f_get_collision_tile(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y);
		tflags = (current_tile->flags) & (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP);
//		previous_tile = t3f_get_collision_tile(tmp, cp->ox + cp->map.bottom.point[0].x, cp->oy + cp->map.bottom.point[0].y);
//		pflags = (previous_tile->flags) & (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP);
		below_tile = t3f_get_collision_tile(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y + tmp->tile_height);
		bflags = (below_tile->flags) & (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP);
		above_tile = t3f_get_collision_tile(tmp, cp->x + cp->map.bottom.point[0].x, cp->y + cp->map.bottom.point[0].y - tmp->tile_height);
		aflags = (above_tile->flags) & (T3F_COLLISION_FLAG_SLOPE_TOP | T3F_COLLISION_FLAG_SOLID_TOP);

		/* current tile is solid on top and is sloped, place sprite on top of the slope */
		if((tflags & T3F_COLLISION_FLAG_SOLID_TOP) && (tflags & T3F_COLLISION_FLAG_SLOPE_TOP))
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + current_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
		}
		else if((bflags & T3F_COLLISION_FLAG_SOLID_TOP) && (bflags & T3F_COLLISION_FLAG_SLOPE_TOP))
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y + tmp->tile_height) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + below_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
		}
/*		else if((pflags & T3F_COLLISION_FLAG_SOLID_TOP) && (pflags & T3F_COLLISION_FLAG_SLOPE))
		{
			printf("2\n");
			return ((int)(cp->oy + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + previous_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
		} */

		/* tile above current tile is solid and sloped */
		else if((aflags & T3F_COLLISION_FLAG_SOLID_TOP) && (aflags & T3F_COLLISION_FLAG_SLOPE_TOP) && (bflags & T3F_COLLISION_FLAG_SOLID_TOP))
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y - tmp->tile_height) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + above_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
		}
		else if((aflags & T3F_COLLISION_FLAG_SOLID_TOP) && (aflags & T3F_COLLISION_FLAG_SLOPE_TOP))
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y - tmp->tile_height) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + above_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
		}
		else if((tflags & T3F_COLLISION_FLAG_SOLID_TOP) && !(tflags & T3F_COLLISION_FLAG_SLOPE_TOP))
		{
			return ((int)(cp->y + cp->map.bottom.point[0].y) / tmp->tile_height) * tmp->tile_height - (int)(cp->map.bottom.point[0].y - cp->map.top.point[0].y) - 1.0;
		}
		else if(bflags & T3F_COLLISION_FLAG_SOLID_TOP)
		{
			if(bflags & T3F_COLLISION_FLAG_SLOPE_TOP)
			{
				return ((int)(cp->y + cp->map.bottom.point[0].y + tmp->tile_height) / tmp->tile_height) * tmp->tile_height - (cp->map.bottom.point[0].y - cp->map.top.point[0].y) + below_tile->slope[(int)fmodf(cp->x + cp->map.bottom.point[0].x, tmp->tile_width)] - 1.0;
			}
			else
			{
				return ((int)(cp->y + cp->map.bottom.point[0].y + tmp->tile_height) / tmp->tile_height) * tmp->tile_height - (int)(cp->map.bottom.point[0].y - cp->map.top.point[0].y) - 1.0;
			}
		}
		return cp->y;
	}
	else if(flags & T3F_COLLISION_FLAG_SOLID_BOTTOM)
	{
	}
	else if(flags & T3F_COLLISION_FLAG_SOLID_LEFT)
	{
	}
	else if(flags & T3F_COLLISION_FLAG_SOLID_RIGHT)
	{
	}
	return 0.0;
}
