#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include "t3f.h"
#include "animation.h"
#include "bitmap.h"
#include "draw.h"
#include "view.h"
#include "resource.h"
#include "file.h"

static char ani_header[12] = {'O', 'C', 'D', 'A', 'S', 0};

/* memory management */
T3F_ANIMATION * t3f_create_animation(void)
{
	T3F_ANIMATION * ap;

	ap = al_malloc(sizeof(T3F_ANIMATION));
	if(ap)
	{
		ap->bitmaps = al_malloc(sizeof(T3F_ANIMATION_BITMAPS));
		if(!ap->bitmaps)
		{
			free(ap);
			return NULL;
		}
		ap->bitmaps->count = 0;
		ap->frames = 0;
		ap->frame_list_total = 0;
		ap->flags = 0;
	}
	return ap;
}

T3F_ANIMATION * t3f_clone_animation(T3F_ANIMATION * ap)
{
	int i;
	T3F_ANIMATION * clone = NULL;

	clone = t3f_create_animation();
	if(clone)
	{
		if(ap->flags & T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS)
		{
			clone->bitmaps = ap->bitmaps;
		}
		else
		{
			for(i = 0; i < ap->bitmaps->count; i++)
			{
				t3f_clone_resource((void **)&(clone->bitmaps->bitmap[i]), ap->bitmaps->bitmap[i]);
				if(!clone->bitmaps->bitmap[i])
				{
					printf("failed to clone bitmap\n");
					return NULL;
				}
			}
			clone->bitmaps->count = ap->bitmaps->count;
		}
		for(i = 0; i < ap->frames; i++)
		{
			if(!t3f_animation_add_frame(clone, ap->frame[i]->bitmap, ap->frame[i]->x, ap->frame[i]->y, ap->frame[i]->z, ap->frame[i]->width, ap->frame[i]->height, ap->frame[i]->angle, ap->frame[i]->ticks, ap->frame[i]->flags))
			{
				return NULL;
			}
		}
		clone->flags = ap->flags;
	}
	return clone;
}

void t3f_destroy_animation(T3F_ANIMATION * ap)
{
	int i;

	for(i = 0; i < ap->frames; i++)
	{
		al_free(ap->frame[i]);
	}
	if(!(ap->flags & T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS))
	{
		for(i = 0; i < ap->bitmaps->count; i++)
		{
			/* Attempt to destroy resource. If it fails, that means the user
			 * probably constructed the animation manually, so destroy the bitmap
			 * directly. */
			if(!t3f_destroy_resource(ap->bitmaps->bitmap[i]))
			{
				al_destroy_bitmap(ap->bitmaps->bitmap[i]);
			}
		}
		al_free(ap->bitmaps);
	}
	al_free(ap);
}

/* see if header matches and return the version number, -1 is no match */
static int check_header(char * h)
{
	int i;

	for(i = 0; i < 11; i++)
	{
		if(h[i] != ani_header[i])
		{
			return -1;
		}
	}
	return h[11];
}

