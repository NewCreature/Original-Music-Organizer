#include "t3f.h"
#include "view.h"
#include "mouse.h"
#include "touch.h"

T3F_VIEW * t3f_default_view = NULL;
T3F_VIEW * t3f_current_view = NULL;

void t3f_set_view_focus(T3F_VIEW * vp, float fx, float fy)
{
	vp->vp_x = fx;
	vp->vp_y = fy;
}

void t3f_adjust_view(T3F_VIEW * vp, float ox, float oy, float w, float h, float vpx, float vpy, int flags)
{
	vp->offset_x = ox;
	vp->offset_y = oy;
	vp->width = w;
	vp->height = h;
	vp->vp_x = vpx;
	vp->vp_y = vpy;
	vp->flags = flags;
	vp->need_update = true;
}

T3F_VIEW * t3f_create_view(T3F_VIEW * parent, float ox, float oy, float w, float h, float vpx, float vpy, int flags)
{
	T3F_VIEW * vp = al_malloc(sizeof(T3F_VIEW));
	if(!vp)
	{
		return NULL;
	}
	vp->parent = parent;
	vp->virtual_width = t3f_virtual_display_width;
	vp->virtual_height = t3f_virtual_display_height;
	vp->aspect_min = 0.0;
	vp->aspect_max = 0.0;
	t3f_adjust_view(vp, ox, oy, w, h, vpx, vpy, flags);
	return vp;
}

void t3f_constrain_view_aspect_ratio(T3F_VIEW * vp, float min, float max)
{
	vp->aspect_min = min;
	vp->aspect_max = max;
	vp->need_update = true;
}

void t3f_set_view_virtual_dimensions(T3F_VIEW * vp, int w, int h)
{
	vp->virtual_width = w;
	vp->virtual_height = h;
	vp->need_update = true;
}

static void _t3f_setup_view_transformation_letterbox(T3F_VIEW * view, float view_ratio, float virtual_display_ratio)
{
	/* need to adjust y */
	if(view_ratio < virtual_display_ratio)
	{
		view->scale_x = view->width / (float)view->virtual_width;
		view->scale_y = view->scale_x;
		view->translate_x = view->offset_x + 0.0;
		view->translate_y = view->offset_y + (view->height - view->width / virtual_display_ratio) / 2.0;
	}
	else
	{
		view->scale_y = view->height / (float)view->virtual_height;
		view->scale_x = view->scale_y;
		view->translate_x = view->offset_x + (view->width - view->height * virtual_display_ratio) / 2.0;
		view->translate_y = view->offset_y + 0.0;
	}
	view->left = 0;
	view->top = 0;
	view->bottom = view->virtual_height;
	view->right = view->virtual_width;
}

static void _t3f_setup_view_transformation_letterbox_fill(T3F_VIEW * view, float view_ratio, float virtual_display_ratio)
{
	float min_size;

	/* constrain the view ratio */
	if(view_ratio < view->aspect_min)
	{
		view_ratio = view->aspect_min;
	}
	else if(view_ratio > view->aspect_max)
	{
		view_ratio = view->aspect_max;
	}
	min_size = view->width;
	view->scale_x = min_size / (float)view->virtual_width;
	view->scale_y = view->scale_x;
	view->left = 0.0;
	view->top = 0.0;
	view->translate_x = 0.0;
	view->translate_y = (view->offset_y + view->height) / 2.0 - (view->virtual_height * view->scale_y) / 2.0;
	view->bottom = view->virtual_height - view->top;
	view->right = view->virtual_width - view->left;
}

