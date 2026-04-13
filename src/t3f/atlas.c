#include "t3f.h"
#include "resource.h"

static T3F_ATLAS * t3f_atlas[T3F_MAX_ATLASES] = {NULL};
static int t3f_atlases = 0;

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
	al_set_new_bitmap_flags(al_get_new_bitmap_flags() | ALLEGRO_NO_PRESERVE_TEXTURE);
	ap->page = al_create_bitmap(w, h);
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

static void t3f_actually_put_bitmap_on_atlas_fbo(T3F_ATLAS * ap, ALLEGRO_BITMAP * bp, int flags)
{
	if(flags & T3F_ATLAS_TILE_CONNECT_LEFT)
	{
		if(flags & T3F_ATLAS_TILE_CONNECT_TOP)
		{
			al_draw_bitmap(bp, ap->x, ap->y, 0);
		}
		if(flags & T3F_ATLAS_TILE_CONNECT_BOTTOM)
		{
			al_draw_bitmap(bp, ap->x, ap->y + 2, 0);
		}
		al_draw_bitmap(bp, ap->x, ap->y + 1, 0);
	}
	if(flags & T3F_ATLAS_TILE_CONNECT_RIGHT)
	{
		if(flags & T3F_ATLAS_TILE_CONNECT_TOP)
		{
			al_draw_bitmap(bp, ap->x + 2, ap->y, 0);
		}
		if(flags & T3F_ATLAS_TILE_CONNECT_BOTTOM)
		{
			al_draw_bitmap(bp, ap->x + 2, ap->y + 2, 0);
		}
		al_draw_bitmap(bp, ap->x + 2, ap->y + 1, 0);
	}
	if(flags & T3F_ATLAS_TILE_CONNECT_TOP)
	{
		al_draw_bitmap(bp, ap->x + 1, ap->y, 0);
	}
	if(flags & T3F_ATLAS_TILE_CONNECT_BOTTOM)
	{
		al_draw_bitmap(bp, ap->x + 1, ap->y + 2, 0);
	}
	al_draw_bitmap(bp, ap->x + 1, ap->y + 1, 0);
}

ALLEGRO_BITMAP * t3f_put_bitmap_on_atlas(T3F_ATLAS * ap, ALLEGRO_BITMAP ** bp, int flags)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_BITMAP * retbp = NULL;
	ALLEGRO_TRANSFORM identity_transform;

	if(ap->y + al_get_bitmap_height(*bp) + 2 >= al_get_bitmap_height(ap->page))
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
		if(ap->y + al_get_bitmap_height(*bp) + 2 >= al_get_bitmap_height(ap->page))
		{
			al_restore_state(&old_state);
			return NULL;
		}
	}
	t3f_actually_put_bitmap_on_atlas_fbo(ap, *bp, flags);
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
bool t3f_add_bitmap_to_atlas(T3F_ATLAS * ap, ALLEGRO_BITMAP ** bp, int flags)
{
	ALLEGRO_BITMAP * retbp = NULL;

	if(!bp || !*bp)
	{
		return false;
	}
	retbp = t3f_put_bitmap_on_atlas(ap, bp, flags);
	if(retbp)
	{
		al_destroy_bitmap(*bp);
		*bp = retbp;
		ap->bitmap[ap->bitmaps] = bp;
		ap->bitmap_flags[ap->bitmaps] = flags;
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
		t3f_atlas[i]->page = al_create_bitmap(t3f_atlas[i]->width, t3f_atlas[i]->height);
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
			bp = t3f_put_bitmap_on_atlas(t3f_atlas[i], t3f_atlas[i]->bitmap[j], t3f_atlas[i]->bitmap_flags[j]);
			if(bp)
			{
				al_destroy_bitmap(*t3f_atlas[i]->bitmap[j]);
				*t3f_atlas[i]->bitmap[j] = bp;
			}
		}
	}
	return true;
}
