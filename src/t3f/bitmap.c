#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <stdio.h>
#include "t3f.h"
#include "resource.h"
#include "bitmap.h"

static ALLEGRO_BITMAP * _t3f_bitmap_placeholder = NULL;

bool _t3f_init_bitmap_system(void)
{
	ALLEGRO_STATE old_state;

	t3f_load_resource((void **)&_t3f_bitmap_placeholder, t3f_create_bitmap_resource_handler_proc, "", 1 | (1 << 16), 0, 0);
	if(!_t3f_bitmap_placeholder)
	{
		goto fail;
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(_t3f_bitmap_placeholder);
	al_clear_to_color(t3f_color_white);
	al_restore_state(&old_state);

	return true;

	fail:
	{
		return false;
	}
}

void _t3f_exit_bitmap_system(void)
{
	t3f_destroy_resource(_t3f_bitmap_placeholder);
}

static bool _t3f_pad_bitmap(ALLEGRO_BITMAP ** bp, int flags)
{
	ALLEGRO_BITMAP * padded_bp = NULL;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	int ox = 0;
	int oy = 0;
	int w = al_get_bitmap_width(*bp);
	int h = al_get_bitmap_height(*bp);

	if(flags & T3F_BITMAP_FLAG_PAD_LEFT)
	{
		ox++;
		w++;
	}
	if(flags & T3F_BITMAP_FLAG_PAD_RIGHT)
	{
		w++;
	}
	if(flags & T3F_BITMAP_FLAG_PAD_TOP)
	{
		oy++;
		h++;
	}
	if(flags & T3F_BITMAP_FLAG_PAD_BOTTOM)
	{
		h++;
	}
	padded_bp = al_create_bitmap(w, h);
	if(!padded_bp)
	{
		return false;
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER | ALLEGRO_STATE_TRANSFORM);
	al_identity_transform(&identity);
	al_set_target_bitmap(padded_bp);
	al_use_transform(&identity);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_draw_bitmap(*bp, ox, oy, 0);
	al_restore_state(&old_state);

	al_destroy_bitmap(*bp);
	*bp = padded_bp;

	return true;
}

static float _t3f_bitmap_get_scale(float target_size, float original_size, float total_padding)
{
	return target_size / (original_size - total_padding);
}

ALLEGRO_BITMAP * _t3f_load_allegro_bitmap_padded(const char * fn, int flags)
{
	ALLEGRO_BITMAP * bp = NULL;

	bp = al_load_bitmap(fn);
	if(!bp)
	{
		goto fail;
	}
	if(flags)
	{
		if(!_t3f_pad_bitmap(&bp, flags))
		{
			goto fail;
		}
	}

	return bp;

	fail:
	{
		if(bp)
		{
			al_destroy_bitmap(bp);
		}
		return NULL;
	}
}

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

bool t3f_save_allegro_bitmap_f(ALLEGRO_FILE * fp, ALLEGRO_BITMAP * bp)
{
	ALLEGRO_FILE * tfp = NULL;;
	ALLEGRO_PATH * path = NULL;
	int i, size = 0;
	bool ret = false;

	path = al_clone_path(t3f_data_path);
	if(path)
	{
		al_set_path_filename(path, "t3saver.png");
		if(al_save_bitmap(al_path_cstr(path, '/'), bp))
		{
			tfp = al_fopen(al_path_cstr(path, '/'), "rb");
			if(tfp)
			{
				size = al_fsize(tfp);
				al_fwrite32le(fp, size);
				for(i = 0; i < size; i++)
				{
					al_fputc(fp, al_fgetc(tfp));
				}
				ret = true;
				al_fclose(tfp);
			}
			al_remove_filename(al_path_cstr(path, '/'));
		}
		al_destroy_path(path);
	}
	return ret;
}

/* imports a bitmap from within the source file, reads size, creates mem buffer,
 * loads from that buffer */
ALLEGRO_BITMAP * t3f_load_allegro_bitmap_f(ALLEGRO_FILE * fp, int flags)
{
	ALLEGRO_BITMAP * bp = NULL;
	ALLEGRO_FILE * tfp = NULL;
	int size = al_fread32le(fp);
	char * buffer = NULL;
	if(size > 0)
	{
		buffer = al_malloc(size);
		if(!buffer)
		{
			goto fail;
		}
		al_fread(fp, buffer, size);
		tfp = al_open_memfile(buffer, size, "rb");
		if(!tfp)
		{
			goto fail;
		}
		bp = al_load_bitmap_f(tfp, ".png");
		al_fclose(tfp);
		al_free(buffer);
	}
	if(!bp)
	{
		goto fail;
	}
	if(!_t3f_pad_bitmap(&bp, flags))
	{
		goto fail;
	}
	return bp;

	fail:
	{
		if(bp)
		{
			al_destroy_bitmap(bp);
		}
		if(tfp)
		{
			al_fclose(tfp);
		}
		if(buffer)
		{
			free(buffer);
		}
		return NULL;
	}
}

void t3f_set_bitmap_placeholder(ALLEGRO_BITMAP * bp)
{
	_t3f_bitmap_placeholder = bp;
}

T3F_BITMAP * t3f_create_bitmap(int w, int h, float target_width, float target_height, int flags)
{
	T3F_BITMAP * bp = NULL;

	bp = malloc(sizeof(T3F_BITMAP));
	if(!bp)
	{
		goto fail;
	}
	memset(bp, 0, sizeof(T3F_BITMAP));
	t3f_load_resource((void **)&bp->bitmap, t3f_create_bitmap_resource_handler_proc, "", w | (h << 16), flags, 0);
	if(!bp->bitmap)
	{
		goto fail;
	}
	if(target_width < 0)
	{
		bp->target_width = w;
	}
	else
	{
		bp->target_width = target_width;
	}
	if(target_height < 0)
	{
		bp->target_height = h;
	}
	else
	{
		bp->target_height = target_height;
	}
	bp->flags = flags;
	if(bp->flags & T3F_BITMAP_FLAG_PAD_LEFT)
	{
		bp->pad_left = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_RIGHT)
	{
		bp->pad_right = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_TOP)
	{
		bp->pad_top = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_BOTTOM)
	{
		bp->pad_bottom = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PADDED)
	{
		bp->target_scale_x = _t3f_bitmap_get_scale(target_width, w, 0);
		bp->target_scale_y = _t3f_bitmap_get_scale(target_height, h, 0);
		bp->adjust_left = bp->pad_left * bp->target_scale_x;
		bp->adjust_right = bp->pad_right * bp->target_scale_x;
		bp->adjust_top = bp->pad_top * bp->target_scale_y;
		bp->adjust_bottom = bp->pad_bottom * bp->target_scale_y;
	}

	return bp;

	fail:
	{
		if(bp)
		{
			t3f_destroy_bitmap(bp);
		}
		return NULL;
	}
}

static const char * _get_config_value_fallback(ALLEGRO_CONFIG * cp, const char * section, const char * fallback_section, const char * key)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(!val)
	{
		val = al_get_config_value(cp, fallback_section, key);
	}
	return val;
}

