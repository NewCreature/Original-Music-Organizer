#include "t3f.h"
#include "gui.h"
#include "mouse.h"
#include "touch.h"

T3F_GUI_DRIVER t3f_gui_allegro_driver;
static T3F_GUI_DRIVER * t3f_gui_current_driver = NULL;
static bool t3f_gui_check_hover_y(T3F_GUI * pp, int i, float y);
static bool t3f_gui_check_hover_y_range(T3F_GUI * pp, int i, int j, float y);
static float t3f_gui_mouse_x = -100000;
static float t3f_gui_mouse_y = -100000;
static ALLEGRO_COLOR _t3f_gui_shadow_color;

static void allegro_get_element_edges(T3F_GUI * pp, int i, int * left, int * top, int * right, int * bottom)
{
	int width = 0, height = 0;
	switch(pp->element[i].type)
	{
		case T3F_GUI_ELEMENT_TEXT:
		{
			width = t3f_get_text_width((T3F_FONT *)pp->element[i].resource, pp->element[i].data);
			height = t3f_get_font_line_height((T3F_FONT *)pp->element[i].resource);
			break;
		}
		case T3F_GUI_ELEMENT_MULTILINE_TEXT:
		{
			T3F_TEXT_LINES * text_lines = pp->element[i].allocated_data;
			T3F_TEXT_LINE * current_line = text_lines->line;
			int line_count = 0;
			int current_width;
			int max_width = 0;

			while(current_line)
			{
				current_width = t3f_get_text_width((T3F_FONT *)pp->element[i].resource, current_line->text);
				if(current_width > max_width)
				{
					max_width = current_width;
				}
				line_count++;
				current_line = current_line->next_line;
			}
			if(line_count > pp->element[i].option)
			{
				line_count = pp->element[i].option;
			}
			width = max_width;
			height = t3f_get_font_line_height((T3F_FONT *)pp->element[i].resource) * line_count;
			break;
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			width = ((T3F_BITMAP *)(pp->element[i].resource))->target_width;
			height = ((T3F_BITMAP *)(pp->element[i].resource))->target_height;
			break;
		}
		case T3F_GUI_ELEMENT_ANIMATION:
		{
			width = ((T3F_ANIMATION *)(pp->element[i].resource))->data->frame[0]->width;
			height = ((T3F_ANIMATION *)(pp->element[i].resource))->data->frame[0]->height;
			break;
		}
	}
	if(left)
	{
		*left = pp->element[i].ox;
		if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
		{
			*left -= width / 2;
		}
	}
	if(top)
	{
		*top = pp->element[i].oy;
		if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
		{
			*top -= height / 2;
		}
	}
	if(right)
	{
		*right = pp->element[i].ox + width;
		if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
		{
			*right -= width / 2;
		}
	}
	if(bottom)
	{
		*bottom = pp->element[i].oy + height;
		if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
		{
			*bottom -= height / 2;
		}
	}
}

