#include "t3f.h"
#include "resource.h"

static T3F_ATLAS * t3f_atlas[T3F_MAX_ATLASES] = {NULL};
static int t3f_atlases = 0;

static ALLEGRO_BITMAP * t3f_create_bitmap(int w, int h)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_BITMAP * bp;

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
	bp = al_create_bitmap(w, h);
	al_restore_state(&old_state);
	return bp;
}

/* create an empty atlas of the specified type and size */
T3F_ATLAS * t3f_create_atlas(int w, int h)
{
	T3F_ATLAS * ap;
	ALLEGRO_STATE old_state;

	ap = al_malloc(sizeof(T3F_ATLAS));
	if(!ap)
	{
		return NULL;
	}
	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
	ap->page = t3f_create_bitmap(w, h);
	al_restore_state(&old_state);
	if(!ap->page)
	{
		al_free(ap);
		return NULL;
	}
	ap->x = 1; // start at 1 so we get consistency with filtered bitmaps
	ap->y = 1;
	ap->width = w;
	ap->height = h;
	ap->line_height = 0;
	ap->bitmaps = 0;

	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
	al_set_target_bitmap(ap->page);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	al_restore_state(&old_state);

	t3f_atlas[t3f_atlases] = ap;
	t3f_atlases++;
	return ap;
}

/* destroy the atlas */
void t3f_destroy_atlas(T3F_ATLAS * ap)
{
	int i, j;

	al_destroy_bitmap(ap->page);
	al_free(ap);
	for(i = 0; i < t3f_atlases; i++)
	{
		if(t3f_atlas[i] == ap)
		{
			for(j = i; j < t3f_atlases - 1; j++)
			{
				t3f_atlas[j] = t3f_atlas[j + 1];
			}
			t3f_atlases--;
			break;
		}
	}
}

#ifndef ALLEGRO_ANDROID

	static void t3f_actually_put_bitmap_on_atlas_fbo(T3F_ATLAS * ap, ALLEGRO_BITMAP * bp, int type)
	{
		switch(type)
		{
			case T3F_ATLAS_TILE:
			{
				/* need to extend edges of tiles so they don't have soft edges */
				al_draw_bitmap(bp, ap->x, ap->y, 0);
				al_draw_bitmap(bp, ap->x + 2, ap->y, 0);
				al_draw_bitmap(bp, ap->x, ap->y + 2, 0);
				al_draw_bitmap(bp, ap->x + 2, ap->y + 2, 0);
				al_draw_bitmap(bp, ap->x + 1, ap->y, 0);
				al_draw_bitmap(bp, ap->x + 1, ap->y + 2, 0);
				al_draw_bitmap(bp, ap->x, ap->y + 1, 0);
				al_draw_bitmap(bp, ap->x + 2, ap->y + 1, 0);
				al_draw_bitmap(bp, ap->x + 1, ap->y + 1, 0);
				break;
			}
			case T3F_ATLAS_SPRITE:
			{
				al_draw_bitmap(bp, ap->x + 1, ap->y + 1, 0);
				break;
			}
		}
	}

#endif

#ifdef ALLEGRO_ANDROID

	static void t3f_pixel_copy_bitmap(ALLEGRO_BITMAP * src, ALLEGRO_BITMAP * dest, int x, int y)
	{
		int i, j;
		ALLEGRO_STATE old_state;

		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		al_lock_bitmap_region(dest, x, y, al_get_bitmap_width(src), al_get_bitmap_height(src), ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_WRITEONLY);
		al_lock_bitmap(src, ALLEGRO_PIXEL_FORMAT_RGBA_8888, ALLEGRO_LOCK_READONLY);
		al_set_target_bitmap(dest);
		for(i = 0; i < al_get_bitmap_height(src); i++)
		{
			for(j = 0; j < al_get_bitmap_width(src); j++)
			{
				al_put_pixel(x + j, y + i, al_get_pixel(src, j, i));
			}
		}
		al_unlock_bitmap(src);
		al_unlock_bitmap(dest);
		al_restore_state(&old_state);
	}

	static void t3f_actually_put_bitmap_on_atlas_pixel_copy(T3F_ATLAS * ap, ALLEGRO_BITMAP * bp, int type)
	{
		switch(type)
		{
			case T3F_ATLAS_TILE:
			{
				/* need to extend edges of tiles so they don't have soft edges */
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x, ap->y);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 2, ap->y);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x, ap->y + 2);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 2, ap->y + 2);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 1, ap->y);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 1, ap->y + 2);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x, ap->y + 1);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 2, ap->y + 1);
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 1, ap->y + 1);
				break;
			}
			case T3F_ATLAS_SPRITE:
			{
				t3f_pixel_copy_bitmap(bp, ap->page, ap->x + 1, ap->y + 1);
				break;
			}
		}
	}