T3F_ANIMATION * t3f_load_animation_f(ALLEGRO_FILE * fp, const char * fn)
{
	T3F_ANIMATION * ap;
	int i;
	char header[12]	= {0};
	int ver;
	int fpos = 0;
	ALLEGRO_STATE old_state;
	ALLEGRO_BITMAP * bp;

	al_fread(fp, header, 12);
	ver = check_header(header);
	if(ver < 0)
	{
		return NULL;
	}

	ap = t3f_create_animation();
	if(ap)
	{
		switch(ver)
		{
			case 0:
			{
				ap->bitmaps->count = al_fread16le(fp);
				for(i = 0; i < ap->bitmaps->count; i++)
				{
					ap->bitmaps->bitmap[i] = t3f_load_resource_f((void **)(&ap->bitmaps->bitmap[i]), t3f_bitmap_resource_handler_proc, fp, fn, 1, 0);
				}
				ap->frames = al_fread16le(fp);
				for(i = 0; i < ap->frames; i++)
				{
					ap->frame[i] = al_malloc(sizeof(T3F_ANIMATION_FRAME));
					if(!ap->frame[i])
					{
						return NULL;
					}
					ap->frame[i]->bitmap = al_fread16le(fp);
					ap->frame[i]->x = t3f_fread_float(fp);
					ap->frame[i]->y = t3f_fread_float(fp);
					ap->frame[i]->z = t3f_fread_float(fp);
					ap->frame[i]->width = t3f_fread_float(fp);
					ap->frame[i]->height = t3f_fread_float(fp);
					ap->frame[i]->angle = t3f_fread_float(fp);
					ap->frame[i]->ticks = al_fread32le(fp);
					ap->frame[i]->flags = al_fread32le(fp);
				}
				ap->flags = al_fread32le(fp);
				break;
			}
			case 1:
			{
				ap->bitmaps->count = al_fread16le(fp);
				for(i = 0; i < ap->bitmaps->count; i++)
				{
					fpos = al_ftell(fp);
					ap->bitmaps->bitmap[i] = t3f_load_resource_f((void **)(&ap->bitmaps->bitmap[i]), t3f_bitmap_resource_handler_proc, fp, fn, 0, 0);
					if(!ap->bitmaps->bitmap[i])
					{
						al_fseek(fp, fpos, ALLEGRO_SEEK_SET);
						al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
						al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
						bp = t3f_load_bitmap_f(fp);
						al_restore_state(&old_state);
						if(bp)
						{
							ap->bitmaps->bitmap[i] = bp;
							t3f_squeeze_bitmap(&ap->bitmaps->bitmap[i], NULL, NULL);
						}
					}
					else if(al_get_bitmap_flags(ap->bitmaps->bitmap[i]) & ALLEGRO_MEMORY_BITMAP)
					{
						t3f_squeeze_bitmap(&ap->bitmaps->bitmap[i], NULL, NULL);
					}
					if(!ap->bitmaps->bitmap[i])
					{
						return NULL;
					}
				}
				ap->frames = al_fread16le(fp);
				for(i = 0; i < ap->frames; i++)
				{
					ap->frame[i] = al_malloc(sizeof(T3F_ANIMATION_FRAME));
					if(!ap->frame[i])
					{
						return NULL;
					}
					ap->frame[i]->bitmap = al_fread16le(fp);
					ap->frame[i]->x = t3f_fread_float(fp);
					ap->frame[i]->y = t3f_fread_float(fp);
					ap->frame[i]->z = t3f_fread_float(fp);
					ap->frame[i]->width = t3f_fread_float(fp);
					ap->frame[i]->height = t3f_fread_float(fp);
					ap->frame[i]->angle = t3f_fread_float(fp);
					ap->frame[i]->ticks = al_fread32le(fp);
					ap->frame[i]->flags = al_fread32le(fp);
				}
				ap->flags = al_fread32le(fp);
				break;
			}
		}
	}
	t3f_animation_build_frame_list(ap);
	return ap;
}

T3F_ANIMATION * t3f_load_animation(const char * fn)
{
	T3F_ANIMATION * ap;
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	ap = t3f_load_animation_f(fp, fn);
	al_fclose(fp);
	return ap;
}

T3F_ANIMATION * t3f_load_animation_from_bitmap(const char * fn)
{
	T3F_ANIMATION * ap;

	ap = t3f_create_animation();
	if(!ap)
	{
		return NULL;
	}
	ap->bitmaps->bitmap[0] = t3f_load_resource((void **)(&(ap->bitmaps->bitmap[0])), t3f_bitmap_resource_handler_proc, fn, 0, 0, 0);
	if(!ap->bitmaps->bitmap[0])
	{
		t3f_destroy_animation(ap);
		return NULL;
	}
	ap->bitmaps->count = 1;
	t3f_animation_add_frame(ap, 0, 0.0, 0.0, 0.0, al_get_bitmap_width(ap->bitmaps->bitmap[0]), al_get_bitmap_height(ap->bitmaps->bitmap[0]), 0.0, 1, 0);
	return ap;
}

int t3f_save_animation_f(T3F_ANIMATION * ap, ALLEGRO_FILE * fp)
{
	int i;

	ani_header[11] = T3F_ANIMATION_REVISION; // put the version number in
	al_fwrite(fp, ani_header, 12);
	al_fwrite16le(fp, ap->bitmaps->count);
	for(i = 0; i < ap->bitmaps->count; i++)
	{
		if(!t3f_save_bitmap_f(fp, ap->bitmaps->bitmap[i]))
		{
			printf("failed to save bitmap\n");
			return 0;
		}
	}
	al_fwrite16le(fp, ap->frames);
	for(i = 0; i < ap->frames; i++)
	{
		al_fwrite16le(fp, ap->frame[i]->bitmap);
		t3f_fwrite_float(fp, ap->frame[i]->x);
		t3f_fwrite_float(fp, ap->frame[i]->y);
		t3f_fwrite_float(fp, ap->frame[i]->z);
		t3f_fwrite_float(fp, ap->frame[i]->width);
		t3f_fwrite_float(fp, ap->frame[i]->height);
		t3f_fwrite_float(fp, ap->frame[i]->angle);
		al_fwrite32le(fp, ap->frame[i]->ticks);
		al_fwrite32le(fp, ap->frame[i]->flags);
	}
	al_fwrite32le(fp, ap->flags);
	return 1;
}

int t3f_save_animation(T3F_ANIMATION * ap, const char * fn)
{
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return 0;
	}
	t3f_save_animation_f(ap, fp);
	al_fclose(fp);
	return 1;
}