static void allegro_render_element(T3F_GUI * pp, int i, bool hover, int flags)
{
	T3F_ANIMATION * animation = NULL;
	T3F_BITMAP * bitmap = NULL;
	T3F_FONT * font = NULL;
	ALLEGRO_COLOR color;
	int x, y, render_flags = 0;
	int hx, hy;

	if(hover)
	{
		hx = pp->element[i].hx;
		hy = pp->element[i].hy;
		color = pp->element[i].active_color;
	}
	else
	{
		hx = 0;
		hy = 0;
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
			font = (T3F_FONT *)pp->element[i].resource;
			x = pp->ox + pp->element[i].ox;
			if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
			{
				render_flags = T3F_FONT_ALIGN_CENTER;
			}
			y = pp->oy + pp->element[i].oy;
			if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
			{
				y -= t3f_get_font_line_height(font) / 2;
			}
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				t3f_draw_text(font, _t3f_gui_shadow_color, x + pp->element[i].sx, y + pp->element[i].sy, 0, render_flags, (char *)pp->element[i].data);
			}
			t3f_draw_text(font, color, x + hx, y + hy, 0, render_flags, (char *)pp->element[i].data);
			break;
		}
		case T3F_GUI_ELEMENT_MULTILINE_TEXT:
		{
			T3F_TEXT_LINES * text_lines = pp->element[i].allocated_data;
			T3F_TEXT_LINE * current_line = text_lines->line;
			int line_count = 0;
			bool elipsis = false;

			font = (T3F_FONT *)pp->element[i].resource;
			x = pp->ox + pp->element[i].ox;
			if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
			{
				render_flags = T3F_FONT_ALIGN_CENTER;
			}
			y = pp->oy + pp->element[i].oy;
			while(current_line)
			{
				line_count++;
				current_line = current_line->next_line;
			}
			if(line_count > pp->element[i].option)
			{
				line_count = pp->element[i].option;
				elipsis = true;
			}
			if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
			{
				y -= (t3f_get_font_line_height(font) * line_count) / 2;
			}
			current_line = text_lines->line;
			for(i = 0; i < line_count - 1; i++)
			{
				if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
				{
					t3f_draw_text(font, _t3f_gui_shadow_color, x + pp->element[i].sx, y + pp->element[i].sy, 0, render_flags, current_line->text);
				}
				t3f_draw_text(font, color, x + hx, y + hy, 0, render_flags, current_line->text);
				y += t3f_get_font_line_height(font);
				current_line = current_line->next_line;
			}
			if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
			{
				t3f_draw_textf(font, _t3f_gui_shadow_color, x + pp->element[i].sx, y + pp->element[i].sy, 0, render_flags, "%s%s", current_line->text, elipsis ? "..." : "");
			}
			t3f_draw_textf(font, color, x + hx, y + hy, 0, render_flags, "%s%s", current_line->text, elipsis ? "..." : "");
			break;
		}
		case T3F_GUI_ELEMENT_IMAGE:
		{
			bitmap = (T3F_BITMAP *)pp->element[i].resource;
			if(bitmap)
			{
				x = pp->ox + pp->element[i].ox;
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
				{
					x -= bitmap->target_width / 2;
				}
				y = pp->oy + pp->element[i].oy;
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
				{
					y -= bitmap->target_height / 2;
				}
				if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
				{
					t3f_draw_bitmap(bitmap, _t3f_gui_shadow_color, x + pp->element[i].sx, y + pp->element[i].sy, 0, 0);
				}
				t3f_draw_bitmap(bitmap, color, x + hx, y + hy, 0, 0);
			}
			break;
		}
		case T3F_GUI_ELEMENT_ANIMATION:
		{
			animation = (T3F_ANIMATION *)pp->element[i].resource;
			if(animation)
			{
				x = pp->ox + pp->element[i].ox;
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_H)
				{
					x -= animation->data->frame[0]->width / 2;
				}
				y = pp->oy + pp->element[i].oy;
				if(pp->element[i].flags & T3F_GUI_ELEMENT_CENTER_V)
				{
					y -= animation->data->frame[0]->height / 2;
				}
				if(pp->element[i].flags & T3F_GUI_ELEMENT_SHADOW)
				{
					t3f_draw_animation(animation, _t3f_gui_shadow_color, pp->tick, x + pp->element[i].sx, y + pp->element[i].sy, 0, 0);
				}
				t3f_draw_animation(animation, color, pp->tick, x + hx, y + hy, 0, 0);
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
		t3f_gui_allegro_driver.get_element_edges = allegro_get_element_edges;
		t3f_gui_allegro_driver.render_element = allegro_render_element;
		t3f_gui_current_driver = &t3f_gui_allegro_driver;
	}
	else
	{
		t3f_gui_current_driver = dp;
	}
	_t3f_gui_shadow_color = al_map_rgba_f(0.0, 0.0, 0.0, 0.5);
}

void t3f_set_gui_shadow_color(ALLEGRO_COLOR color)
{
	_t3f_gui_shadow_color = color;
}

