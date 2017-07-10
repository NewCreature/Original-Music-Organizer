#include "t3f.h"
#include "gui.h"

static bool t3f_gui_left_clicked = 0;
T3F_GUI_DRIVER t3f_gui_allegro_driver;
static T3F_GUI_DRIVER * t3f_gui_current_driver = NULL;
static bool t3f_gui_check_hover_y(T3F_GUI * pp, int i, float y);
static float t3f_gui_hover_y;

static float allegro_get_element_width(T3F_GUI_ELEMENT * ep)
{
	switch(ep->type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			return al_get_text_width((ALLEGRO_FONT *)ep->aux_data, ep->data);
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			return al_get_bitmap_width(((ALLEGRO_BITMAP *)(ep->data)));
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
			return al_get_font_line_height((ALLEGRO_FONT *)ep->aux_data);
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			return al_get_bitmap_height(((ALLEGRO_BITMAP *)(ep->data)));
		}
	}
	return 0.0;
}

static void allegro_render_element(T3F_GUI * pp, int i, bool hover)
{
	int sx, sy;

	if(hover)
	{
		sx = -pp->element[i].sx * 2;
		sy = -pp->element[i].sy * 2;
	}
	else
	{
		sx = -pp->element[i].sx;
		sy = -pp->element[i].sy;
	}

	switch(pp->element[i].type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				if(!(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE) || t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y))
				{
					if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
					{
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox, pp->oy + pp->element[i].oy, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, pp->element[i].color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
					}
					else
					{
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox, pp->oy + pp->element[i].oy, 0, (char *)pp->element[i].data);
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, pp->element[i].color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, (char *)pp->element[i].data);
					}
				}
			}
			else
			{
				if(!(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE) || t3f_gui_check_hover_y(pp, i, t3f_gui_hover_y))
				{
					if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
					{
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, pp->element[i].color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, ALLEGRO_ALIGN_CENTRE, (char *)pp->element[i].data);
					}
					else
					{
						al_draw_text((ALLEGRO_FONT *)pp->element[i].aux_data, pp->element[i].color, pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0, (char *)pp->element[i].data);
					}
				}
			}
			break;
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
				{
					if(pp->element[i].data)
					{
						al_draw_tinted_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2, pp->oy + pp->element[i].oy - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2, 0);
						al_draw_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2 + sx, pp->oy + pp->element[i].oy - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2 + sy, 0);
					}
				}
				else
				{
					if(pp->element[i].data)
					{
						al_draw_tinted_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), al_map_rgba_f(0.0, 0.0, 0.0, 0.5), pp->ox + pp->element[i].ox, pp->oy + pp->element[i].oy, 0);
						al_draw_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0);
					}
				}
			}
			else
			{
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTRE)
				{
					if(pp->element[i].data)
					{
						al_draw_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2 + sx, pp->oy + pp->element[i].oy - al_get_bitmap_width(((ALLEGRO_BITMAP *)(pp->element[i].data))) / 2 + sy, 0);
					}
				}
				else
				{
					if(pp->element[i].data)
					{
						al_draw_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox + sx, pp->oy + pp->element[i].oy + sy, 0);
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
					al_free(pp->element[i].data);
					break;
				}
				case T3F_GUI_ELEMENT_IMAGE:
				{
					al_destroy_bitmap((ALLEGRO_BITMAP *)(pp->element[i].data));
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

int t3f_add_gui_image_element(T3F_GUI * pp, int (*proc)(void *, int, void *), void * bp, int ox, int oy, int flags)
{
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_IMAGE;
	pp->element[pp->elements].proc = proc;
	pp->element[pp->elements].data = bp;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->elements++;
	return 1;
}

int t3f_add_gui_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), char * text, void * fp, int ox, int oy, ALLEGRO_COLOR color, int flags)
{
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_TEXT;
	pp->element[pp->elements].proc = proc;
	if(flags & T3F_GUI_ELEMENT_COPY)
	{
		pp->element[pp->elements].data = al_malloc(strlen(text) + 1);
		memcpy(pp->element[pp->elements].data, text, strlen(text) + 1);
	}
	else
	{
		pp->element[pp->elements].data = text;
	}
	pp->element[pp->elements].aux_data = fp;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].color = color;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
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

void t3f_center_gui(T3F_GUI * pp, float oy, float my)
{
	int i;
	float top = 1000.0;
	float bottom = 0.0;
	float dheight = my - oy;
	float height;
	float offset;

	for(i = 0; i < pp->elements; i++)
	{
		if(pp->element[i].oy < top)
		{
			top = pp->element[i].oy;
		}
		if(pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]) > bottom)
		{
			bottom = pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]);
		}
	}
	height = bottom - top;
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
		if(x >= pp->ox + pp->element[i].ox && x < pp->ox + pp->element[i].ox + t3f_gui_current_driver->get_element_width(&pp->element[i]))
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
	if(y >= pp->oy + pp->element[i].oy && y < pp->oy + pp->element[i].oy + t3f_gui_current_driver->get_element_height(&pp->element[i]))
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

void t3f_process_gui(T3F_GUI * pp, void * data)
{
	int i;
	bool mouse_moved = false;
	bool touched = false;
	bool touching = false;
	int touch_id = 0;
	int x, y;
	float mouse_x = 0.0, mouse_y = 0.0;

	/* check if the mouse has been moved */
	t3f_get_mouse_mickeys(&x, &y, NULL);
	if(x != 0 || y != 0 || t3f_mouse_button[0])
	{
		mouse_x = t3f_mouse_x;
		mouse_y = t3f_mouse_y;
		mouse_moved = true;
	}

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
