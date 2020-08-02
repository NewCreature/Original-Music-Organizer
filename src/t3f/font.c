#include "t3f.h"
#include "font.h"
#include "draw.h"

/* make "magic pink" transparent and grays different levels of alpha */
static void t3f_convert_grey_to_alpha(ALLEGRO_BITMAP * bitmap)
{
	int x, y;
	unsigned char ir, ig, ib, ia;
	ALLEGRO_COLOR pixel;
	ALLEGRO_STATE old_state;

	if(!al_lock_bitmap(bitmap, al_get_bitmap_format(bitmap), 0))
	{
		return;
	}

	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(bitmap);

	for(y = 0; y < al_get_bitmap_height(bitmap); y++)
	{
		for(x = 0; x < al_get_bitmap_width(bitmap); x++)
		{
			pixel = al_get_pixel(bitmap, x, y);
			al_unmap_rgba(pixel, &ir, &ig, &ib, &ia);
			if(ir == 255 && ig == 0 && ib == 255)
			{
				pixel = al_map_rgba(0, 0, 0, 0);
				al_put_pixel(x, y, pixel);
			}
			else if(ia > 0 && !(ir == 255 && ig == 255 && ib == 0))
			{
				pixel = al_map_rgba(ir, ir, ir, ir);
				al_put_pixel(x, y, pixel);
			}
		}
	}

	al_restore_state(&old_state);
	al_unlock_bitmap(bitmap);
}

/* loads the old anti-aliased fonts created with TTF2PCX */
ALLEGRO_FONT * t3f_load_bitmap_font(const char * fn)
{
	ALLEGRO_BITMAP * fimage;
	ALLEGRO_FONT * fp;
	ALLEGRO_STATE old_state;
	int ranges[] = {32, 126};

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	fimage = al_load_bitmap(fn);
	if(!fimage)
	{
		return NULL;
	}
	t3f_convert_grey_to_alpha(fimage);
	al_restore_state(&old_state);
	fp = al_grab_font_from_bitmap(fimage, 1, ranges);
	al_destroy_bitmap(fimage);
	return fp;
}

static bool t3f_font_file_is_true_type(const char * fn)
{
	ALLEGRO_PATH * pp;
	const char * ext;
	bool ret = false;

	pp = al_create_path(fn);
	if(pp)
	{
		ext = al_get_path_extension(pp);
		if(!strcasecmp(ext, ".ttf"))
		{
			ret = true;
		}
	}
	al_destroy_path(pp);
	return ret;
}

/* detect bitmap/ttf and load accordingly */
T3F_FONT * t3f_generate_font(const char * fn, int size, int flags)
{
	T3F_FONT * fp;
	ALLEGRO_FONT * normal_font = NULL;
	ALLEGRO_FONT * small_font = NULL;
	ALLEGRO_FONT * font;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	int ox = 1, oy = 1;
	int w, h;
	char buf[2] = {0};
	int i, j;
	int space;
	int cx, cy, cw, ch;
	int page_size = 256;

	if(flags & T3F_FONT_OUTLINE)
	{
		space = 3;
	}
	else
	{
		space = 1;
	}

	fp = al_malloc(sizeof(T3F_FONT));
	if(fp)
	{
		if(t3f_font_file_is_true_type(fn))
		{
			normal_font = al_load_ttf_font(fn, size, 0);
			small_font = al_load_ttf_font(fn, size / 2, 0);
		}
		else
		{
			normal_font = t3f_load_bitmap_font(fn);
			small_font = t3f_load_bitmap_font(fn);
		}
		if(normal_font && small_font)
		{
			fp->character_sheet = al_create_bitmap(page_size, page_size);
			if(fp->character_sheet)
			{
				al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
				al_set_target_bitmap(fp->character_sheet);
				al_identity_transform(&identity);
				al_use_transform(&identity);
				al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
				for(i = 0; i < 256; i++)
				{
					font = normal_font;
					buf[0] = i;
					w = al_get_text_width(font, buf);
					if(w <= 0 && i == 179)
					{
						buf[0] = '3';
						font = small_font;
						w = al_get_text_width(font, buf);
					}
					w += space;
					h = al_get_font_line_height(font) + space;
					al_get_text_dimensions(font, buf, &cx, &cy, &cw, &ch);
					if(cx < 0)
					{
						ox -= cx;
						w -= cx;
					}
					if(ox + w > al_get_bitmap_width(fp->character_sheet))
					{
						ox = 1;
						oy += h + 1;

						/* start over with larger page size if font doesn't fit */
						if(oy + h >= page_size)
						{
							al_destroy_bitmap(fp->character_sheet);
							page_size *= 2;
							fp->character_sheet = al_create_bitmap(page_size, page_size);
							al_set_target_bitmap(fp->character_sheet);
							al_identity_transform(&identity);
							al_use_transform(&identity);
							al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
							ox = 1;
							oy = 1;
							for(j = 0; j < i - 1; j++)
							{
								al_destroy_bitmap(fp->character[j].bitmap);
							}
							i = -1;
						}
					}
					if(i >= 0) // -1 == restarted with larger page
					{
						if(flags & T3F_FONT_OUTLINE)
						{
							al_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 1.0), ox + 1, oy, 0, buf);
							al_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 1.0), ox + 1, oy + 2, 0, buf);
							al_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 1.0), ox, oy + 1, 0, buf);
							al_draw_text(font, al_map_rgba_f(0.0, 0.0, 0.0, 1.0), ox + 2, oy + 1, 0, buf);
						}
						al_draw_text(font, al_map_rgba_f(1.0, 1.0, 1.0, 1.0), ox + 1, oy + 1, 0, buf);
						fp->character[i].x = ox + 1;
						fp->character[i].y = oy;
						fp->character[i].width = w - 1;
						fp->character[i].height = h - 1;
						fp->character[i].bitmap = al_create_sub_bitmap(fp->character_sheet, fp->character[i].x, fp->character[i].y, fp->character[i].width, fp->character[i].height);
						if(!fp->character[i].bitmap)
						{
							printf("could not create sub-bitmap\n");
							return NULL;
						}
						ox += w + 1;
					}
				}
				al_restore_state(&old_state);
				al_destroy_font(normal_font);
				al_destroy_font(small_font);
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			printf("font did not load\n");
			return NULL;
		}
		if(flags & T3F_FONT_OUTLINE)
		{
			fp->adjust = 1.0;
		}
		else
		{
			fp->adjust = 0.0;
		}
		fp->scale = 1.0;
	}
	return fp;
}