T3F_GUI * t3f_create_gui(int ox, int oy, void * data)
{
	T3F_GUI * lp;
	lp = al_malloc(sizeof(T3F_GUI));
	if(!lp)
	{
		return NULL;
	}
	memset(lp, 0, sizeof(T3F_GUI));
	lp->data = data;
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
		if((pp->element[i].flags & T3F_GUI_ELEMENT_COPY) || (pp->element[i].flags & T3F_GUI_ELEMENT_OWN))
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
					t3f_destroy_bitmap((T3F_BITMAP *)pp->element[i].resource);
					break;
				}
			}
		}
		if((pp->element[i].description_flags & T3F_GUI_ELEMENT_COPY) || (pp->element[i].description_flags & T3F_GUI_ELEMENT_OWN))
		{
			if(pp->element[i].description)
			{
				free(pp->element[i].description);
			}
		}
	}
	al_free(pp);
}

T3F_GUI_ELEMENT * t3f_add_gui_image_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_BITMAP * bitmap, ALLEGRO_COLOR color, int ox, int oy, int flags)
{
	memset(&pp->element[pp->elements], 0, sizeof(T3F_GUI_ELEMENT));
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_IMAGE;
	pp->element[pp->elements].proc = proc;
	if(flags & T3F_GUI_ELEMENT_COPY)
	{
		pp->element[pp->elements].resource = t3f_clone_bitmap(bitmap);
	}
	else
	{
		pp->element[pp->elements].resource = bitmap;
	}
	pp->element[pp->elements].color = color;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
	pp->element[pp->elements].hx = -2;
	pp->element[pp->elements].hy = -2;
	pp->element[pp->elements].show_element_start = pp->elements;
	pp->element[pp->elements].show_element_end = pp->elements;
	pp->elements++;
	return &pp->element[pp->elements - 1];
}

T3F_GUI_ELEMENT * t3f_add_gui_animation_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_ANIMATION * animation, ALLEGRO_COLOR color, int ox, int oy, int flags)
{
	memset(&pp->element[pp->elements], 0, sizeof(T3F_GUI_ELEMENT));
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_ANIMATION;
	pp->element[pp->elements].proc = proc;
	if(flags & T3F_GUI_ELEMENT_COPY)
	{
		pp->element[pp->elements].resource = t3f_clone_animation(animation);
	}
	else
	{
		pp->element[pp->elements].resource = animation;
	}
	pp->element[pp->elements].color = color;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
	pp->element[pp->elements].hx = -2;
	pp->element[pp->elements].hy = -2;
	pp->element[pp->elements].show_element_start = pp->elements;
	pp->element[pp->elements].show_element_end = pp->elements;
	pp->elements++;
	return &pp->element[pp->elements - 1];
}

T3F_GUI_ELEMENT * t3f_add_gui_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_FONT * font, ALLEGRO_COLOR color, const char * text, int ox, int oy, int flags)
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
	pp->element[pp->elements].resource = font;
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
	pp->element[pp->elements].show_element_start = pp->elements;
	pp->element[pp->elements].show_element_end = pp->elements;
	pp->elements++;
	return &pp->element[pp->elements - 1];
}

T3F_GUI_ELEMENT * t3f_add_gui_multiline_text_element(T3F_GUI * pp, int (*proc)(void *, int, void *), T3F_FONT * font, ALLEGRO_COLOR color, const char * text, int ox, int oy, int max_width, int max_lines, int flags)
{
	T3F_TEXT_LINES * text_lines = NULL;

	memset(&pp->element[pp->elements], 0, sizeof(T3F_GUI_ELEMENT));
	pp->element[pp->elements].type = T3F_GUI_ELEMENT_MULTILINE_TEXT;
	pp->element[pp->elements].proc = proc;
	pp->element[pp->elements].allocated_data = al_malloc(sizeof(T3F_TEXT_LINES));
	if(!pp->element[pp->elements].allocated_data)
	{
		goto fail;
	}
	text_lines = pp->element[pp->elements].allocated_data;
	if(!t3f_init_text_lines(text_lines))
	{
		goto fail;
	}
	if(!t3f_create_text_lines(text_lines, font, max_width, 0, text))
	{
		goto fail;
	}
	pp->element[pp->elements].resource = font;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].option = max_lines;
	pp->element[pp->elements].color = color;
	pp->element[pp->elements].inactive_color = color;
	pp->element[pp->elements].active_color = color;
	pp->element[pp->elements].flags = flags;
	pp->element[pp->elements].description = NULL;
	pp->element[pp->elements].sx = 2;
	pp->element[pp->elements].sy = 2;
	pp->element[pp->elements].hx = -2;
	pp->element[pp->elements].hy = -2;
	pp->element[pp->elements].show_element_start = pp->elements;
	pp->element[pp->elements].show_element_end = pp->elements;
	pp->elements++;

	return &pp->element[pp->elements - 1];

	fail:
	{
		if(text_lines)
		{
			t3f_free_text_lines(text_lines);
		}
		if(pp->element[pp->elements].allocated_data)
		{
			free(pp->element[pp->elements].allocated_data);
		}
		return NULL;
	}
}