typedef struct
{

	ALLEGRO_PATH * path;
	int flags;
	ALLEGRO_BITMAP ** bitmap;

} _T3F_BITMAP_LOAD_INFO;

static void * _t3f_load_actual_bitmap(void * arg)
{
	_T3F_BITMAP_LOAD_INFO * load_info = (_T3F_BITMAP_LOAD_INFO *)arg;
	void * ret;

	if(arg)
	{
		ret = t3f_load_resource((void **)load_info->bitmap, t3f_bitmap_resource_handler_proc, al_path_cstr(load_info->path, '/'), 0, load_info->flags, 0);
		al_destroy_path(load_info->path);
		free(load_info);
		return ret;
	}
	return _t3f_bitmap_placeholder;
}

static void _t3f_bitmap_setup_variables(T3F_BITMAP * bp)
{
	if(bp->flags & T3F_BITMAP_FLAG_PAD_LEFT)
	{
		bp->pad_left = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_RIGHT)
	{
		bp->pad_right = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_TOP)
	{
		bp->pad_top = 1.0;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_BOTTOM)
	{
		bp->pad_bottom = 1.0;
	}
	if(bp->target_width < 0)
	{
		bp->target_width = al_get_bitmap_width(bp->bitmap) - bp->pad_left - bp->pad_right;
	}
	if(bp->target_height < 0)
	{
		bp->target_height = al_get_bitmap_height(bp->bitmap) - bp->pad_top - bp->pad_bottom;
	}
	bp->target_scale_x = _t3f_bitmap_get_scale(bp->target_width, al_get_bitmap_width(bp->bitmap), bp->pad_left + bp->pad_right);
	bp->target_scale_y = _t3f_bitmap_get_scale(bp->target_height, al_get_bitmap_height(bp->bitmap), bp->pad_top + bp->pad_bottom);
	bp->adjust_left = bp->pad_left * bp->target_scale_x;
	bp->adjust_right = bp->pad_right * bp->target_scale_x;
	bp->adjust_top = bp->pad_top * bp->target_scale_y;
	bp->adjust_bottom = bp->pad_bottom * bp->target_scale_y;
}