void t3f_destroy_font(T3F_FONT * fp)
{
	int i;

	for(i = 0; i < 256; i++)
	{
		al_destroy_bitmap(fp->character[i].bitmap);
	}
	al_destroy_bitmap(fp->character_sheet);
	al_free(fp);
}

T3F_FONT * t3f_load_font(const char * fn, int flags)
{
	T3F_FONT * fp;
	ALLEGRO_PATH * pp;
	ALLEGRO_CONFIG * cp;
	int i;
	char buf[64] = {0};
	const char * val;

	fp = al_malloc(sizeof(T3F_FONT));
	if(fp)
	{
		pp = al_create_path(fn);
		if(pp)
		{
			fp->character_sheet = al_load_bitmap(al_path_cstr(pp, '/'));
			if(fp->character_sheet)
			{
				al_set_path_extension(pp, ".ini");
				cp = al_load_config_file(al_path_cstr(pp, '/'));
				if(cp)
				{
					val = al_get_config_value(cp, "Settings", "adjust");
					if(val)
					{
						fp->adjust = atof(val);
					}
					val = al_get_config_value(cp, "Settings", "scale");
					if(val)
					{
						fp->scale = atof(val);
					}
					for(i = 0; i < T3F_FONT_MAX_CHARACTERS; i++)
					{
						snprintf(buf, 64, "glyph %d", i);
						val = al_get_config_value(cp, buf, "x");
						if(val)
						{
							fp->character[i].x = atoi(val);
						}
						val = al_get_config_value(cp, buf, "y");
						if(val)
						{
							fp->character[i].y = atoi(val);
						}
						val = al_get_config_value(cp, buf, "width");
						if(val)
						{
							fp->character[i].width = atoi(val);
						}
						val = al_get_config_value(cp, buf, "height");
						if(val)
						{
							fp->character[i].height = atoi(val);
						}
						fp->character[i].bitmap = al_create_sub_bitmap(fp->character_sheet, fp->character[i].x, fp->character[i].y, fp->character[i].width, fp->character[i].height);
					}
					al_destroy_config(cp);
				}
			}
			al_destroy_path(pp);
		}
	}
	return fp;
}

bool t3f_save_font(T3F_FONT * fp, const char * fn)
{
	ALLEGRO_PATH * pp;
	ALLEGRO_CONFIG * cp;
	int i;
	char buf[2][64] = {{0}};
	bool ret = false;

	pp = al_create_path(fn);
	if(pp)
	{
		al_save_bitmap(al_path_cstr(pp, '/'), fp->character_sheet);
		cp = al_create_config();
		if(cp)
		{
			snprintf(buf[0], 64, "%f", fp->adjust);
			al_set_config_value(cp, "Settings", "adjust", buf[0]);
			snprintf(buf[0], 64, "%f", fp->scale);
			al_set_config_value(cp, "Settings", "scale", buf[0]);
			for(i = 0; i < T3F_FONT_MAX_CHARACTERS; i++)
			{
				snprintf(buf[0], 64, "glyph %d", i);
				snprintf(buf[1], 64, "%d", fp->character[i].x);
				al_set_config_value(cp, buf[0], "x", buf[1]);
				snprintf(buf[1], 64, "%d", fp->character[i].y);
				al_set_config_value(cp, buf[0], "y", buf[1]);
				snprintf(buf[1], 64, "%d", fp->character[i].width);
				al_set_config_value(cp, buf[0], "width", buf[1]);
				snprintf(buf[1], 64, "%d", fp->character[i].height);
				al_set_config_value(cp, buf[0], "height", buf[1]);
			}
			al_set_path_extension(pp, ".ini");
			al_save_config_file(al_path_cstr(pp, '/'), cp);
			ret = true;
			al_destroy_config(cp);
		}
		al_destroy_path(pp);
	}
	return ret;
}

