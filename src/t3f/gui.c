#include "t3f.h"
#include "gui.h"

static bool t3f_gui_left_clicked = 0;
T3F_GUI_DRIVER t3f_gui_allegro_driver;
static T3F_GUI_DRIVER * t3f_gui_current_driver = NULL;
static bool t3f_gui_check_hover_y(T3F_GUI * pp, int i, float y);
static float t3f_gui_hover_y;
static float t3f_gui_mouse_x = 0.0;
static float t3f_gui_mouse_y = 0.0;

static float allegro_get_element_width(T3F_GUI_ELEMENT * ep)
{
	switch(ep->type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			return t3f_get_text_width(*((T3F_FONT **)ep->resource), ep->data);
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			return al_get_bitmap_width(*((ALLEGRO_BITMAP **)ep->resource));
		}
	}
	return 0.0;
}

static float allegro_get_element_height(T3F_GUI_ELEMENT * ep)
{
	switch(ep->type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			return t3f_get_font_line_height(*((T3F_FONT **)ep->resource));
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			return al_get_bitmap_height(*((ALLEGRO_BITMAP **)ep->resource));
		}
	}
	return 0.0;
}

static void allegro_render_element(T3F_GUI * pp, int i, bool hover)
{
	ALLEGRO_BITMAP * bitmap = NULL;
	T3F_FONT * font = NULL;
	ALLEGRO_COLOR color;
	int sx, sy;

	if(hover)
	{
		sx = pp->element[i].hx;
		sy = pp->element[i].hy;
		color = pp->element[i].active_color;
	}
	else
	{
		sx = 0;
		sy = 0;
		if(!(pp->element[i].flags & T3F_GUI_ELEMENT_STATIC))
		{
			color = pp->element[i].inactive_color;
		}
		else
		{
			color = pp->element[i].color;
		}
	}

	switch(pp->element[i].type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			font = *((T3F_FONT **)pp->element[i].resource);
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				if(!(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE) || t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y))
				{
					if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
					{
						t3f_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox + pp->element[i].sx, pp->oy + pp->element[i].oy + pp->element[i].sy, 0, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
						t3f_draw_text(font, color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
					}
					else
					{
						t3f_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox + pp->element[i].sx, pp->oy + pp->element[i].oy + pp->element[i].sy, 0, 0, (char *)pp->element[i].data);
						t3f_draw_text(font, color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, 0, (char *)pp->element[i].data);
					}
				}
			}
			else
			{
				if(!(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE) || t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y))
				{
					if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
					{
						t3f_draw_text(font, color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
					}
					else
					{
						t3f_draw_text(font, color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, 0, (char *)pp->element[i].data);
					}
				}
			}
			break;
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			bitmap = *((ALLEGRO_BITMAP **)pp->element[i].resource);
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
				{
					if(bitmap)
					{
						al_draw_tinted_bitmap(bitmap, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox - al_get_bitmap_width(bitmap) / 2, pp->oy + pp->element[i].oy - al_get_bitmap_height(bitmap) / 2, 0);
						al_draw_bitmap(bitmap, pp->ox + pp->element[i].ox - al_get_bitmap_width(bitmap) / 2 + sx, pp->oy + pp->element[i].oy - al_get_bitmap_height(bitmap) / 2 + sy, 0);
					}
				}
				else
				{
					if(bitmap)
					{
						al_draw_tinted_bitmap(bitmap, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox, pp->oy + pp->element[i].oy, 0);
						al_draw_bitmap(bitmap, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0);
					}
				}
			}
			else
			{
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
				{
					if(bitmap)
					{
						al_draw_bitmap(bitmap, pp->ox + pp->element[i].ox - al_get_bitmap_width(bitmap) / 2 + sx, pp->oy + pp->element[i].oy - al_get_bitmap_height(bitmap) / 2 + sy, 0);
					}
				}
				else
				{
					if(bitmap)
					{
						al_draw_bitmap(bitmap, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0);
					}
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void t3f_set_gui_driver(T3F_GUI_DRIVER * dp)
{
	if(dp == NULL)
	{
		t3f_gui_allegro_driver.get_element_width = allegro_get_element_width;
		t3f_gui_allegro_driver.get_element_height = allegro_get_element_height;
		t3f_gui_allegro_driver.render_element = allegro_render_element;
		t3f_gui_current_driver = &t3f_gui_allegro_driver;
	}
	else
	{
		t3f_gui_current_driver = dp;
	}
}

T3F_GUI * t3f_create_gui(int ox, int oy)
{
	T3F_GUI * lp;
	lp = al_malloc(sizeof(T3F_GUI));
	if(!lp)
	{
		return NULL;
	}
	lp->elements = 0;
	lp->ox = ox;
	lp->oy = oy;
	lp->hover_element = -1;
	lp->font_margin_top = 0;
	lp->font_margin_bottom = 0;
	lp->font_margin_left = 0;
	lp->font_margin_right = 0;
	return lp;
}

void t3f_destroy_gui(T3F_GUI * pp)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		if(pp->element[i].flags & T3F_GUI_ELEMENT_COPY)
		{
			switch(pp->element[i].type)
			{
				case T3F_GUI_ELEMENT_TEXT:
				{
					al_free(pp->element[i].allocated_data);
					break;
				}
				case T3F_GUI_ELEMENT_IMAGE:
				{
					t3f_destroy_resource(pp->element[i].resource);
					break;
				}
			}
			if(pp->element[i].description)
			{
				al_free(pp->element[i].description);
			}
		}
	}
	al_free(pp);
}