/* utilities */
int t3f_animation_add_bitmap(T3F_ANIMATION * ap, ALLEGRO_BITMAP * bp)
{
	ap->bitmaps->bitmap[ap->bitmaps->count] = bp;
	ap->bitmaps->count++;
	return 1;
}

int t3f_animation_delete_bitmap(T3F_ANIMATION * ap, int bitmap)
{
	int i;

	if(bitmap < ap->bitmaps->count)
	{
		if(!t3f_destroy_resource(ap->bitmaps->bitmap[bitmap]))
		{
			al_destroy_bitmap(ap->bitmaps->bitmap[bitmap]);
		}
	}
	else
	{
		return 0;
	}
	for(i = bitmap; i < ap->bitmaps->count - 1; i++)
	{
		ap->bitmaps->bitmap[i] = ap->bitmaps->bitmap[i + 1];
	}
	ap->bitmaps->count--;
	return 1;
}

int t3f_animation_add_frame(T3F_ANIMATION * ap, int bitmap, float x, float y, float z, float w, float h, float angle, int ticks, int flags)
{
	ap->frame[ap->frames] = al_malloc(sizeof(T3F_ANIMATION_FRAME));
	if(ap->frame[ap->frames])
	{
		ap->frame[ap->frames]->bitmap = bitmap;
		ap->frame[ap->frames]->x = x;
		ap->frame[ap->frames]->y = y;
		ap->frame[ap->frames]->z = z;
		if(w < 0.0)
		{
			ap->frame[ap->frames]->width = al_get_bitmap_width(ap->bitmaps->bitmap[bitmap]);
		}
		else
		{
			ap->frame[ap->frames]->width = w;
		}
		if(h < 0.0)
		{
			ap->frame[ap->frames]->height = al_get_bitmap_height(ap->bitmaps->bitmap[bitmap]);
		}
		else
		{
			ap->frame[ap->frames]->height = h;
		}
		ap->frame[ap->frames]->angle = angle;
		ap->frame[ap->frames]->ticks = ticks;
		ap->frame[ap->frames]->flags = flags;
		ap->frames++;
		t3f_animation_build_frame_list(ap);
		return 1;
	}
	return 0;
}

int t3f_animation_delete_frame(T3F_ANIMATION * ap, int frame)
{
	int i;

	if(frame < ap->frames)
	{
		al_free(ap->frame[frame]);
	}
	else
	{
		return 0;
	}
	for(i = frame; i < ap->frames - 1; i++)
	{
		ap->frame[i] = ap->frame[i + 1];
	}
	ap->frames--;
	t3f_animation_build_frame_list(ap);
	return 1;
}

int t3f_animation_build_frame_list(T3F_ANIMATION * ap)
{
	int i, j;

	ap->frame_list_total = 0;
	for(i = 0; i < ap->frames; i++)
	{
		for(j = 0; j < ap->frame[i]->ticks; j++)
		{
			ap->frame_list[ap->frame_list_total] = i;
			ap->frame_list_total++;
		}
	}
	return 1;
}

bool t3f_add_animation_to_atlas(T3F_ATLAS * sap, T3F_ANIMATION * ap, int type)
{
	int i, failed = 0;

	/* add bitmaps to sprite sheet */
	for(i = 0; i < ap->bitmaps->count; i++)
	{
		if(!t3f_add_bitmap_to_atlas(sap, &ap->bitmaps->bitmap[i], type))
		{
			failed = 1;
		}
	}

	/* if all bitmaps added successfully, point ap bitmaps to new ones */
	if(failed)
	{
		return false;
	}
	return true;
}

/* in-game */
ALLEGRO_BITMAP * t3f_animation_get_bitmap(T3F_ANIMATION * ap, int tick)
{
	if(tick >= ap->frame_list_total && ap->flags & T3F_ANIMATION_FLAG_ONCE)
	{
		return ap->bitmaps->bitmap[ap->frame[ap->frame_list[ap->frame_list_total - 1]]->bitmap];
	}
	else
	{
		return ap->bitmaps->bitmap[ap->frame[ap->frame_list[tick % ap->frame_list_total]]->bitmap];
	}
}

T3F_ANIMATION_FRAME * t3f_animation_get_frame(T3F_ANIMATION * ap, int tick)
{
	if(ap->frames <= 0)
	{
		return NULL;
	}
	if(tick >= ap->frame_list_total && ap->flags & T3F_ANIMATION_FLAG_ONCE)
	{
		return ap->frame[ap->frame_list[ap->frame_list_total - 1]];
	}
	else
	{
		return ap->frame[ap->frame_list[tick % ap->frame_list_total]];
	}
}