#endif

ALLEGRO_BITMAP * t3f_put_bitmap_on_atlas(T3F_ATLAS * ap, ALLEGRO_BITMAP ** bp, int type)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_BITMAP * retbp = NULL;
	ALLEGRO_TRANSFORM identity_transform;

	if(ap->y >= al_get_bitmap_height(ap->page))
	{
		return NULL;
	}

	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER | ALLEGRO_STATE_TRANSFORM);
	al_set_target_bitmap(ap->page);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_identity_transform(&identity_transform);
	al_use_transform(&identity_transform);

	/* move position if we need to */
	if(ap->x + al_get_bitmap_width(*bp) + 2 >= al_get_bitmap_width(ap->page))
	{
		ap->x = 1; // start at 1 so we get consistency with filtered bitmaps
		ap->y += ap->line_height;

		/* if it still doesn't fit, fail */
		if(ap->y  + al_get_bitmap_height(*bp) + 2 >= al_get_bitmap_height(ap->page))
		{
			al_restore_state(&old_state);
			return NULL;
		}
	}
	#ifdef ALLEGRO_ANDROID
		t3f_actually_put_bitmap_on_atlas_pixel_copy(ap, *bp, type);
	#else
		t3f_actually_put_bitmap_on_atlas_fbo(ap, *bp, type);
	#endif
	retbp = al_create_sub_bitmap(ap->page, ap->x + 1, ap->y + 1, al_get_bitmap_width(*bp), al_get_bitmap_height(*bp));
	ap->x += al_get_bitmap_width(*bp) + 2;
	if(al_get_bitmap_height(*bp) + 2 > ap->line_height)
	{
		ap->line_height = al_get_bitmap_height(*bp) + 2;
	}
	al_restore_state(&old_state);
	return retbp;
}

/* fix for when you have exceeded the size of the sprite sheet */
bool t3f_add_bitmap_to_atlas(T3F_ATLAS * ap, ALLEGRO_BITMAP ** bp, int type)
{
	ALLEGRO_BITMAP * retbp = NULL;

	if(!bp || !*bp)
	{
		return false;
	}
	retbp = t3f_put_bitmap_on_atlas(ap, bp, type);
	if(retbp)
	{
		al_destroy_bitmap(*bp);
		*bp = retbp;
		ap->bitmap[ap->bitmaps] = bp;
		ap->bitmap_type[ap->bitmaps] = type;
		ap->bitmaps++;
	}

	return true;
}

void t3f_unload_atlases(void)
{
	int i;

	for(i = 0; i < t3f_atlases; i++)
	{
		al_destroy_bitmap(t3f_atlas[i]->page);
		t3f_atlas[i]->page = NULL;
	}
}

bool t3f_rebuild_atlases(void)
{
	ALLEGRO_STATE old_state;
	int i, j;
	ALLEGRO_BITMAP * bp;

	for(i = 0; i < t3f_atlases; i++)
	{
		al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
		al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
		t3f_atlas[i]->page = t3f_create_bitmap(t3f_atlas[i]->width, t3f_atlas[i]->height);
		al_restore_state(&old_state);
		if(!t3f_atlas[i]->page)
		{
			return false;
		}
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
		al_set_target_bitmap(t3f_atlas[i]->page);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
		al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
		al_restore_state(&old_state);
		t3f_atlas[i]->x = 1; // start at 1 so we get consistency with filtered bitmaps
		t3f_atlas[i]->y = 1;
		t3f_atlas[i]->line_height = 0;
		for(j = 0; j < t3f_atlas[i]->bitmaps; j++)
		{
			bp = t3f_put_bitmap_on_atlas(t3f_atlas[i], t3f_atlas[i]->bitmap[j], t3f_atlas[i]->bitmap_type[j]);
			if(bp)
			{
				al_destroy_bitmap(*t3f_atlas[i]->bitmap[j]);
				*t3f_atlas[i]->bitmap[j] = bp;
			}
		}
	}
	return true;
}
