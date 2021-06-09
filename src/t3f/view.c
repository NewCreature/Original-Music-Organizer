#include "t3f.h"
#include "view.h"

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

T3F_VIEW * t3f_create_view(float ox, float oy, float w, float h, float vpx, float vpy, int flags)
{
	T3F_VIEW * vp = al_malloc(sizeof(T3F_VIEW));
	if(!vp)
	{
		return NULL;
	}
	vp->virtual_width = t3f_virtual_display_width;
	vp->virtual_height = t3f_virtual_display_height;
	t3f_adjust_view(vp, ox, oy, w, h, vpx, vpy, flags);
	return vp;
}

void t3f_set_view_virtual_dimensions(T3F_VIEW * vp, int w, int h)
{
	vp->virtual_width = w;
	vp->virtual_height = h;
	vp->need_update = true;
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
		view_ratio = view->height / view->width;
		virtual_display_ratio = (float)view->virtual_height / (float)view->virtual_width;

		if(view->flags & T3F_FILL_SCREEN)
		{
			/* need to adjust y */
			if(view_ratio <= virtual_display_ratio)
			{
				view->scale_x = view->width / (float)view->virtual_width;
				view->scale_y = view->scale_x;
				view->left = 0.0;
				view->top = -(view->height - view->width * virtual_display_ratio) / 2.0;
				view->translate_x = view->offset_x + 0.0;
				view->translate_y = view->offset_y - view->top;
			}
			else
			{
				view->scale_y = view->height / (float)view->virtual_height;
				view->scale_x = view->scale_y;
				view->left = -(view->width - view->height / virtual_display_ratio) / 2.0;
				view->top = 0.0;
				view->translate_x = view->offset_x - view->left;
				view->translate_y = view->offset_y + 0.0;
			}
			view->left /= view->scale_x;
			view->top /= view->scale_y;
			view->bottom = view->virtual_height - view->top;
			view->right = view->virtual_width - view->left;
		}
		else
		{
			/* need to adjust y */
			if(view_ratio > virtual_display_ratio)
			{
				view->scale_x = view->width / (float)view->virtual_width;
				view->scale_y = view->scale_x;
				view->translate_x = view->offset_x + 0.0;
				view->translate_y = view->offset_y + (view->height - view->width * virtual_display_ratio) / 2.0;
			}
			else
			{
				view->scale_y = view->height / (float)view->virtual_height;
				view->scale_x = view->scale_y;
				view->translate_x = view->offset_x + (view->width - view->height / virtual_display_ratio) / 2.0;
				view->translate_y = view->offset_y + 0.0;
			}
			view->left = 0;
			view->top = 0;
			view->bottom = view->virtual_height;
			view->right = view->virtual_width;
		}
	}
	else
	{
		view->translate_x = 0.0;
		view->translate_y = 0.0;
		view->scale_x = view->width / (float)view->virtual_width;
		view->scale_y = view->height / (float)view->virtual_height;
		view->left = 0;
		view->top = 0;
		view->right = view->virtual_width;
		view->bottom = view->virtual_height;
	}
}

/* combine a base view with another view to generate a transformation */
static void t3f_select_views(T3F_VIEW * base_view, T3F_VIEW * view)
{
	bool regenerate_transformation = false;
	float translate_x = 0.0;
	float translate_y = 0.0;
	float scale_x = 1.0;
	float scale_y = 1.0;

	if(al_is_bitmap_drawing_held())
	{
		al_hold_bitmap_drawing(false);
		al_hold_bitmap_drawing(true);
	}
	if(!base_view)
	{
		base_view = t3f_default_view;
	}

	/* generate scaling and positioning for base_view */
	if(base_view->need_update)
	{
		t3f_get_view_transformation(base_view);
		regenerate_transformation = true;
	}

	/* generate scaling and positioning for view */
	if(view && view->need_update)
	{
		t3f_get_view_transformation(view);
		regenerate_transformation = true;
		view->need_update = false;
	}

	/* select the desired view */
	if(view && view != base_view)
	{
		translate_x = base_view->translate_x + view->translate_x * base_view->scale_x;
		translate_y = base_view->translate_y + view->translate_y * base_view->scale_y;
		scale_x = base_view->scale_x * view->scale_x;
		scale_y = base_view->scale_y * view->scale_y;
		t3f_current_view = view;
	}
	else
	{
		translate_x = base_view->translate_x;
		translate_y = base_view->translate_y;
		scale_x = base_view->scale_x;
		scale_y = base_view->scale_y;
		t3f_current_view = base_view;
	}

	/* create transformation from base_view and view offset and scale */
	if(regenerate_transformation)
	{
		al_build_transform(&t3f_current_view->transform, translate_x, translate_y, scale_x, scale_y, 0.0);
	}
	al_copy_transform(&t3f_current_transform, &t3f_current_view->transform);
	al_use_transform(&t3f_current_transform);
	t3f_set_clipping_rectangle(0, 0, 0, 0);
}

void t3f_select_view(T3F_VIEW * sp)
{
	t3f_select_views(t3f_default_view, sp);
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

void t3f_select_input_view(T3F_VIEW * vp)
{
	T3F_VIEW * old_view = t3f_current_view;
	float translate_x = 0.0;
	float translate_y = 0.0;
	float scale_x = 1.0;
	float scale_y = 1.0;
	int i;

	if(vp->need_update)
	{
		t3f_select_view(vp);
		t3f_current_view = old_view;
	}

	if(vp && vp != t3f_default_view)
	{
		translate_x = t3f_default_view->translate_x + vp->translate_x * t3f_default_view->scale_x;
		translate_y = t3f_default_view->translate_y + vp->translate_y * t3f_default_view->scale_y;
		scale_x = t3f_default_view->scale_x * vp->scale_x;
		scale_y = t3f_default_view->scale_y * vp->scale_y;
	}
	else
	{
		translate_x = t3f_default_view->translate_x;
		translate_y = t3f_default_view->translate_y;
		scale_x = t3f_default_view->scale_x;
		scale_y = t3f_default_view->scale_y;
	}

	/* get new mouse coordinates */
	if(t3f_flags & T3F_USE_MOUSE)
	{
		t3f_mouse_x = (t3f_real_mouse_x - translate_x) / scale_x;
		t3f_mouse_y = (t3f_real_mouse_y - translate_y) / scale_y;
		t3f_touch[0].x = (t3f_touch[0].real_x - translate_x) / scale_x;
		t3f_touch[0].y = (t3f_touch[0].real_y - translate_y) / scale_y;
	}

	/* get new touch coordinates */
	if(t3f_flags & T3F_USE_TOUCH)
	{
		for(i = 1; i < T3F_MAX_TOUCHES; i++)
		{
			t3f_touch[i].x = (t3f_touch[i].real_x - translate_x) / scale_x;
			t3f_touch[i].y = (t3f_touch[i].real_y - translate_y) / scale_y;
		}
	}
}