int t3f_describe_last_gui_element(T3F_GUI * pp, char * text, int flags)
{
	if(pp->elements > 0)
	{
		pp->element[pp->elements - 1].description_flags = flags;
		if(pp->element[pp->elements - 1].description_flags & T3F_GUI_ELEMENT_COPY)
		{
			pp->element[pp->elements - 1].description = strdup(text);
		}
		else
		{
			pp->element[pp->elements - 1].description = text;
		}
		return 1;
	}
	return 0;
}

void t3f_set_gui_nav_procs(T3F_GUI * pp, int (*up_proc)(void *, int, void *), int (*down_proc)(void *, int, void *), int (*left_proc)(void *, int, void *), int (*right_proc)(void *, int, void *))
{
	if(pp->elements > 0)
	{
		pp->element[pp->elements - 1].up_proc = up_proc;
		pp->element[pp->elements - 1].down_proc = down_proc;
		pp->element[pp->elements - 1].left_proc = left_proc;
		pp->element[pp->elements - 1].right_proc = right_proc;
	}
}

void t3f_set_gui_show_element_range(T3F_GUI * pp, int start_element, int end_element)
{
	pp->element[pp->elements - 1].show_element_start = start_element;
	pp->element[pp->elements - 1].show_element_end = end_element;
}

