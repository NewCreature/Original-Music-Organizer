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
static T3F_BITMAP * _t3f_animation_placeholder = NULL;

bool _t3f_init_animation_system(void)
{
	ALLEGRO_STATE old_state;

	_t3f_animation_placeholder = t3f_create_bitmap(1, 1, -1, -1, 0);
	if(!_t3f_animation_placeholder)
	{
		goto fail;
	}
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(_t3f_animation_placeholder->bitmap);
	al_clear_to_color(t3f_color_white);
	al_restore_state(&old_state);

	return true;

	fail:
	{
		return false;
	}
}

void _t3f_exit_animation_system(void)
{
	t3f_destroy_bitmap(_t3f_animation_placeholder);
}

static void _t3f_destroy_animation_data(T3F_ANIMATION_DATA * ap)
{
	int i;

	if(ap)
	{
		if(ap->frame_list)
		{
			free(ap->frame_list);
		}
		if(ap->frame)
		{
			for(i = 0; i < ap->frames; i++)
			{
				if(ap->frame[i])
				{
					free(ap->frame[i]);
				}
			}
			free(ap->frame);
		}
		if(!(ap->flags & T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS))
		{
			if(ap->bitmaps)
			{
				for(i = 0; i < ap->bitmaps->count; i++)
				{
					if(ap->bitmaps->bitmap[i])
					{
						t3f_destroy_bitmap(ap->bitmaps->bitmap[i]);
					}
				}
				free(ap->bitmaps);
			}
		}
		free(ap);
	}
}

static T3F_ANIMATION_DATA * _t3f_create_animation_data(void)
{
	T3F_ANIMATION_DATA * ap;

	ap = malloc(sizeof(T3F_ANIMATION_DATA));
	if(!ap)
	{
		goto fail;
	}
	memset(ap, 0, sizeof(T3F_ANIMATION_DATA));
	ap->bitmaps = malloc(sizeof(T3F_ANIMATION_BITMAPS));
	if(!ap->bitmaps)
	{
		goto fail;
	}
	memset(ap->bitmaps, 0, sizeof(T3F_ANIMATION_BITMAPS));

	return ap;

	fail:
	{
		_t3f_destroy_animation_data(ap);
		return NULL;
	}
}

/* memory management */
T3F_ANIMATION * t3f_create_animation(void)
{
	T3F_ANIMATION * ap;

	ap = malloc(sizeof(T3F_ANIMATION));
	if(!ap)
	{
		goto fail;
	}
	memset(ap, 0, sizeof(T3F_ANIMATION));
	ap->data = _t3f_create_animation_data();
	if(!ap->data)
	{
		goto fail;
	}
	return ap;

	fail:
	{
		t3f_destroy_animation(ap);
		return NULL;
	}
}

T3F_ANIMATION * t3f_clone_animation(T3F_ANIMATION * ap)
{
	int i;
	T3F_ANIMATION * clone = NULL;

	clone = t3f_create_animation();
	if(!clone)
	{
		goto fail;
	}
	clone->data = _t3f_create_animation_data();
	if(!clone->data)
	{
		goto fail;
	}
	if(ap->data->flags & T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS)
	{
		clone->data->bitmaps = ap->data->bitmaps;
	}
	else
	{
		for(i = 0; i < ap->data->bitmaps->count; i++)
		{
			t3f_clone_resource((void **)&(clone->data->bitmaps->bitmap[i]), (void **)&ap->data->bitmaps->bitmap[i]);
			if(!clone->data->bitmaps->bitmap[i])
			{
				printf("failed to clone bitmap\n");
				goto fail;
			}
		}
		clone->data->bitmaps->count = ap->data->bitmaps->count;
	}
	for(i = 0; i < ap->data->frames; i++)
	{
		if(!t3f_animation_add_frame(clone, ap->data->frame[i]->bitmap, ap->data->frame[i]->x, ap->data->frame[i]->y, ap->data->frame[i]->z, ap->data->frame[i]->width, ap->data->frame[i]->height, ap->data->frame[i]->angle, ap->data->frame[i]->ticks, ap->data->frame[i]->flags))
		{
			goto fail;
		}
	}
	clone->data->loop_point = ap->data->loop_point;
	clone->data->flags = ap->data->flags;
	return clone;

	fail:
	{
		t3f_destroy_animation(clone);
		return NULL;
	}
}