float t3f_get_text_width(T3F_FONT * fp, const char * text)
{
	const unsigned char * utext = (const unsigned char *)text;
	float w = 0.0;
	unsigned int i;

	for(i = 0; i < strlen(text); i++)
	{
		w += ((float)al_get_bitmap_width(fp->character[(int)utext[i]].bitmap) - fp->adjust) * fp->scale;
	}
	w += 2.0; // include outline pixels
	return w;
}

float t3f_get_font_line_height(T3F_FONT * fp)
{
	float h = 0.0;

	h = ((float)al_get_bitmap_height(fp->character[' '].bitmap) - fp->adjust * 2.0) * fp->scale;
	return h;
}

/* need to make this not rely on spaces, sometimes there might be long stretches with no space which need to be broken up 'mid-word' */
void t3f_create_text_line_data(T3F_TEXT_LINE_DATA * lp, T3F_FONT * fp, float w, float tab, const char * text)
{
	char current_line[256];
	int current_line_pos = 0;
	int current_line_start_pos = 0;
	int last_space = -1;
	int i;
	float wi = w;

	lp->font = fp;
	lp->tab = tab;
	lp->lines = 0;
	strcpy(lp->line[lp->lines].text, "");
	if(strlen(text) < 1)
	{
		return;
	}

	/* divide text into lines */
	for(i = 0; i < (int)strlen(text); i++)
	{
		current_line[current_line_pos] = text[i];
		current_line[current_line_pos + 1] = '\0';
		if(text[i] == ' ')
		{
			last_space = current_line_pos;
		}
		current_line_pos++;

		/* copy line since we encountered a manual new line */
		if(text[i] == '\n')
		{
			current_line[current_line_pos] = '\0';
			strcpy(lp->line[lp->lines].text, current_line);
			current_line_start_pos += i + 1;
			lp->lines++;
			strcpy(lp->line[lp->lines].text, "");
			current_line_pos = 0;
			current_line[current_line_pos] = '\0';
			wi = w - tab;
		}

		/* copy this line to our list of lines because it is long enough */
		else if(t3f_get_text_width(fp, current_line) > wi)
		{
			current_line[last_space] = '\0';
			strcpy(lp->line[lp->lines].text, current_line);
			current_line_start_pos += last_space + 1;
			while(text[i] != ' ' && i >= 0)
			{
				i--;
			}
			lp->lines++;
			strcpy(lp->line[lp->lines].text, "");
			current_line_pos = 0;
			current_line[current_line_pos] = '\0';
			wi = w - tab;
		}
	}
	strcpy(lp->line[lp->lines].text, current_line);
	lp->lines++;
}

void t3f_draw_text_lines(T3F_TEXT_LINE_DATA * lines, ALLEGRO_COLOR color, float x, float y, float z)
{
	int i;
	float px = x;
	float py = y;

	for(i = 0; i < lines->lines; i++)
	{
		t3f_draw_text(lines->font, color, px, py, z, 0.0, 0.0, 0, lines->line[i].text);
		px = x + lines->tab;
		py += t3f_get_font_line_height(lines->font);
	}
}

void t3f_draw_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * text)
{
	const unsigned char * utext = (const unsigned char *)text;
	T3F_TEXT_LINE_DATA line_data;
	unsigned int i;
	float pos = x - fp->adjust * fp->scale;
	float posy = y - fp->adjust * fp->scale;
	float fw, fh;
	bool held;

	if(strlen(text) < 1)
	{
		return;
	}
	held = al_is_bitmap_drawing_held();
	if(!held)
	{
		al_hold_bitmap_drawing(true);
	}
	if(flags & T3F_FONT_ALIGN_CENTER)
	{
		pos -= t3f_get_text_width(fp, text) / 2.0;
	}
	else if(flags & T3F_FONT_ALIGN_RIGHT)
	{
		pos -= t3f_get_text_width(fp, text);
	}
	if(w > 0.0)
	{
		t3f_create_text_line_data(&line_data, fp, w, tab, text);
		t3f_draw_text_lines(&line_data, color, x, y, z);
	}
	else
	{
		for(i = 0; i < strlen(text); i++)
		{
			fw = (float)al_get_bitmap_width(fp->character[(int)utext[i]].bitmap) * fp->scale;
			fh = (float)al_get_bitmap_height(fp->character[(int)utext[i]].bitmap) * fp->scale;
			if(utext[i] != '\n')
			{
				t3f_draw_scaled_bitmap(fp->character[(int)utext[i]].bitmap, color, pos, posy, z, fw, fh, 0);
				pos += fw - fp->adjust * fp->scale;
			}
//			pos += fw - ((fp->adjust * 2.0) * fp->scale);
//			pos += ((float)al_get_bitmap_width(fp->character[(int)text[i]]) - fp->adjust * 2.0) * fp->scale;
		}
	}
	if(!held)
	{
		al_hold_bitmap_drawing(false);
	}
}

void t3f_draw_textf(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	t3f_draw_text(vf, color, x, y, z, w, tab, flags, buf);
}