static void _t3f_setup_view_transformation_fillscreen(T3F_VIEW * view, float view_ratio, float virtual_display_ratio)
{
	if(view->aspect_min > 0.0)
	{
		if(view_ratio < view->aspect_min)
		{
			_t3f_setup_view_transformation_letterbox_fill(view, view_ratio, virtual_display_ratio);
			return;
		}
	}
	if(view->aspect_max > 0.0)
	{
		if(view_ratio > view->aspect_max)
		{
			_t3f_setup_view_transformation_letterbox(view, view_ratio, virtual_display_ratio);
			return;
		}
	}
	/* need to adjust y */
	if(view_ratio >= virtual_display_ratio)
	{
		view->scale_x = view->width / (float)view->virtual_width;
		view->scale_y = view->scale_x;
		view->left = 0.0;
		view->top = -(view->height - view->width / virtual_display_ratio) / 2.0;
		view->translate_x = view->offset_x + 0.0;
		view->translate_y = view->offset_y - view->top;
	}
	else
	{
		view->scale_y = view->height / (float)view->virtual_height;
		view->scale_x = view->scale_y;
		view->left = -(view->width - view->height * virtual_display_ratio) / 2.0;
		view->top = 0.0;
		view->translate_x = view->offset_x - view->left;
		view->translate_y = view->offset_y + 0.0;
	}
	view->left /= view->scale_x;
	view->top /= view->scale_y;
	view->bottom = view->virtual_height - view->top;
	view->right = view->virtual_width - view->left;
}

static void t3f_get_view_transformation(T3F_VIEW * view)
{
	double view_ratio, virtual_display_ratio;

	if(view->flags & T3F_NO_SCALE)
	{
		view->translate_x = view->offset_x;
		view->translate_y = view->offset_y;
		view->scale_x = 1.0;
		view->scale_y = 1.0;
		view->left = 0;
		view->top = 0;
		view->bottom = view->height;
		view->right = view->width;
	}
	else if(view->flags & T3F_FORCE_ASPECT)
	{
		view_ratio = view->width / view->height;
		virtual_display_ratio = (float)view->virtual_width / (float)view->virtual_height;

		if(view->flags & T3F_FILL_SCREEN)
		{
			_t3f_setup_view_transformation_fillscreen(view, view_ratio, virtual_display_ratio);
		}
		else
		{
			_t3f_setup_view_transformation_letterbox(view, view_ratio, virtual_display_ratio);
		}
	}
	else
	{
		view->translate_x = view->offset_x;
		view->translate_y = view->offset_y;
		view->scale_x = view->width / (float)view->virtual_width;
		view->scale_y = view->height / (float)view->virtual_height;
		view->left = 0;
		view->top = 0;
		view->right = view->virtual_width;
		view->bottom = view->virtual_height;
	}
}

void t3f_select_view(T3F_VIEW * vp)
{
	T3F_VIEW * current_view;
	T3F_VIEW * last_view = NULL;
	bool regenerate_transformation = false;
	float translate_x = 0.0;
	float translate_y = 0.0;
	float scale_x = 1.0;
	float scale_y = 1.0;
	float parent_scale_x;
	float parent_scale_y;

	/* flush drawing cache (ensures clipping applies properly) */
	if(al_is_bitmap_drawing_held())
	{
		al_hold_bitmap_drawing(false);
		al_hold_bitmap_drawing(true);
	}

	/* if any view in the lineage is dirty, regenerate transformation */
	current_view = vp;
	while(current_view)
	{
		current_view->child = last_view;
		last_view = current_view;
		if(current_view->need_update)
		{
			t3f_get_view_transformation(current_view);
			regenerate_transformation = true;
		}
		current_view = current_view->parent;
	}

	if(regenerate_transformation)
	{
		/* determine final translation and scale */
		current_view = last_view;
		parent_scale_x = 1.0;
		parent_scale_y = 1.0;
		while(current_view)
		{
			if(current_view->parent)
			{
				parent_scale_x *= current_view->parent->scale_x;
				parent_scale_y *= current_view->parent->scale_y;
			}
			translate_x += current_view->translate_x * parent_scale_x;
			translate_y += current_view->translate_y * parent_scale_y;
			scale_x *= current_view->scale_x;
			scale_y *= current_view->scale_y;
			al_build_transform(&current_view->transform, translate_x, translate_y, scale_x, scale_y, 0.0);
			current_view->need_update = false;
			current_view = current_view->child;
		}
	}
	al_use_transform(&vp->transform);
	t3f_current_view = vp;
	t3f_set_clipping_rectangle(0, 0, 0, 0);
}