int t3f_add_gui_image_element(T3F_GUI * pp, int (*proc)(void *, int, void *), void ** bp, int ox, int oy, int flags)
{
	memset(&pp->element[pp->elements], 0, sizeof(T3F_GUI_ELEMENT));
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_IMAGE;
	pp->element[pp->elements].proc = proc;
	if(flags & T3F_GUI_ELEMENT_COPY)
	{
		t3f_clone_resource(pp->element[pp->elements].resource, *bp);
	}
	else
	{
		pp->element[pp->elements].resource = bp;
	}
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
	pp->element[pp->elements].hx = -2;
	pp->element[pp->elements].hy = -2;
	pp->elements++;
	return 1;
}

int t3f_add_gui_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), const char * text, void ** fp, int ox, int oy, ALLEGRO_COLOR color, int flags)
{
	memset(&pp->element[pp->elements], 0, sizeof(T3F_GUI_ELEMENT));
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_TEXT;
	pp->element[pp->elements].proc = proc;
	if(flags & T3F_GUI_ELEMENT_COPY)
	{
		pp->element[pp->elements].allocated_data = al_malloc(strlen(text) + 1);
		memcpy(pp->element[pp->elements].allocated_data, text, strlen(text) + 1);
		pp->element[pp->elements].data = pp->element[pp->elements].allocated_data;
	}
	else
	{
		pp->element[pp->elements].data = text;
	}
	pp->element[pp->elements].resource = fp;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].color = color;
	pp->element[pp->elements].inactive_color = color;
	pp->element[pp->elements].active_color = color;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
	pp->element[pp->elements].hx = -2;
	pp->element[pp->elements].hy = -2;
	pp->elements++;
	return 1;
}

int t3f_describe_last_gui_element(T3F_GUI * pp, char * text)
{
	if(pp->elements > 0)
	{
		if(pp->element[pp->elements - 1].flags & T3F_GUI_ELEMENT_COPY)
		{
			pp->element[pp->elements - 1].description = al_malloc(strlen(text) + 1);
			strcpy(pp->element[pp->elements - 1].description, text);
		}
		else
		{
			pp->element[pp->elements - 1].description = text;
		}
		return 1;
	}
	return 0;
}

int t3f_get_gui_width(T3F_GUI * pp)
{
	int i;
	int max_width = 0;
	int width;

	for(i = 0; i < pp->elements; i++)
	{
		width = t3f_gui_current_driver->get_element_width(&pp->element[i]);
		if(width > max_width)
		{
			max_width = width;
		}
	}
	return max_width;
}

int t3f_get_gui_height(T3F_GUI * pp, float * top)
{
	int i;
	float itop = 1000.0;
	float bottom = 0.0;

	for(i = 0; i < pp->elements; i++)
	{
		if(pp->element[i].oy < itop)
		{
			itop = pp->element[i].oy;
		}
		if(pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]) > bottom)
		{
			bottom = pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]);
		}
	}
	if(top)
	{
		*top = itop;
	}

	return bottom - itop;
}

void t3f_center_gui(T3F_GUI * pp, float oy, float my)
{
	float dheight = my - oy;
	float top;
	float height;
	float offset;

	height = t3f_get_gui_height(pp, &top);
	offset = oy + dheight / 2.0 - height / 2.0;
	pp->oy = offset - top;
}

void t3f_set_gui_shadow(T3F_GUI * pp, float x, float y)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		pp->element[i].sx = x;
		pp->element[i].sy = y;
	}
}

void t3f_set_gui_hover_lift(T3F_GUI * pp, float x, float y)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		pp->element[i].hx = x;
		pp->element[i].hy = y;
	}
}

void t3f_set_gui_element_interaction_colors(T3F_GUI * pp, ALLEGRO_COLOR inactive_color, ALLEGRO_COLOR active_color)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		pp->element[i].inactive_color = inactive_color;
		pp->element[i].active_color = active_color;
	}
}

static bool t3f_gui_check_hover_x(T3F_GUI * pp, int i, float x)
{
	if((pp->element[i].flags & T3F_GUI_ELEMENT_STATIC))
	{
		return false;
	}
	if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
	{
		if(x >= pp->ox + pp->element[i].ox - t3f_gui_current_driver->get_element_width(&pp->element[i]) / 2 && x < pp->ox + pp->element[i].ox + t3f_gui_current_driver->get_element_width(&pp->element[i]) / 2)
		{
			return true;
		}
	}
	else
	{
		if(x >= pp->ox + pp->element[i].ox + pp->font_margin_left && x < pp->ox + pp->element[i].ox + t3f_gui_current_driver->get_element_width(&pp->element[i]) - pp->font_margin_right)
		{
			return true;
		}
	}
	return false;
}