T3F_BITMAP * t3f_load_bitmap_f(ALLEGRO_FILE * fp, const char * fn, int flags)
{
	T3F_BITMAP * bp = NULL;

	bp = malloc(sizeof(T3F_BITMAP));
	if(!bp)
	{
		goto fail;
	}
	memset(bp, 0, sizeof(T3F_BITMAP));
	bp->object_loader = t3f_create_object_loader();
	if(!bp->object_loader)
	{
		goto fail;
	}
	bp->target_width = -1;
	bp->target_height = -1;
	bp->flags = flags;

	t3f_load_resource_f((void **)(&bp->bitmap), t3f_bitmap_resource_handler_proc, fp, fn, (flags & T3F_BITMAP_FLAG_DIRECT_LOAD) ? 1 : 0, 0);
	if(!bp->bitmap)
	{
		goto fail;
	}
	_t3f_bitmap_setup_variables(bp);

	return bp;

	fail:
	{
		if(bp)
		{
			t3f_destroy_bitmap(bp);
		}
		return NULL;
	}
}

T3F_BITMAP * t3f_load_bitmap(const char * fn, int flags, bool threaded)
{
	ALLEGRO_CONFIG * cp = NULL;
	const char * val;
	T3F_BITMAP * bp = NULL;
	char * loading_fn = NULL;
	char * loading_section;
	char * base_fn;
	_T3F_BITMAP_LOAD_INFO * load_info = NULL;

	if(!fn)
	{
		goto fail;
	}
	bp = malloc(sizeof(T3F_BITMAP));
	if(!bp)
	{
		goto fail;
	}
	memset(bp, 0, sizeof(T3F_BITMAP));
	bp->object_loader = t3f_create_object_loader();
	if(!bp->object_loader)
	{
		goto fail;
	}
	bp->target_width = -1;
	bp->target_height = -1;
	bp->flags = flags;

	load_info = malloc(sizeof(_T3F_BITMAP_LOAD_INFO));
	if(!load_info)
	{
		goto fail;
	}
	memset(load_info, 0, sizeof(_T3F_BITMAP_LOAD_INFO));
	load_info->flags = flags;
	load_info->bitmap = &bp->loading_bitmap;

	loading_fn = strdup(fn);
	if(!loading_fn)
	{
		goto fail;
	}
	base_fn = strtok(loading_fn, "#");
	loading_section = strtok(NULL, "#");

	/* detect load from INI or image file */
	if(strstr(fn, ".ini"))
	{
		cp = al_load_config_file(base_fn);
		if(!cp)
		{
			goto fail;
		}

		/* get the source image filename from 'section' */
		val = _get_config_value_fallback(cp, loading_section, "Bitmap", "filename");
		if(!val)
		{
			goto fail;
		}

		/* populate 'loading_path' */
		load_info->path = al_create_path(base_fn);
		if(!load_info->path)
		{
			goto fail;
		}
		al_set_path_filename(load_info->path, val);

		/* read bitmap properties */
		val = _get_config_value_fallback(cp, loading_section, "Bitmap", "target_width");
		if(val)
		{
			bp->target_width = atof(val);
		}
		val = _get_config_value_fallback(cp, loading_section, "Bitmap", "target_height");
		if(val)
		{
			bp->target_height = atof(val);
		}

		al_destroy_config(cp);
		cp = NULL;
	}
	else
	{
		load_info->path = al_create_path(base_fn);
		if(!load_info->path)
		{
			goto fail;
		}
	}

	free(loading_fn);
	loading_fn = NULL;
	bp->bitmap = t3f_load_object(bp->object_loader, _t3f_load_actual_bitmap, NULL, load_info, threaded);
	if(!bp->bitmap)
	{
		load_info = NULL;
		goto fail;
	}
	else if(!threaded)
	{
		t3f_remap_resource((void **)&bp->loading_bitmap, (void **)&bp->bitmap);
	}

	_t3f_bitmap_setup_variables(bp);

	return bp;

	fail:
	{
		if(load_info)
		{
			if(load_info->path)
			{
				al_destroy_path(load_info->path);			
			}
			free(load_info);
		}
		if(cp)
		{
			al_destroy_config(cp);
		}
		if(loading_fn)
		{
			free(loading_fn);
		}
		t3f_destroy_bitmap(bp);
		return NULL;
	}
}

