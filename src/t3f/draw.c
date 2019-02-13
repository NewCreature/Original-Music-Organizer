#include "t3f.h"
#include "view.h"

void t3f_draw_bitmap(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2];

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(x, z);
	obj_x[1] = t3f_project_x(x + al_get_bitmap_width(bp), z);
	obj_y[0] = t3f_project_y(y, z);
	obj_y[1] = t3f_project_y(y + al_get_bitmap_height(bp), z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		al_draw_tinted_scaled_bitmap(bp, color, 0, 0, al_get_bitmap_width(bp), al_get_bitmap_height(bp), obj_x[0], obj_y[0], screen_w, screen_h, flags);
	}
}

void t3f_draw_rotated_bitmap(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2], obj_cx, obj_cy;
	float rx, ry;

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(x - cx, z);
	obj_x[1] = t3f_project_x(x - cx + al_get_bitmap_width(bp), z);
	obj_y[0] = t3f_project_y(y - cy, z);
	obj_y[1] = t3f_project_y(y - cy + al_get_bitmap_height(bp), z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;
	obj_cx = t3f_project_x(x, z);
	obj_cy = t3f_project_y(y, z);

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		rx = screen_w / al_get_bitmap_width(bp);
		ry = screen_h / al_get_bitmap_height(bp);
		al_draw_tinted_scaled_rotated_bitmap(bp, color, cx, cy, obj_cx, obj_cy, rx, ry, angle, flags);
	}
}

void t3f_draw_scaled_rotated_bitmap(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float cx, float cy, float x, float y, float z, float angle, float scale_x, float scale_y, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2], obj_cx, obj_cy;
	float rx, ry;

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(0, z);
	obj_x[1] = t3f_project_x((float)al_get_bitmap_width(bp) * scale_x, z);
	obj_y[0] = t3f_project_y(0, z);
	obj_y[1] = t3f_project_y((float)al_get_bitmap_height(bp) * scale_y, z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;
	obj_cx = t3f_project_x(x, z);
	obj_cy = t3f_project_y(y, z);

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		rx = screen_w / al_get_bitmap_width(bp);
		ry = screen_h / al_get_bitmap_height(bp);
		al_draw_tinted_scaled_rotated_bitmap(bp, color, cx, cy, obj_cx, obj_cy, rx, ry, angle, flags);
	}
}

void t3f_draw_scaled_bitmap(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, float w, float h, int flags)
{
	/* upper left and bottom right points in 3d */
	float obj_x[2], obj_y[2], obj_z[2];

	/* upper left and bottom right points in 2d */
	float screen_w, screen_h;

	obj_x[0] = t3f_project_x(x, z);
	obj_x[1] = t3f_project_x(x + w, z);
	obj_y[0] = t3f_project_y(y, z);
	obj_y[1] = t3f_project_y(y + h, z);
	obj_z[0] = z + t3f_current_view->virtual_width;
//	obj_z[1] = z + t3f_virtual_display_width;

	/* clip sprites at z = 0 */
	if(obj_z[0] > 0)
	{
		screen_w = obj_x[1] - obj_x[0];
		screen_h = obj_y[1] - obj_y[0];
		al_draw_tinted_scaled_bitmap(bp, color, 0, 0, al_get_bitmap_width(bp), al_get_bitmap_height(bp), obj_x[0], obj_y[0], screen_w, screen_h, flags);
	}
}