static bool t3f_gui_check_hover_y(T3F_GUI * pp, int i, float y)
{
	if((pp->element[i].flags & T3F_GUI_ELEMENT_STATIC))
	{
		return false;
	}
	if(y >= pp->oy + pp->element[i].oy + pp->font_margin_top && y < pp->oy + pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]) - pp->font_margin_bottom)
	{
		return true;
	}
	return false;
}

static bool t3f_gui_check_hover(T3F_GUI * pp, int i, float x, float y)
{
	return t3f_gui_check_hover_x(pp, i, x) && t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y);
}

void t3f_select_previous_gui_element(T3F_GUI * pp)
{
	while(1)
	{
		pp->hover_element--;
		if(pp->hover_element < 0)
		{
			pp->hover_element = pp->elements - 1;
		}
		if(!(pp->element[pp->hover_element].flags & T3F_GUI_ELEMENT_STATIC))
		{
			break;
		}
	}
}

void t3f_select_next_gui_element(T3F_GUI * pp)
{
	while(1)
	{
		pp->hover_element++;
		if(pp->hover_element >= pp->elements)
		{
			pp->hover_element = 0;
		}
		if(!(pp->element[pp->hover_element].flags & T3F_GUI_ELEMENT_STATIC))
		{
			break;
		}
	}
}

void t3f_activate_selected_gui_element(T3F_GUI * pp, void * data)
{
	if(pp->hover_element >= 0 && pp->hover_element < pp->elements)
	{
		if(pp->element[pp->hover_element].proc)
		{
			pp->element[pp->hover_element].proc(data, pp->hover_element, pp);
		}
	}
}

static bool check_mouse_moved(void)
{
	if(fabs(t3f_gui_mouse_x - t3f_mouse_x) < 0.5 && fabs(t3f_gui_mouse_y - t3f_mouse_y) < 0.5)
	{
		return false;
	}
	return true;
}

void t3f_process_gui(T3F_GUI * pp, void * data)
{
	int i;
	bool mouse_moved = false;
	bool touched = false;
	bool touching = false;
	int touch_id = 0;
	float mouse_x = 0.0, mouse_y = 0.0;

	/* check if the mouse has been moved */
	if(check_mouse_moved() || t3f_mouse_button[0])
	{
		mouse_x = t3f_mouse_x;
		mouse_y = t3f_mouse_y;
		mouse_moved = true;
	}
	t3f_gui_mouse_x = t3f_mouse_x;
	t3f_gui_mouse_y = t3f_mouse_y;

	if(t3f_mouse_button[0])
	{
		touch_id = 0;
	}
	for(i = 1; i < T3F_MAX_TOUCHES; i++)
	{
		if(t3f_touch[i].active)
		{
			mouse_x = t3f_touch[i].x;
			mouse_y = t3f_touch[i].y;
			mouse_moved = true;
			touching = true;
			break;
		}
		else if(t3f_touch[i].released)
		{
			mouse_x = t3f_touch[i].x;
			mouse_y = t3f_touch[i].y;
			touched = true;
			touch_id = i;
			break;
		}
	}
	if(pp)
	{
		if(mouse_moved || touched)
		{
			t3f_gui_hover_y = mouse_y;
			pp->hover_element = -1;
			for(i = 0; i < pp->elements; i++)
			{
				if(t3f_gui_check_hover(pp, i, mouse_x, mouse_y))
				{
					pp->hover_element = i;
					break;
				}
			}
		}
		else if(pp->hover_element >= 0)
		{
			t3f_gui_hover_y = pp->oy + pp->element[pp->hover_element].oy;
		}
		if((t3f_mouse_button[0] || touched || (touching && pp->element[pp->hover_element].flags & T3F_GUI_ELEMENT_ON_TOUCH)) && !t3f_gui_left_clicked && pp->hover_element >= 0)
		{
			t3f_activate_selected_gui_element(pp, data);
			t3f_gui_left_clicked = true;
			t3f_touch[touch_id].released = false;
		}
		if(!t3f_mouse_button[0] && !touched)
		{
			t3f_gui_left_clicked = false;
		}
	}
}

void t3f_render_gui_element(T3F_GUI * pp, int i, bool hover)
{
	if(!(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE) || t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y))
	{
		t3f_gui_current_driver->render_element(pp, i, hover);
	}
}

void t3f_render_gui(T3F_GUI * pp)
{
	int i;

	if(pp)
	{
		for(i = 0; i < pp->elements; i++)
		{
			t3f_render_gui_element(pp, i, i == pp->hover_element);
		}

		/* render the hover element last so it appears on top */
//		if(pp->hover_element >= 0 && pp->hover_element < pp->elements)
//		{
//			t3f_hyperlink_page_render_element(pp, i, true);
//		}
	}
}