T3F_VIEW * t3f_get_current_view(void)
{
	return t3f_current_view;
}

bool t3f_project_coordinates(float vw, float vpx, float vpy, float * x, float * y, float z)
{
	if(z + vw > 0)
	{
		if(x)
		{
			*x = (((*x - vpx) * vw) / (z + vw) + vpx);
		}
		if(y)
		{
			*y = (((*y - vpy) * vw) / (z + vw) + vpy);
		}
		return true;
	}
	return false;
}

/* get the x coordinate of the pixel at the given (x, z) coordinate
   takes current projection state into account */
float t3f_project_x(float x, float z)
{
	float rx;

	if(z + t3f_current_view->virtual_width > 0)
	{
		rx = (((x - t3f_current_view->vp_x) * t3f_current_view->virtual_width) / (z + t3f_current_view->virtual_width) + t3f_current_view->vp_x);
		return rx;
	}
	else
	{
		return -65536;
	}
}

/* get the y coordinate of the pixel at the given (y, z) coordinate
   takes current projection state into account */
float t3f_project_y(float y, float z)
{
	float ry;

	if(z + t3f_current_view->virtual_width > 0)
	{
		ry = (((y - t3f_current_view->vp_y) * t3f_current_view->virtual_width) / (z + t3f_current_view->virtual_width) + t3f_current_view->vp_y);
		return ry;
	}
	else
	{
		return -65536;
	}
}

/* apply a transformation to render starting at (x, y, z) */
void t3f_set_view_render_offset(float x, float y, float z)
{
	float final_x, final_y, final_x2, final_y2;
	float final_scale_x, final_scale_y;
	ALLEGRO_TRANSFORM new_transform;

	final_x = t3f_project_x(x, z);
	final_y = t3f_project_y(y, z);
	final_x2 = t3f_project_x(x + 1.0, z);
	final_y2 = t3f_project_y(y + 1.0, z);
	al_transform_coordinates(&t3f_current_view->transform, &final_x, &final_y);
	al_transform_coordinates(&t3f_current_view->transform, &final_x2, &final_y2);
	final_scale_x = final_x2 - final_x;
	final_scale_y = final_y2 - final_y;
	al_build_transform(&new_transform, final_x, final_y, final_scale_x, final_scale_y, 0.0);
	al_use_transform(&new_transform);
}

void t3f_select_input_view(T3F_VIEW * vp)
{
	T3F_VIEW * old_view = t3f_current_view;
	T3F_VIEW * current_view;
	T3F_VIEW * last_view = NULL;
	float translate_x = 0.0;
	float translate_y = 0.0;
	float scale_x = 1.0;
	float scale_y = 1.0;
	bool regenerate_transformation = false;

	current_view = vp;
	while(current_view)
	{
		current_view->child = last_view;
		last_view = current_view;
		if(current_view->need_update)
		{
			t3f_get_view_transformation(current_view);
			regenerate_transformation = true;
		}
		current_view = current_view->parent;
	}

	if(regenerate_transformation)
	{
		t3f_select_view(vp);
		t3f_select_view(old_view);
	}
	al_transform_coordinates(&vp->transform, &translate_x, &translate_y);
	al_transform_coordinates(&vp->transform, &scale_x, &scale_y);
	scale_x -= translate_x;
	scale_y -= translate_y;

	/* get new mouse coordinates */
	if(t3f_flags & T3F_USE_MOUSE)
	{
		_t3f_update_mouse_pos(translate_x, scale_x, translate_y, scale_y);
		_t3f_update_touch_pos(0, translate_x, scale_x, translate_y, scale_y);
	}

	/* get new touch coordinates */
	if(t3f_flags & T3F_USE_TOUCH)
	{
		_t3f_update_touch_pos(-1, translate_x, scale_x, translate_y, scale_y);
	}
}