T3F_BITMAP * t3f_clone_bitmap(T3F_BITMAP * bp)
{
	T3F_BITMAP * new_bitmap;

	new_bitmap = malloc(sizeof(T3F_BITMAP));
	if(!new_bitmap)
	{
		goto fail;
	}
	memcpy(new_bitmap, bp, sizeof(T3F_BITMAP));
	new_bitmap->object_loader = NULL;
	new_bitmap->loading_bitmap = NULL;
	new_bitmap->bitmap = NULL;
	if(!t3f_clone_resource((void **)(&new_bitmap->bitmap), (void **)&bp->bitmap))
	{
		goto fail;
	}
	return new_bitmap;

	fail:
	{
		if(new_bitmap)
		{
			free(new_bitmap);
		}
		return NULL;
	}
}

T3F_BITMAP * t3f_encapsulate_bitmap(ALLEGRO_BITMAP * bp)
{
	T3F_BITMAP * new_bp;

	new_bp = malloc(sizeof(T3F_BITMAP));
	if(!new_bp)
	{
		goto fail;
	}
	memset(new_bp, 0, sizeof(T3F_BITMAP));
	if(bp)
	{
		new_bp->bitmap = bp;
		new_bp->target_width = al_get_bitmap_width(bp);
		new_bp->target_height = al_get_bitmap_height(bp);
	}

	return new_bp;

	fail:
	{
		t3f_destroy_bitmap(new_bp);
		return NULL;
	}
}

void t3f_reset_bitmap_target_size(T3F_BITMAP * bp)
{
	bp->target_width = al_get_bitmap_width(bp->bitmap);
	bp->target_height = al_get_bitmap_height(bp->bitmap);
}


void t3f_destroy_bitmap(T3F_BITMAP * bp)
{
	if(bp)
	{
		if(bp->object_loader)
		{
			t3f_destroy_object_loader(bp->object_loader);
		}
		if(bp->bitmap && bp->bitmap != _t3f_bitmap_placeholder)
		{
			if(!t3f_destroy_resource(bp->bitmap))
			{
				al_destroy_bitmap(bp->bitmap);
			}
		}
		free(bp);
	}
}

int t3f_get_bitmap_width(T3F_BITMAP * bp)
{
	int w = al_get_bitmap_width(bp->bitmap);

	if(bp->flags & T3F_BITMAP_FLAG_PAD_LEFT)
	{
		w++;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_RIGHT)
	{
		w++;
	}

	return w;
}

int t3f_get_bitmap_height(T3F_BITMAP * bp)
{
	int h = al_get_bitmap_height(bp->bitmap);

	if(bp->flags & T3F_BITMAP_FLAG_PAD_TOP)
	{
		h++;
	}
	if(bp->flags & T3F_BITMAP_FLAG_PAD_BOTTOM)
	{
		h++;
	}

	return h;
}

