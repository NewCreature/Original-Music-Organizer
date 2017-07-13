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
	t3f_adjust_view(vp, ox, oy, w, h, vpx, vpy, flags);
	return vp;
}

void t3f_select_view(T3F_VIEW * sp)
{
	float dsx, dsy;
	float r, vr;
	float ox, oy;
	float dw, dh;
	float vox, voy;

	/* select default view if NULL is passed */
	if(sp != NULL)
	{
		t3f_current_view = sp;
	}
	else
	{
		t3f_current_view = t3f_default_view;
	}

	if(t3f_current_view->need_update)
	{
		if(t3f_current_view->flags & T3F_FORCE_ASPECT)
		{
			r = t3f_current_view->height / t3f_current_view->width;
			vr = (float)t3f_virtual_display_height / (float)t3f_virtual_display_width;
			if(t3f_current_view->flags & T3F_FILL_SCREEN)
			{
				/* need to adjust y */
				if(r <= vr)
				{
					vox = 0;
					ox = t3f_current_view->offset_x + vox;
					dw = t3f_current_view->width;
					voy = (t3f_current_view->height - t3f_current_view->width * vr) / 2.0;
					oy = t3f_current_view->offset_y + voy;
					dh = t3f_current_view->width * vr;
				}
				else
				{
					vox = (t3f_current_view->width - t3f_current_view->height / vr) / 2.0;
					ox = t3f_current_view->offset_x + vox;
					dw = t3f_current_view->height / vr;
					voy = 0;
					oy = t3f_current_view->offset_y + voy;
					dh = t3f_current_view->height;
				}
				dsx = dw / (float)t3f_virtual_display_width;
				dsy = dh / (float)t3f_virtual_display_height;
				al_build_transform(&t3f_current_view->transform, t3f_display_offset_x + ox * t3f_display_scale_x, t3f_display_offset_y + oy * t3f_display_scale_y, dsx * t3f_display_scale_x, dsy * t3f_display_scale_y, 0.0);
			}
			else
			{
				/* need to adjust y */
				if(r > vr)
				{
					vox = 0;
					ox = t3f_current_view->offset_x + vox;
					dw = t3f_current_view->width;
					voy = (t3f_current_view->height - t3f_current_view->width * vr) / 2.0;
					oy = t3f_current_view->offset_y + voy;
					dh = t3f_current_view->width * vr;
				}
				else
				{
					vox = (t3f_current_view->width - t3f_current_view->height / vr) / 2.0;
					ox = t3f_current_view->offset_x + vox;
					dw = t3f_current_view->height / vr;
					voy = 0;
					oy = t3f_current_view->offset_y + voy;
					dh = t3f_current_view->height;
				}
				dsx = dw / (float)t3f_virtual_display_width;
				dsy = dh / (float)t3f_virtual_display_height;
				al_build_transform(&t3f_current_view->transform, t3f_display_offset_x + ox * t3f_display_scale_x, t3f_display_offset_y + oy * t3f_display_scale_y, dsx * t3f_display_scale_x, dsy * t3f_display_scale_y, 0.0);
			}
		}
		else
		{
			dw = t3f_current_view->width;
			dh = t3f_current_view->height;
			dsx = t3f_current_view->width / (float)t3f_virtual_display_width;
			dsy = t3f_current_view->height / (float)t3f_virtual_display_height;
			al_build_transform(&t3f_current_view->transform, t3f_current_view->offset_x, t3f_current_view->offset_y, dsx, dsy, 0.0);
		}

		/* set up edge coordinates for use with T3F_FILL_SCREEN */
		if(dsx == 0)
		{
			t3f_current_view->left = 0;
			t3f_current_view->right = t3f_virtual_display_width;
			t3f_current_view->top = -voy / dsy;
			t3f_current_view->bottom = t3f_virtual_display_height - t3f_current_view->top;
		}
		else
		{
			t3f_current_view->top = 0;
			t3f_current_view->bottom = t3f_virtual_display_height;
			t3f_current_view->left = -vox / dsx;
			t3f_current_view->right = t3f_virtual_display_width - t3f_current_view->left;
		}
		t3f_current_view->need_update = false;
	}
	al_copy_transform(&t3f_current_transform, &t3f_current_view->transform);
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