int t3f_get_gui_width(T3F_GUI * pp)
{
	int i;
	int max_width = 0;
	int width;
	int left, right;

	for(i = 0; i < pp->elements; i++)
	{
		t3f_gui_current_driver->get_element_edges(pp, i, &left, NULL, &right, NULL);
		width = right - left;
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
	float itop = 10000.0;
	float bottom = 0.0;
	int edge_top, edge_bottom;

	for(i = 0; i < pp->elements; i++)
	{
		t3f_gui_current_driver->get_element_edges(pp, i, NULL, &edge_top, NULL, &edge_bottom);
		if(edge_top < itop)
		{
			itop = edge_top;
		}
		if(pp->element[i].oy + edge_bottom > bottom)
		{
			bottom = edge_bottom;
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
	int left, right;

	if((pp->element[i].flags & T3F_GUI_ELEMENT_NO_HOVER))
	{
		return false;
	}
	t3f_gui_current_driver->get_element_edges(pp, i, &left, NULL, &right, NULL);
	if(x >= pp->ox + left && x < pp->ox + right)
	{
		return true;
	}
	return false;
}

static bool t3f_gui_check_hover_y(T3F_GUI * pp, int i, float y)
{
	int top, bottom;

	if((pp->element[i].flags & T3F_GUI_ELEMENT_NO_HOVER))
	{
		return false;
	}
	t3f_gui_current_driver->get_element_edges(pp, i, NULL, &top, NULL, &bottom);
	if(y >= pp->oy + top && y < pp->oy + bottom)
	{
		return true;
	}
	return false;
}

static bool t3f_gui_check_hover_y_range(T3F_GUI * pp, int i, int j, float y)
{
	int top, bottom;

	if((pp->element[i].flags & T3F_GUI_ELEMENT_NO_HOVER))
	{
		return false;
	}
	t3f_gui_current_driver->get_element_edges(pp, i, NULL, &top, NULL, NULL);
	t3f_gui_current_driver->get_element_edges(pp, j, NULL, NULL, NULL, &bottom);
	if(y >= pp->oy + top && y < pp->oy + bottom)
	{
		return true;
	}
	return false;
}

static bool t3f_gui_check_hover(T3F_GUI * pp, int i, float x, float y)
{
	return t3f_gui_check_hover_x(pp, i, x) && t3f_gui_check_hover_y(pp, i, y);
}

bool t3f_select_hover_gui_element(T3F_GUI * pp, float x, float y)
{
	int i;

	pp->hover_element = -1;
	pp->hover_y = y;
	for(i = 0; i < pp->elements; i++)
	{
		if(t3f_gui_check_hover(pp, i, x, y))
		{
			pp->hover_element = i;
			return true;
		}
	}
	return false;
}

void t3f_select_previous_gui_element(T3F_GUI * pp)
{
	int loop_count = 0;

	while(1)
	{
		pp->hover_element--;
		if(pp->hover_element < 0)
		{
			pp->hover_element = pp->elements - 1;
			loop_count++;
			if(loop_count >= 2)
			{
				pp->hover_element = -1;
				break;
			}
		}
		if(!(pp->element[pp->hover_element].flags & T3F_GUI_ELEMENT_NO_NAV))
		{
			break;
		}
	}
}

void t3f_select_next_gui_element(T3F_GUI * pp)
{
	int loop_count = 0;

	while(1)
	{
		pp->hover_element++;
		if(pp->hover_element >= pp->elements)
		{
			pp->hover_element = 0;
			loop_count++;
			if(loop_count >= 2)
			{
				pp->hover_element = -1;
				break;
			}
		}
		if(!(pp->element[pp->hover_element].flags & T3F_GUI_ELEMENT_NO_NAV))
		{
			break;
		}
	}
}

bool t3f_select_gui_element_by_flags(T3F_GUI * pp, int flags)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		if(pp->element[i].flags & flags)
		{
			pp->hover_element = i;
			return true;
		}
	}
	return false;
}

bool t3f_select_gui_element_by_text(T3F_GUI * pp, const char * text)
{
	int i;

	for(i = 0; i < pp->elements; i++)
	{
		if(pp->element[i].type == T3F_GUI_ELEMENT_TEXT && !strcmp(pp->element[i].data, text))
		{
			pp->hover_element = i;
			return true;
		}
	}
	return false;
}

bool t3f_gui_up(T3F_GUI * pp, int flags)
{
	int element = pp->hover_element;
	int i;
	bool ret = false;

	if(flags)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(pp->element[i].flags & flags)
			{
				element = i;
				break;
			}
		}
	}

	if(element >= 0 && pp->element[element].up_proc)
	{
		pp->element[element].up_proc(pp->data, element, pp);
		ret = true;
	}
	else
	{
		t3f_select_previous_gui_element(pp);
		ret = true;
	}
	pp->input_mode = T3F_GUI_INPUT_NAV;

	return ret;
}

bool t3f_gui_down(T3F_GUI * pp, int flags)
{
	int element = pp->hover_element;
	int i;
	bool ret = false;

	if(flags)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(pp->element[i].flags & flags)
			{
				element = i;
				break;
			}
		}
	}

	if(element >= 0 && pp->element[element].down_proc)
	{
		pp->element[element].down_proc(pp->data, element, pp);
		ret = true;
	}
	else
	{
		t3f_select_next_gui_element(pp);
		ret = true;
	}
	pp->input_mode = T3F_GUI_INPUT_NAV;

	return ret;
}

bool t3f_gui_left(T3F_GUI * pp, int flags)
{
	int element = pp->hover_element;
	int i;
	bool ret = false;

	if(flags)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(pp->element[i].flags & flags)
			{
				element = i;
				break;
			}
		}
	}

	if(element >= 0)
	{
		if(pp->element[element].left_proc)
		{
			pp->element[element].left_proc(pp->data, element, pp);
			ret = true;
		}
	}
	pp->input_mode = T3F_GUI_INPUT_NAV;

	return ret;
}