/* return false if object was ready but the fetched bitmap was NULL */
bool t3f_update_bitmap(T3F_BITMAP * bp)
{
	ALLEGRO_BITMAP * new_bitmap;

	if(t3f_object_ready(bp->object_loader))
	{
		new_bitmap = t3f_fetch_object(bp->object_loader);
		if(new_bitmap)
		{
			bp->bitmap = new_bitmap;
			al_convert_bitmap(bp->bitmap);
			if(bp->flags & T3F_BITMAP_FLAG_PADDED)
			{
				bp->target_scale_x = _t3f_bitmap_get_scale(bp->target_width, al_get_bitmap_width(bp->bitmap), bp->pad_left + bp->pad_right);
				bp->target_scale_y = _t3f_bitmap_get_scale(bp->target_height, al_get_bitmap_height(bp->bitmap), bp->pad_top + bp->pad_bottom);
				bp->adjust_left = bp->pad_left * bp->target_scale_x;
				bp->adjust_right = bp->pad_right * bp->target_scale_x;
				bp->adjust_top = bp->pad_top * bp->target_scale_y;
				bp->adjust_bottom = bp->pad_bottom * bp->target_scale_y;
			}
			t3f_remap_resource((void **)&bp->loading_bitmap, (void **)&bp->bitmap);
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void t3f_draw_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2];

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(x - bp->adjust_left, z);
	obj_x[1] = t3f_project_x(x + bp->target_width + bp->adjust_right, z);
	obj_y[0] = t3f_project_y(y - bp->adjust_top, z);
	obj_y[1] = t3f_project_y(y + bp->target_height + bp->adjust_bottom, z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		/* adjust screen position for integer snap */
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			obj_x[0] = (int)obj_x[0];
			obj_y[0] = (int)obj_y[0];
			obj_x[1] = (int)obj_x[1];
			obj_y[1] = (int)obj_y[1];
			flags &= ~T3F_DRAW_INTEGER_SNAP;
		}
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		al_draw_tinted_scaled_bitmap(bp->bitmap, color, 0, 0, al_get_bitmap_width(bp->bitmap), al_get_bitmap_height(bp->bitmap), obj_x[0], obj_y[0], screen_w, screen_h, flags);
	}
}

void t3f_draw_bitmap_region(T3F_BITMAP * bp, ALLEGRO_COLOR color, float sx, float sy, float sw, float sh, float dx, float dy, float dz, int flags)
{
	t3f_draw_scaled_rotated_bitmap_region(bp, sx, sy, sw, sh, color, bp->target_width / 2, bp->target_height / 2, dx + bp->target_width / 2, dy + bp->target_height / 2, dz, 1.0, 0.0, flags);
}


void t3f_draw_rotated_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2], obj_cx, obj_cy;
	float rx, ry;

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(x - bp->adjust_left, z);
	obj_x[1] = t3f_project_x(x + bp->target_width + bp->adjust_right, z);
	obj_y[0] = t3f_project_y(y - bp->adjust_top, z);
	obj_y[1] = t3f_project_y(y + bp->target_height + bp->adjust_bottom, z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;
	obj_cx = t3f_project_x(x, z);
	obj_cy = t3f_project_y(y, z);

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			obj_x[0] = (int)obj_x[0];
			obj_x[1] = (int)obj_x[1];
			obj_y[0] = (int)obj_y[0];
			obj_y[1] = (int)obj_y[1];
			obj_cx = (int)obj_cx;
			obj_cy = (int)obj_cy;
			flags &= ~T3F_DRAW_INTEGER_SNAP;
		}
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			if((int)screen_w & 1)
			{
				obj_cx += 0.5;
			}
			if((int)screen_h & 1)
			{
				obj_cy += 0.5;
			}
		}
		rx = screen_w / bp->target_width;
		ry = screen_h / bp->target_height;
		al_draw_tinted_scaled_rotated_bitmap(bp->bitmap, color, (cx / bp->target_scale_x) + bp->pad_left, (cy / bp->target_scale_y) + bp->pad_top, obj_cx, obj_cy, rx * bp->target_scale_x, ry * bp->target_scale_y, angle, flags);
	}
}

void t3f_draw_scaled_rotated_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, float scale_x, float scale_y, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2], obj_cx, obj_cy;
	float rx, ry;

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_z[0] = z + t3f_current_view->virtual_width;

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		obj_x[0] = t3f_project_x(x - bp->adjust_left * scale_x, z);
		obj_x[1] = t3f_project_x(x + bp->target_width * scale_x + bp->adjust_right * scale_x, z);
		obj_y[0] = t3f_project_y(y - bp->adjust_top * scale_y, z);
		obj_y[1] = t3f_project_y(y + bp->target_height * scale_y + bp->adjust_bottom * scale_y, z);
		obj_cx = t3f_project_x(x, z);
		obj_cy = t3f_project_y(y, z);
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			obj_x[0] = (int)obj_x[0];
			obj_x[1] = (int)obj_x[1];
			obj_y[0] = (int)obj_y[0];
			obj_y[1] = (int)obj_y[1];
			obj_cx = (int)obj_cx;
			obj_cy = (int)obj_cy;
			screen_w = roundf(screen_w);
			screen_h = roundf(screen_h);
			if((int)screen_w & 1)
			{
				obj_cx += 0.5;
			}
			if((int)screen_h & 1)
			{
				obj_cy += 0.5;
			}
			flags &= ~T3F_DRAW_INTEGER_SNAP;
		}
		rx = screen_w / bp->target_width;
		ry = screen_h / bp->target_height;
		al_draw_tinted_scaled_rotated_bitmap(bp->bitmap, color, (cx / bp->target_scale_x) + bp->pad_left, (cy / bp->target_scale_y) + bp->pad_top, obj_cx, obj_cy, rx * bp->target_scale_x, ry * bp->target_scale_y, angle, flags);
		al_hold_bitmap_drawing(false);
	}
}

