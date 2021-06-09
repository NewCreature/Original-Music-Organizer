#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdio.h>
#include "t3f.h"

ALLEGRO_COLOR interpolate(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac)
{
	return al_map_rgba_f(c1.r + frac * (c2.r - c1.r), c1.g + frac * (c2.g - c1.g), c1.b + frac * (c2.b - c1.b), c1.a + frac * (c2.a - c1.a));
}

static bool resize_bitmap_hq(ALLEGRO_BITMAP ** bp, int w, int h, int flags)
{
	ALLEGRO_BITMAP * rbp = NULL;
	int start_w = al_get_bitmap_width(*bp);
	int start_h = al_get_bitmap_height(*bp);
	int x, y;
	float pixx, pixx_f, pixy, pixy_f;
	ALLEGRO_COLOR a, b, c, d, ab, cd, result;
	ALLEGRO_STATE old_state;

	/* scale with software filtering */
	rbp = al_create_bitmap(w, h);
	if(!rbp)
	{
		printf("failed to create return bitmap\n");
		return false;
	}
	al_lock_bitmap(rbp, ALLEGRO_LOCK_READWRITE, ALLEGRO_PIXEL_FORMAT_ANY);
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(rbp);
	for(y = 0; y < h; y++)
	{
		pixy = ((float)y / h) * ((float)start_h - 1);
		pixy_f = floor(pixy);
		for(x = 0; x < w; x++)
		{
			pixx = ((float)x / w) * ((float)start_w - 1);
			pixx_f = floor(pixx);

			a = al_get_pixel(*bp, pixx_f, pixy_f);
			b = al_get_pixel(*bp, pixx_f + 1, pixy_f);
			c = al_get_pixel(*bp, pixx_f, pixy_f + 1);
			d = al_get_pixel(*bp, pixx_f + 1, pixy_f + 1);

			ab = interpolate(a, b, pixx - pixx_f);
			cd = interpolate(c, d, pixx - pixx_f);
			result = interpolate(ab, cd, pixy - pixy_f);

			al_put_pixel(x, y, result);
		}
	}
	al_unlock_bitmap(rbp);
	al_restore_state(&old_state);
	t3f_destroy_resource(*bp);
	*bp = rbp;
	return true;
}

static bool resize_bitmap_lq(ALLEGRO_BITMAP ** bp, int w, int h, int flags)
{
	ALLEGRO_TRANSFORM identity;
	ALLEGRO_BITMAP * rbp;
	ALLEGRO_STATE old_state;
	bool ret = false;

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS | ALLEGRO_STATE_TRANSFORM | ALLEGRO_STATE_TARGET_BITMAP);
	al_set_new_bitmap_flags(flags);
	rbp = al_create_bitmap(w, h);
	if(rbp)
	{
		al_set_target_bitmap(rbp);
		al_identity_transform(&identity);
		al_use_transform(&identity);
		al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
		al_draw_scaled_bitmap(*bp, 0, 0, al_get_bitmap_width(*bp), al_get_bitmap_height(*bp), 0, 0, w, h, 0);
		t3f_destroy_resource(*bp);
		*bp = rbp;
	}
	al_restore_state(&old_state);

	return ret;
}

bool t3f_resize_bitmap(ALLEGRO_BITMAP ** bp, int w, int h, bool hq, int flags)
{
	int start_w = al_get_bitmap_width(*bp);
	int start_h = al_get_bitmap_height(*bp);

	/* don't resize if size is already correct */
	if(w == start_w && h == start_h)
	{
		return true;
	}

	if(hq)
	{
		return resize_bitmap_hq(bp, w, h, flags);
	}
	return resize_bitmap_lq(bp, w, h, flags);
}

/* function to squeeze a large bitmap to fit within the GPU's maximum texture size */
bool t3f_squeeze_bitmap(ALLEGRO_BITMAP ** bp, int * ow, int * oh)
{
	int start_w = al_get_bitmap_width(*bp);
	int start_h = al_get_bitmap_height(*bp);
	int width = al_get_display_option(t3f_display, ALLEGRO_MAX_BITMAP_SIZE);
	int height = width;

	if(start_w < width)
	{
		width = start_w;
	}
	if(start_h < height)
	{
		height = start_h;
	}
	/* store original bitmap size if pointers passed */
	if(ow)
	{
		*ow = start_w;
	}
	if(oh)
	{
		*oh = start_h;
	}

	/* return original bitmap if it already fits */
	if(start_w <= width && start_h <= height)
	{
		return true;
	}
	return t3f_resize_bitmap(bp, width, width, true, 0);
}