void t3f_destroy_animation(T3F_ANIMATION * ap)
{
	if(ap)
	{
		if(ap->object_loader)
		{
			t3f_destroy_object_loader(ap->object_loader);
		}
		if(ap->data)
		{
			_t3f_destroy_animation_data(ap->data);
		}
		free(ap);
	}
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

int _t3f_animation_data_build_frame_list(T3F_ANIMATION_DATA * ap)
{
	int i, j;

	/* count frames */
	ap->frame_list_total = 0;
	for(i = 0; i < ap->frames; i++)
	{
		for(j = 0; j < ap->frame[i]->ticks; j++)
		{
			ap->frame_list_total++;
		}
	}

	/* allocate frame list */
	if(ap->frame_list)
	{
		free(ap->frame_list);
	}
	ap->frame_list = NULL;
	if(ap->frame_list_total > 0)
	{
		ap->frame_list = malloc(sizeof(int) * ap->frame_list_total);
		if(!ap->frame_list)
		{
			goto fail;
		}

		/* build frame list */
		ap->frame_list_total = 0;
		for(i = 0; i < ap->frames; i++)
		{
			for(j = 0; j < ap->frame[i]->ticks; j++)
			{
				ap->frame_list[ap->frame_list_total] = i;
				ap->frame_list_total++;
			}
		}
	}
	return 1;

	fail:
	{
		return 0;
	}
}

static T3F_ANIMATION_DATA * _t3f_load_animation_data_f(ALLEGRO_FILE * fp, const char * fn, int flags)
{
	T3F_ANIMATION_DATA * ap = NULL;
	int i;
	char header[12]	= {0};
	int ver;

	al_fread(fp, header, 12);
	ver = check_header(header);
	if(ver < 0)
	{
		goto fail;
	}

	ap = _t3f_create_animation_data();
	if(!ap)
	{
		goto fail;
	}
	switch(ver)
	{
		case 0:
		{
			ap->bitmaps->count = al_fread16le(fp);
			for(i = 0; i < ap->bitmaps->count; i++)
			{
				ap->bitmaps->bitmap[i] = t3f_load_bitmap_f(fp, fn, flags | T3F_BITMAP_FLAG_DIRECT_LOAD);
				if(!ap->bitmaps->bitmap[i])
				{
					goto fail;
				}
			}
			ap->frames = al_fread16le(fp);
			ap->frame = malloc(sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			if(!ap->frame)
			{
				goto fail;
			}
			memset(ap->frame, 0, sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			for(i = 0; i < ap->frames; i++)
			{
				ap->frame[i] = malloc(sizeof(T3F_ANIMATION_FRAME));
				if(!ap->frame[i])
				{
					goto fail;
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
				ap->bitmaps->bitmap[i] = t3f_load_bitmap_f(fp, fn, flags);
				if(!ap->bitmaps->bitmap[i])
				{
					goto fail;
				}
			}
			ap->frames = al_fread16le(fp);
			ap->frame = malloc(sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			if(!ap->frame)
			{
				goto fail;
			}
			memset(ap->frame, 0, sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			for(i = 0; i < ap->frames; i++)
			{
				ap->frame[i] = al_malloc(sizeof(T3F_ANIMATION_FRAME));
				if(!ap->frame[i])
				{
					goto fail;
				}
				memset(ap->frame[i], 0, sizeof(T3F_ANIMATION_FRAME));
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
		case 2:
		{
			ap->bitmaps->count = al_fread16le(fp);
			for(i = 0; i < ap->bitmaps->count; i++)
			{
				ap->bitmaps->bitmap[i] = t3f_load_bitmap_f(fp, fn, flags);
				if(!ap->bitmaps->bitmap[i])
				{
					goto fail;
				}
			}
			ap->frames = al_fread16le(fp);
			ap->frame = malloc(sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			if(!ap->frame)
			{
				goto fail;
			}
			memset(ap->frame, 0, sizeof(T3F_ANIMATION_FRAME *) * ap->frames);
			for(i = 0; i < ap->frames; i++)
			{
				ap->frame[i] = al_malloc(sizeof(T3F_ANIMATION_FRAME));
				if(!ap->frame[i])
				{
					goto fail;
				}
				memset(ap->frame[i], 0, sizeof(T3F_ANIMATION_FRAME));
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
			ap->loop_point = al_fread32le(fp);
			ap->flags = al_fread32le(fp);
			break;
		}
	}
	_t3f_animation_data_build_frame_list(ap);
	return ap;

	fail:
	{
		_t3f_destroy_animation_data(ap);
		return NULL;
	}
}

T3F_ANIMATION * t3f_load_animation_f(ALLEGRO_FILE * fp, const char * fn, int flags)
{
	T3F_ANIMATION * ap;

	ap = t3f_create_animation();
	if(!ap)
	{
		goto fail;
	}
	ap->data = _t3f_load_animation_data_f(fp, fn, flags);

	return ap;

	fail:
	{
		t3f_destroy_animation(ap);
		return NULL;
	}
}

static T3F_ANIMATION_DATA * _t3f_really_load_animation_data(const char * fn, int flags)
{
	T3F_ANIMATION_DATA * ap;
	ALLEGRO_FILE * fp;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}

	ap = _t3f_load_animation_data_f(fp, fn, flags);
	al_fclose(fp);
	return ap;
}

static int _t3f_animation_data_add_bitmap(T3F_ANIMATION_DATA * ap, T3F_BITMAP * bp)
{
	if(ap->bitmaps->count < T3F_ANIMATION_MAX_BITMAPS)
	{
		ap->bitmaps->bitmap[ap->bitmaps->count] = bp;
		ap->bitmaps->count++;
		return 1;
	}
	return 0;
}

static int _t3f_animation_data_add_frame(T3F_ANIMATION_DATA * ap, int bitmap, float x, float y, float z, float w, float h, float angle, int ticks, int flags)
{
	int i;
	T3F_ANIMATION_FRAME ** new_frame;

	new_frame = malloc(sizeof(T3F_ANIMATION_FRAME *) * (ap->frames + 1));
	if(!new_frame)
	{
		goto fail;
	}
	memset(new_frame, 0, sizeof(T3F_ANIMATION_FRAME *) * (ap->frames + 1));

	if(ap->frame)
	{
		for(i = 0; i < ap->frames; i++)
		{
			new_frame[i] = malloc(sizeof(T3F_ANIMATION_FRAME));
			if(!new_frame[i])
			{
				goto fail;
			}
			memcpy(new_frame[i], ap->frame[i], sizeof(T3F_ANIMATION_FRAME));
		}
		for(i = 0; i < ap->frames; i++)
		{
			if(ap->frame[i])
			{
				free(ap->frame[i]);
			}
		}
		free(ap->frame);
	}
	ap->frame = new_frame;
	ap->frame[ap->frames] = malloc(sizeof(T3F_ANIMATION_FRAME));
	if(!ap->frame[ap->frames])
	{
		goto fail;
	}
	memset(ap->frame[ap->frames], 0, sizeof(T3F_ANIMATION_FRAME));
	ap->frame[ap->frames]->bitmap = bitmap;
	ap->frame[ap->frames]->x = x;
	ap->frame[ap->frames]->y = y;
	ap->frame[ap->frames]->z = z;
	if(w < 0.0)
	{
		ap->frame[ap->frames]->width = ap->bitmaps->bitmap[bitmap]->target_width;
	}
	else
	{
		ap->frame[ap->frames]->width = w;
	}
	if(h < 0.0)
	{
		ap->frame[ap->frames]->height = ap->bitmaps->bitmap[bitmap]->target_height;
	}
	else
	{
		ap->frame[ap->frames]->height = h;
	}
	ap->frame[ap->frames]->angle = angle;
	ap->frame[ap->frames]->ticks = ticks > 0 ? ticks : 1;
	ap->frame[ap->frames]->flags = flags;
	ap->frames++;
	_t3f_animation_data_build_frame_list(ap);

	return 1;

	fail:
	{
		return 0;
	}
}

static T3F_ANIMATION_DATA * _t3f_load_animation_data_from_bitmap(const char * fn, int flags)
{
	T3F_ANIMATION_DATA * ap = NULL;

	ap = _t3f_create_animation_data();
	if(!ap)
	{
		goto fail;
	}
	ap->bitmaps->bitmap[0] = t3f_load_bitmap(fn, flags, false);
	if(!ap->bitmaps->bitmap[0])
	{
		goto fail;
	}
	ap->bitmaps->count = 1;
	_t3f_animation_data_add_frame(ap, 0, 0.0, 0.0, 0.0, -1, -1, 0.0, 1, 0);
	return ap;

	fail:
	{
		if(ap)
		{
			_t3f_destroy_animation_data(ap);
		}
		return NULL;
	}
}

static const char * _get_config_value_fallbacks(ALLEGRO_CONFIG * cp, const char * section, const char * fallback_section, const char * fallback_section_2, const char * key)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(!val)
	{
		val = al_get_config_value(cp, fallback_section, key);
		if(!val)
		{
			val = al_get_config_value(cp, fallback_section_2, key);
		}
	}
	return val;
}

typedef struct
{

	ALLEGRO_PATH * path;
	int flags;

} _T3F_ANIMATION_LOAD_INFO;

static void * _t3f_load_actual_animation_data(void * arg)
{
	_T3F_ANIMATION_LOAD_INFO * load_info = (_T3F_ANIMATION_LOAD_INFO *)arg;
	T3F_ANIMATION_DATA * ret = NULL;

	if(arg)
	{
		ret = _t3f_really_load_animation_data(al_path_cstr(load_info->path, '/'), load_info->flags);
		if(!ret)
		{
			ret = _t3f_load_animation_data_from_bitmap(al_path_cstr(load_info->path, '/'), load_info->flags);
		}
		al_destroy_path(load_info->path);
		free(load_info);
	}
	else
	{
		ret = _t3f_create_animation_data();
		if(!ret)
		{
			goto fail;
		}
		ret->flags = T3F_ANIMATION_FLAG_EXTERNAL_BITMAPS;
		_t3f_animation_data_add_bitmap(ret, _t3f_animation_placeholder);
		_t3f_animation_data_add_frame(ret, 0, 0, 0, 0, 1, 1, 0.0, 1, 0);
	}

	return ret;

	fail:
	{
		return NULL;
	}
}

/* If a target size has been set, override frame sizes. Mostly used for loading
   animations from bitmaps. */
static void _t3f_fix_animation_frames(T3F_ANIMATION * ap)
{
	int i;

	if(ap->target_width > 0.0)
	{
		for(i = 0; i < ap->data->frames; i++)
		{
			ap->data->frame[i]->width = ap->target_width;
		}
	}
	if(ap->target_height > 0.0)
	{
		for(i = 0; i < ap->data->frames; i++)
		{
			ap->data->frame[i]->height = ap->target_height;
		}
	}
}

T3F_ANIMATION * t3f_load_animation(const char * fn, int flags, bool threaded)
{
	ALLEGRO_CONFIG * cp = NULL;
	const char * val;
	T3F_ANIMATION * ap = NULL;
	char * loading_fn = NULL;
	char * base_fn;
	char * loading_section;
	_T3F_ANIMATION_LOAD_INFO * load_info = NULL;

	ap = malloc(sizeof(T3F_ANIMATION));
	if(!ap)
	{
		goto fail;
	}
	memset(ap, 0, sizeof(T3F_ANIMATION));

	load_info = malloc(sizeof(_T3F_ANIMATION_LOAD_INFO));
	if(!load_info)
	{
		goto fail;
	}
	memset(load_info, 0, sizeof(_T3F_ANIMATION_LOAD_INFO));
	load_info->flags = flags;

	ap->object_loader = t3f_create_object_loader();
	if(!ap->object_loader)
	{
		goto fail;
	}
	loading_fn = strdup(fn);
	if(!loading_fn)
	{
		goto fail;
	}
	base_fn = strtok(loading_fn, "#");
	loading_section = strtok(NULL, "#");

	load_info->path = al_create_path(base_fn);
	if(!load_info->path)
	{
		goto fail;
	}

	if(strstr(fn, ".ini"))
	{
		cp = al_load_config_file(base_fn);
		if(!cp)
		{
			goto fail;
		}

		/* get the source animatio filename from 'section' */
		val = _get_config_value_fallbacks(cp, loading_section, "Animation", "Bitmap", "filename");

		/* populate 'loading_path' */
		if(val)
		{
			al_set_path_filename(load_info->path, val);
		}
		else
		{
			goto fail;
		}

		val = _get_config_value_fallbacks(cp, loading_section, "Animation", "Bitmap", "target_width");
		if(val)
		{
			ap->target_width = atof(val);
		}
		val = _get_config_value_fallbacks(cp, loading_section, "Animation", "Bitmap", "target_height");
		if(val)
		{
			ap->target_height = atof(val);
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
	ap->data = t3f_load_object(ap->object_loader, _t3f_load_actual_animation_data, NULL, load_info, threaded);
	if(!ap->data)
	{
		load_info = NULL;
		goto fail;
	}
	_t3f_fix_animation_frames(ap);

	return ap;

	fail:
	{
		if(cp)
		{
			al_destroy_config(cp);
		}
		if(loading_fn)
		{
			free(loading_fn);
		}
		if(load_info)
		{
			if(load_info->path)
			{
				al_destroy_path(load_info->path);
			}
			free(load_info);
		}
		t3f_destroy_animation(ap);
		return NULL;
	}
}

int t3f_save_animation_f(T3F_ANIMATION * ap, ALLEGRO_FILE * fp)
{
	int i;

	ani_header[11] = T3F_ANIMATION_REVISION; // put the version number in
	al_fwrite(fp, ani_header, 12);
	al_fwrite16le(fp, ap->data->bitmaps->count);
	for(i = 0; i < ap->data->bitmaps->count; i++)
	{
		if(!t3f_save_allegro_bitmap_f(fp, ap->data->bitmaps->bitmap[i]->bitmap))
		{
			printf("failed to save bitmap\n");
			return 0;
		}
	}
	al_fwrite16le(fp, ap->data->frames);
	for(i = 0; i < ap->data->frames; i++)
	{
		al_fwrite16le(fp, ap->data->frame[i]->bitmap);
		t3f_fwrite_float(fp, ap->data->frame[i]->x);
		t3f_fwrite_float(fp, ap->data->frame[i]->y);
		t3f_fwrite_float(fp, ap->data->frame[i]->z);
		t3f_fwrite_float(fp, ap->data->frame[i]->width);
		t3f_fwrite_float(fp, ap->data->frame[i]->height);
		t3f_fwrite_float(fp, ap->data->frame[i]->angle);
		al_fwrite32le(fp, ap->data->frame[i]->ticks);
		al_fwrite32le(fp, ap->data->frame[i]->flags);
	}
	al_fwrite32le(fp, ap->data->loop_point);
	al_fwrite32le(fp, ap->data->flags);
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

bool t3f_update_animation(T3F_ANIMATION * ap)
{
	int i;
	T3F_ANIMATION_DATA * new_data;

	if(t3f_object_ready(ap->object_loader))
	{
		new_data = t3f_fetch_object(ap->object_loader);
		if(new_data)
		{
			if(new_data != ap->data)
			{
				_t3f_destroy_animation_data(ap->data);
				ap->data = new_data;
			}
			_t3f_fix_animation_frames(ap);
			for(i = 0; i < ap->data->bitmaps->count; i++)
			{
				if(ap->data->bitmaps->bitmap[i])
				{
					al_convert_bitmap(ap->data->bitmaps->bitmap[i]->bitmap);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

/* utilities */
int t3f_animation_add_bitmap(T3F_ANIMATION * ap, T3F_BITMAP * bp)
{
	return _t3f_animation_data_add_bitmap(ap->data, bp);
}

int t3f_animation_delete_bitmap(T3F_ANIMATION * ap, int bitmap)
{
	int i;

	if(bitmap < ap->data->bitmaps->count)
	{
		t3f_destroy_bitmap(ap->data->bitmaps->bitmap[bitmap]);
	}
	else
	{
		return 0;
	}
	for(i = bitmap; i < ap->data->bitmaps->count - 1; i++)
	{
		ap->data->bitmaps->bitmap[i] = ap->data->bitmaps->bitmap[i + 1];
	}
	ap->data->bitmaps->count--;
	return 1;
}

int t3f_animation_add_frame(T3F_ANIMATION * ap, int bitmap, float x, float y, float z, float w, float h, float angle, int ticks, int flags)
{
	return _t3f_animation_data_add_frame(ap->data, bitmap, x, y, z, w, h, angle, ticks, flags);
}

int t3f_animation_delete_frame(T3F_ANIMATION * ap, int frame)
{
	int i;

	if(frame < ap->data->frames)
	{
		free(ap->data->frame[frame]);
	}
	else
	{
		return 0;
	}
	for(i = frame; i < ap->data->frames - 1; i++)
	{
		ap->data->frame[i] = ap->data->frame[i + 1];
	}
	ap->data->frames--;
	t3f_animation_build_frame_list(ap);
	return 1;
}

int t3f_animation_build_frame_list(T3F_ANIMATION * ap)
{
	return _t3f_animation_data_build_frame_list(ap->data);
}

bool t3f_add_animation_to_atlas(T3F_ATLAS * sap, T3F_ANIMATION * ap, int flags)
{
	int i, failed = 0;

	/* add bitmaps to sprite sheet */
	for(i = 0; i < ap->data->bitmaps->count; i++)
	{
		if(!t3f_add_bitmap_to_atlas(sap, &(ap->data->bitmaps->bitmap[i]->bitmap), flags))
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
T3F_BITMAP * t3f_animation_get_bitmap(T3F_ANIMATION * ap, int tick)
{
	if(tick >= ap->data->frame_list_total && ap->data->flags & T3F_ANIMATION_FLAG_ONCE)
	{
		return ap->data->bitmaps->bitmap[ap->data->frame[ap->data->frame_list[ap->data->frame_list_total - 1]]->bitmap];
	}
	else
	{
		return ap->data->bitmaps->bitmap[ap->data->frame[ap->data->frame_list[tick % ap->data->frame_list_total]]->bitmap];
	}
}

T3F_ANIMATION_FRAME * t3f_animation_get_frame(T3F_ANIMATION * ap, int tick)
{
	int loop_ticks;

	if(ap->data->frames <= 0)
	{
		return NULL;
	}
	if(tick >= ap->data->frame_list_total && ap->data->flags & T3F_ANIMATION_FLAG_ONCE)
	{
		return ap->data->frame[ap->data->frame_list[ap->data->frame_list_total - 1]];
	}
	else
	{
		loop_ticks = ap->data->frame_list_total - ap->data->loop_point;
		if(tick >= ap->data->frame_list_total)
		{
			tick -= ap->data->loop_point;
			tick = tick % loop_ticks + ap->data->loop_point;
		}
		return ap->data->frame[ap->data->frame_list[tick]];
	}
}

static void handle_vh_flip(T3F_ANIMATION_FRAME * base_fp, T3F_ANIMATION_FRAME * fp, int flags, float * fox, float * foy, int * dflags)
{
	bool vflip = false;
	bool hflip = false;

	if(fp->flags & T3F_DRAW_H_FLIP && !(flags & T3F_DRAW_H_FLIP))
	{
		hflip = true;
	}
	else if(flags & T3F_DRAW_H_FLIP && !(fp->flags & T3F_DRAW_H_FLIP))
	{
		hflip = true;
	}
	if(fp->flags & T3F_DRAW_V_FLIP && !(flags & T3F_DRAW_V_FLIP))
	{
		vflip = true;
	}
	else if(flags & T3F_DRAW_V_FLIP && !(fp->flags & T3F_DRAW_V_FLIP))
	{
		vflip = true;
	}
	if(hflip)
	{
		*dflags |= T3F_DRAW_H_FLIP;
		*fox = -((fp->x + fp->width) - (base_fp->x + base_fp->width)) - (fp->x - base_fp->x);
	}
	if(vflip)
	{
		*dflags |= T3F_DRAW_V_FLIP;
		*foy = -((fp->y + fp->height) - (base_fp->y + base_fp->height)) - (fp->y - base_fp->y);
	}
}

void t3f_draw_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, int flags)
{
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = flags & ~(T3F_DRAW_H_FLIP | T3F_DRAW_V_FLIP);

	if(fp)
	{
		handle_vh_flip(ap->data->frame[0], fp, flags, &fox, &foy, &dflags);
		t3f_draw_scaled_bitmap(ap->data->bitmaps->bitmap[fp->bitmap], color, x + fp->x + fox, y + fp->y + foy, z + fp->z, fp->width, fp->height, dflags);
	}
}

void t3f_draw_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float x, float y, float z, float scale, int flags)
{
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	int dflags = 0;
	float fox = 0.0, foy = 0.0;

	if(fp)
	{
		handle_vh_flip(ap->data->frame[0], fp, flags, &fox, &foy, &dflags);
		t3f_draw_scaled_bitmap(ap->data->bitmaps->bitmap[fp->bitmap], color, x + (fp->x + fox) * scale, y + (fp->y + foy) * scale, z + fp->z, fp->width * scale, fp->height * scale, dflags);
	}
}

void t3f_draw_rotated_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, int flags)
{
	float scale_x, scale_y;
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = flags & ~(T3F_DRAW_H_FLIP | T3F_DRAW_V_FLIP);
	if(fp)
	{
		handle_vh_flip(ap->data->frame[0], fp, flags, &fox, &foy, &dflags);
		scale_x = fp->width / ap->data->bitmaps->bitmap[fp->bitmap]->target_width;
		scale_y = fp->height / ap->data->bitmaps->bitmap[fp->bitmap]->target_height;
		t3f_draw_scaled_rotated_bitmap(ap->data->bitmaps->bitmap[fp->bitmap], color, cx / scale_x - fp->x, cy / scale_y - fp->y, x + fox, y + foy, z + fp->z, angle, scale_x, scale_y, dflags);
	}
}

void t3f_draw_rotated_scaled_animation(T3F_ANIMATION * ap, ALLEGRO_COLOR color, int tick, float cx, float cy, float x, float y, float z, float angle, float scale, int flags)
{
	float scale_x, scale_y;
	T3F_ANIMATION_FRAME * fp = t3f_animation_get_frame(ap, tick);
	float fox = 0.0;
	float foy = 0.0;
	int dflags = flags & ~(T3F_DRAW_H_FLIP | T3F_DRAW_V_FLIP);
	if(fp)
	{
		handle_vh_flip(ap->data->frame[0], fp, flags, &fox, &foy, &dflags);
		scale_x = fp->width / ap->data->bitmaps->bitmap[fp->bitmap]->target_width;
		scale_y = fp->height / ap->data->bitmaps->bitmap[fp->bitmap]->target_height;
		t3f_draw_scaled_rotated_bitmap(ap->data->bitmaps->bitmap[fp->bitmap], color, cx / scale_x - fp->x, cy / scale_y - fp->y, x + fox, y + foy, z + fp->z, angle, scale * scale_x, scale * scale_y, dflags);
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
		sscale_x = fp->width / ap->data->bitmaps->bitmap[fp->bitmap]->target_width;
		sscale_y = fp->height / ap->data->bitmaps->bitmap[fp->bitmap]->target_height;
		fsx = sx / sscale_x;
		fsy = sy / sscale_y;
		fsw = sw / sscale_x;
		fsh = sh / sscale_y;
		t3f_draw_scaled_rotated_bitmap_region(ap->data->bitmaps->bitmap[fp->bitmap], fsx, fsy, fsw, fsh, color, cx / sscale_x - fp->x, cy / sscale_y - fp->y, x + fox, y + foy, scale * sscale_x * pw, scale * sscale_y * pw, angle, flags);
	}
}