bool t3f_gui_right(T3F_GUI * pp, int flags)
{
	int element = pp->hover_element;
	int i;
	bool ret = false;

	if(flags)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(pp->element[i].flags & flags)
			{
				element = i;
				break;
			}
		}
	}

	if(element >= 0)
	{
		if(pp->element[element].right_proc)
		{
			pp->element[element].right_proc(pp->data, element, pp);
			ret = true;
		}
	}
	pp->input_mode = T3F_GUI_INPUT_NAV;

	return ret;
}

void t3f_activate_selected_gui_element(T3F_GUI * pp)
{
	if(pp->hover_element >= 0 && pp->hover_element < pp->elements)
	{
		if(pp->element[pp->hover_element].proc)
		{
			pp->element[pp->hover_element].proc(pp->data, pp->hover_element, pp);
		}
	}
}

//static bool check_mouse_moved(void)
//{
//	if(fabs(t3f_gui_mouse_x - t3f_get_mouse_x()) < 0.01 && fabs(t3f_gui_mouse_y - t3f_get_mouse_y()) < 0.01)
//	{
//		return false;
//	}
//	return true;
//}

void t3f_reset_gui_input(T3F_GUI * pp)
{
	if(pp)
	{
		pp->hover_element = -1;
	}
	t3f_gui_mouse_x = t3f_get_mouse_x();
	t3f_gui_mouse_y = t3f_get_mouse_y();
}

bool t3f_process_gui(T3F_GUI * pp, int flags)
{
	int i;
	bool touched = false;
	float mouse_x = 0.0, mouse_y = 0.0;
	bool ret = false;

	if(t3f_flags & T3F_USE_MOUSE && !(flags & T3F_GUI_NO_MOUSE))
	{
		if(t3f_gui_mouse_x <= -100000)
		{
			t3f_reset_gui_input(pp);
		}
		mouse_x = t3f_get_mouse_x();
		mouse_y = t3f_get_mouse_y();
		t3f_gui_mouse_x = t3f_get_mouse_x();
		t3f_gui_mouse_y = t3f_get_mouse_y();
		if(t3f_mouse_button_pressed(0))
		{
			touched = true;
			t3f_use_mouse_button_press(0);
		}
	}

	if(t3f_flags & T3F_USE_TOUCH && !(flags & T3F_GUI_NO_TOUCH))
	{
		if(t3f_touch_pressed(1))
		{
			mouse_x = t3f_get_touch_x(1);
			mouse_y = t3f_get_touch_y(1);
			touched = true;
			t3f_use_touch_press(1);
		}
	}
	if(pp)
	{
		if(!(flags & T3F_GUI_NO_MOUSE) || touched)
		{
			pp->hover_y = mouse_y;
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
			pp->hover_y = pp->oy + pp->element[pp->hover_element].oy;
		}
		if(touched && pp->hover_element >= 0)
		{
			t3f_activate_selected_gui_element(pp);
			ret = true;
		}
		pp->tick++;
	}
	return ret;
}

void t3f_render_gui_element(T3F_GUI * pp, int i, bool hover, int flags)
{
	bool hide = false;

	if(!(flags & T3F_GUI_NO_HIDE))
	{
		if(pp->element[i].flags & T3F_GUI_ELEMENT_AUTOHIDE)
		{
			if(flags & T3F_GUI_NO_MOUSE)
			{
				if(pp->element[i].show_element_start != i || pp->element[i].show_element_end != i)
				{
					if(pp->hover_element < pp->element[i].show_element_start || pp->hover_element > pp->element[i].show_element_end)
					{
						hide = true;
					}
				}
			}
			else
			{
				if(!t3f_gui_check_hover_y(pp, i, pp->hover_y) && !t3f_gui_check_hover_y_range(pp, pp->element[i].show_element_start, pp->element[i].show_element_end, pp->hover_y))
				{
					hide = true;
				}
			}
		}
	}
	if(!hide)
	{
		t3f_gui_current_driver->render_element(pp, i, hover, flags);
	}
}

void t3f_render_gui(T3F_GUI * pp, int flags)
{
	int i;

	if(pp)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(i != pp->hover_element)
			{
				t3f_render_gui_element(pp, i, false, flags);
			}
		}
		if(pp->hover_element >= 0)
		{
			t3f_render_gui_element(pp, pp->hover_element, true, flags);
		}
	}
}