static void handle_vh_flip(T3F_ANIMATION_FRAME * base_fp, T3F_ANIMATION_FRAME * fp, int flags, float * fox, float * foy, int * dflags)
{
	bool vflip = false;
	bool hflip = false;

	if(fp->flags & ALLEGRO_FLIP_HORIZONTAL && !(flags & ALLEGRO_FLIP_HORIZONTAL))
	{
		hflip = true;
	}
	else if(flags & ALLEGRO_FLIP_HORIZONTAL && !(fp->flags & ALLEGRO_FLIP_HORIZONTAL))
	{
		hflip = true;
	}
	if(fp->flags & ALLEGRO_FLIP_VERTICAL && !(flags & ALLEGRO_FLIP_VERTICAL))
	{
		vflip = true;
	}
	else if(flags & ALLEGRO_FLIP_VERTICAL && !(fp->flags & ALLEGRO_FLIP_VERTICAL))
	{
		vflip = true;
	}
	if(hflip)
	{
		*dflags |= ALLEGRO_FLIP_HORIZONTAL;
		*fox = -((fp->x + fp->width) - (base_fp->x + base_fp->width)) - (fp->x - base_fp->x);
	}
	if(vflip)
	{
		*dflags |= ALLEGRO_FLIP_VERTICAL;
		*foy = -((fp->y + fp->height) - (base_fp->y + base_fp->height)) - (fp->y - base_fp->y);
	}
}

void t3f_draw_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, int flags)
{
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = 0;

	if(fp)
	{
		handle_vh_flip(ap->frame[0], fp, flags, &fox, &foy, &dflags);
		t3f_draw_scaled_bitmap(ap->bitmaps->bitmap[fp->bitmap], color, x + fp->x + fox, y + fp->y + foy, z + fp->z, fp->width, fp->height, dflags);
	}
}

void t3f_draw_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, float scale, int flags)
{
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	int dflags = 0;
	float fox = 0.0, foy = 0.0;

	if(fp)
	{
		handle_vh_flip(ap->frame[0], fp, flags, &fox, &foy, &dflags);
		t3f_draw_scaled_bitmap(ap->bitmaps->bitmap[fp->bitmap], color, x + (fp->x + fox) * scale, y + (fp->y + foy) * scale, z + fp->z, fp->width * scale, fp->height * scale, dflags);
	}
}

void t3f_draw_rotated_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, int flags)
{
	float scale_x, scale_y;
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = 0;
	if(fp)
	{
		handle_vh_flip(ap->frame[0], fp, flags, &fox, &foy, &dflags);
		scale_x = fp->width / al_get_bitmap_width(ap->bitmaps->bitmap[fp->bitmap]);
		scale_y = fp->height / al_get_bitmap_height(ap->bitmaps->bitmap[fp->bitmap]);
		t3f_draw_scaled_rotated_bitmap(ap->bitmaps->bitmap[fp->bitmap], color, cx / scale_x - fp->x, cy / scale_y - fp->y, x + fox, y + foy, z + fp->z, angle, scale_x, scale_y, dflags);
	}
}

void t3f_draw_rotated_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, float scale, int flags)
{
	float scale_x, scale_y;
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = 0;
	if(fp)
	{
		handle_vh_flip(ap->frame[0], fp, flags, &fox, &foy, &dflags);
		scale_x = fp->width / al_get_bitmap_width(ap->bitmaps->bitmap[fp->bitmap]);
		scale_y = fp->height / al_get_bitmap_height(ap->bitmaps->bitmap[fp->bitmap]);
		t3f_draw_scaled_rotated_bitmap(ap->bitmaps->bitmap[fp->bitmap], color, cx / scale_x - fp->x, cy / scale_y - fp->y, x + fox, y + foy, z + fp->z, angle, scale * scale_x, scale * scale_y, dflags);
	}
}

void t3f_draw_scaled_rotated_animation_region(T3F_ANIMATION * ap, float sx, float sy, float sw, float sh, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float scale, float angle, int flags)
{
	float sscale_x, sscale_y;
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	float fsx, fsy, fsw, fsh;
	float pw;
	if(fp)
	{
		pw = t3f_project_x(1.0, z) - t3f_project_x(0.0, z);
		sscale_x = fp->width / (float)al_get_bitmap_width(ap->bitmaps->bitmap[fp->bitmap]);
		sscale_y = fp->height / (float)al_get_bitmap_height(ap->bitmaps->bitmap[fp->bitmap]);
		fsx = sx / sscale_x;
		fsy = sy / sscale_y;
		fsw = sw / sscale_x;
		fsh = sh / sscale_y;
		al_draw_tinted_scaled_rotated_bitmap_region(ap->bitmaps->bitmap[fp->bitmap], fsx, fsy, fsw, fsh, color, cx / sscale_x - fp->x, cy / sscale_y - fp->y, t3f_project_x(x + fox, z), t3f_project_y(y + foy, z), scale * sscale_x * pw, scale * sscale_y * pw, angle, flags);
	}
}
