#include "t3f.h"
#include "view.h"

T3F_VIEW * t3f_default_view = NULL;
T3F_VIEW * t3f_current_view = NULL;

void t3f_set_view_focus(T3F_VIEW * vp, float fx, float fy)
{
	vp->vp_x = fx;
	vp->vp_y = fy;
}

void t3f_select_view(T3F_VIEW * sp)
{
	float sx, sy;
	float dsx, dsy;

	if(sp != NULL)
	{
		t3f_current_view = sp;
	}
	else
	{
		t3f_current_view = t3f_default_view;
	}
	dsx = (float)t3f_display_width / t3f_virtual_display_width;
	dsy = (float)t3f_display_height / t3f_virtual_display_height;
	sx = t3f_current_view->width / t3f_virtual_display_width;
	sy = t3f_current_view->height / t3f_virtual_display_height;

	/* apply additional transformations */
	al_build_transform(&t3f_current_transform, t3f_display_offset_x + (t3f_current_view->offset_x * dsx), t3f_display_offset_y + (t3f_current_view->offset_y * dsy), dsx * sx, dsy * sy, 0.0);
	al_use_transform(&t3f_current_transform);
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

//	if(z + t3f_current_view->width > 0)
	if(z + t3f_virtual_display_width > 0)
	{
//		rx = (((x - t3f_current_view->vp_x) * t3f_current_view->width) / (z + t3f_current_view->width) + t3f_current_view->vp_x);
		rx = (((x - t3f_current_view->vp_x) * t3f_virtual_display_width) / (z + t3f_virtual_display_width) + t3f_current_view->vp_x);
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

//	if(z + t3f_current_view->height > 0)
	if(z + t3f_virtual_display_width > 0)
	{
//		ry = (((y - t3f_current_view->vp_y) * t3f_current_view->width) / (z + t3f_current_view->width) + t3f_current_view->vp_y);
		ry = (((y - t3f_current_view->vp_y) * t3f_virtual_display_width) / (z + t3f_virtual_display_width) + t3f_current_view->vp_y);
		return ry;
	}
	else
	{
		return -65536;
	}
}