void t3f_draw_scaled_bitmap(T3F_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, float w, float h, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2];

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;
	float scale_x, scale_y;

	obj_z[0] = z + t3f_current_view->virtual_width;

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		scale_x = _t3f_bitmap_get_scale(w, al_get_bitmap_width(bp->bitmap), bp->pad_left + bp->pad_right);
		scale_y = _t3f_bitmap_get_scale(h, al_get_bitmap_height(bp->bitmap), bp->pad_top + bp->pad_bottom);
		obj_x[0] = t3f_project_x(x - bp->pad_left * scale_x, z);
		obj_x[1] = t3f_project_x(x + w + bp->pad_right * scale_x, z);
		obj_y[0] = t3f_project_y(y - bp->pad_top * scale_y, z);
		obj_y[1] = t3f_project_y(y + h + bp->pad_bottom * scale_y, z);
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			obj_x[0] = (int)obj_x[0];
			obj_x[1] = (int)obj_x[1];
			obj_y[0] = (int)obj_y[0];
			obj_y[1] = (int)obj_y[1];
			screen_w = roundf(screen_w);
			screen_h = roundf(screen_h);
			flags &= ~T3F_DRAW_INTEGER_SNAP;
		}
		al_draw_tinted_scaled_bitmap(bp->bitmap, color, 0, 0, al_get_bitmap_width(bp->bitmap), al_get_bitmap_height(bp->bitmap), obj_x[0], obj_y[0], screen_w, screen_h, flags);
	}
}

void t3f_draw_scaled_rotated_bitmap_region(T3F_BITMAP * bp, float sx, float sy, float sw, float sh, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float scale, float angle, int flags)
{
	float fsx, fsy, fsw, fsh;
	float screen_w, screen_h;
	float obj_x[2], obj_y[2], obj_z[2], obj_cx, obj_cy;

	obj_z[0] = z + t3f_current_view->virtual_width;

	if(obj_z[0] > 0)
	{
		/* scale the region coordinates so we can pretend the bitmap is target_width/height */
		fsx = sx / bp->target_scale_x + bp->pad_left;
		fsy = sy / bp->target_scale_y + bp->pad_top;
		fsw = sw / bp->target_scale_x;
		fsh = sh / bp->target_scale_y;

		obj_cx = t3f_project_x(x, z);
		obj_cy = t3f_project_y(y, z);
		if(flags & T3F_DRAW_INTEGER_SNAP)
		{
			obj_x[0] = t3f_project_x(x - bp->adjust_left * scale, z);
			obj_x[1] = t3f_project_x(x + bp->target_width * scale + bp->adjust_right * scale, z);
			obj_y[0] = t3f_project_y(y - bp->adjust_top * scale, z);
			obj_y[1] = t3f_project_y(y + bp->target_height * scale + bp->adjust_bottom * scale, z);
			obj_cx = (int)obj_cx;
			obj_cy = (int)obj_cy;
			screen_w = roundf(obj_x[1] - obj_x[0]);
			screen_h = roundf(obj_y[1] - obj_y[0]);
			if((int)screen_w & 1)
			{
				obj_cx += 0.5;
			}
			if((int)screen_h & 1)
			{
				obj_cy += 0.5;
			}
			flags &= ~T3F_DRAW_INTEGER_SNAP;
		}
		al_draw_tinted_scaled_rotated_bitmap_region(bp->bitmap, fsx, fsy, fsw, fsh, color, (cx / bp->target_scale_x) + bp->pad_left, (cy / bp->target_scale_y) + bp->pad_top, obj_cx, obj_cy, scale * bp->target_scale_x, scale * bp->target_scale_y, angle, flags);
	}
}
