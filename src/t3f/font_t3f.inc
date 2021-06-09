/* T3F font engine */

static ALLEGRO_FONT * load_bitmap_font(const char * fn)
{
	ALLEGRO_FONT * font = NULL;
	ALLEGRO_FILE * fp;
	const char * extension = t3f_get_path_extension(fn);

	fp = al_fopen(fn, "rb");
	if(fp)
	{
		font = load_bitmap_font_f(fp, "extension");
		al_fclose(fp);
	}
	return font;
}

static void destroy_font_old(T3F_FONT_DATA * fp)
{
	int i;

	for(i = 0; i < 256; i++)
	{
		al_destroy_bitmap(fp->character[i].bitmap);
	}
	al_destroy_bitmap(fp->character_sheet);
	al_free(fp);
}

/* T3F generated font engine */

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
static T3F_FONT_DATA * generate_font(const char * fn, int size, bool outline, ALLEGRO_COLOR outline_color)
{
	T3F_FONT_DATA * fp;
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
	const char * val;

	if(outline)
	{
		space = 3;
	}
	else
	{
		space = 1;
	}

	fp = al_malloc(sizeof(T3F_FONT_DATA));
	if(!fp)
	{
		goto fail;
	}
	if(t3f_font_file_is_true_type(fn))
	{
		normal_font = al_load_ttf_font(fn, size, 0);
		small_font = al_load_ttf_font(fn, size / 2, 0);
	}
	else
	{
		normal_font = load_bitmap_font(fn);
		small_font = load_bitmap_font(fn);
	}
	if(!normal_font || !small_font)
	{
		goto fail;
	}
	fp->character_sheet = al_create_bitmap(page_size, page_size);
	if(!fp->character_sheet)
	{
		goto fail;
	}
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
			if(outline)
			{
				al_draw_text(font, outline_color, ox + 1, oy, 0, buf);
				al_draw_text(font, outline_color, ox + 1, oy + 2, 0, buf);
				al_draw_text(font, outline_color, ox, oy + 1, 0, buf);
				al_draw_text(font, outline_color, ox + 2, oy + 1, 0, buf);
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
				goto fail;
			}
			ox += w + 1;
		}
	}
	al_restore_state(&old_state);
	al_destroy_font(normal_font);
	al_destroy_font(small_font);

	if(outline)
	{
		fp->adjust = 1.0;
	}
	else
	{
		fp->adjust = 0.0;
	}
	fp->scale = 1.0;
	return fp;

	fail:
	{
		if(small_font)
		{
			al_destroy_font(small_font);
		}
		if(normal_font)
		{
			al_destroy_font(normal_font);
		}
		if(fp)
		{
			destroy_font_old(fp);
		}
		return NULL;
	}
}

static T3F_FONT_DATA * load_font_old(const char * fn, int size, int flags)
{
	T3F_FONT_DATA * fp;
	ALLEGRO_PATH * pp;
	ALLEGRO_CONFIG * cp;
	int i;
	char buf[64] = {0};
	const char * val;
	const char * source_path;
	bool generate = false;
	bool outline = false;
	ALLEGRO_COLOR outline_color = t3f_color_black;

	cp = al_load_config_file(fn);
	if(!cp)
	{
		goto fail;
	}

	pp = al_create_path(fn);
	if(!pp)
	{
		goto fail;
	}

	/* get source file */
	source_path = al_get_config_value(cp, "T3F Font Generator", "source");
	if(!source_path)
	{
		source_path = al_get_config_value(cp, "T3F Font", "source");
		if(!source_path)
		{
			goto fail;
		}
	}
	else
	{
		val = al_get_config_value(cp, "T3F Font Generator", "outline");
		if(val)
		{
			if(!strcasecmp(val, "yes"))
			{
				outline = true;
			}
		}
		val = al_get_config_value(cp, "T3F Font Generator", "outline_color");
		if(val)
		{
			outline_color = al_color_html(val);
		}
		generate = true;
	}
	al_set_path_filename(pp, source_path);

	if(generate)
	{
		fp = generate_font(al_path_cstr(pp, '/'), size, outline, outline_color);
	}
	else
	{
		fp = al_malloc(sizeof(T3F_FONT_DATA));
		if(!fp)
		{
			goto fail;
		}
		fp->character_sheet = al_load_bitmap(al_path_cstr(pp, '/'));
		if(fp->character_sheet)
		{
			val = al_get_config_value(cp, "T3F Font", "adjust");
			if(val)
			{
				fp->adjust = atof(val);
			}
			val = al_get_config_value(cp, "T3F Font", "scale");
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
		}
	}
	al_destroy_path(pp);
	al_destroy_config(cp);
	return fp;

	fail:
	{
		if(fp)
		{
			destroy_font_old(fp);
		}
		return NULL;
	}
}

bool t3f_save_font(T3F_FONT_DATA * fp, const char * fn)
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

float old_get_text_width(const T3F_FONT_DATA * fp, const char * text)
{
	const unsigned char * utext = (const unsigned char *)text;
	float w = 0.0;
	unsigned int i;
	int iw;

	for(i = 0; i < strlen(text); i++)
	{
		w += ((float)al_get_bitmap_width(fp->character[(int)utext[i]].bitmap) - fp->adjust) * fp->scale;
	}
	w += 2.0; // include outline pixels
	return w;
}

float old_get_font_line_height(const T3F_FONT_DATA * fp)
{
	float h = 0.0;

	h = ((float)al_get_bitmap_height(fp->character[' '].bitmap) - fp->adjust * 2.0) * fp->scale;
	return h;
}

void old_draw_text(const T3F_FONT_DATA * fp, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * text)
{
	const unsigned char * utext = (const unsigned char *)text;
	T3F_TEXT_LINE_DATA line_data;
	unsigned int i;
	float pos = x;
	float posy = y;
	float fw, fh;
	bool held;

	held = al_is_bitmap_drawing_held();
	if(!held)
	{
		al_hold_bitmap_drawing(true);
	}
	if(flags & T3F_FONT_ALIGN_CENTER)
	{
		pos -= old_get_text_width(fp, text) / 2.0;
	}
	else if(flags & T3F_FONT_ALIGN_RIGHT)
	{
		pos -= old_get_text_width(fp, text);
	}
	for(i = 0; i < strlen(text); i++)
	{
		fw = (float)al_get_bitmap_width(fp->character[(int)utext[i]].bitmap) * fp->scale;
		fh = (float)al_get_bitmap_height(fp->character[(int)utext[i]].bitmap) * fp->scale;
		if(utext[i] != '\n')
		{
			t3f_draw_scaled_bitmap(fp->character[(int)utext[i]].bitmap, color, pos, posy, z, fw, fh, 0);
			pos += fw - fp->adjust * fp->scale;
		}
	}
	if(!held)
	{
		al_hold_bitmap_drawing(false);
	}
}

static int font_engine_get_text_width_t3f(const void * font, const char * text)
{
	return old_get_text_width(font, text);
}

static int font_engine_get_font_height_t3f(const void * font)
{
	return old_get_font_line_height(font);
}

static void font_engine_draw_glyph_t3f(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int codepoint)
{
	T3F_FONT_DATA * font_data = (T3F_FONT_DATA *)font;

	if(font_data->character[codepoint].bitmap)
	{
		t3f_draw_bitmap(font_data->character[codepoint].bitmap, color, x, y, z, codepoint);
	}
}

static int font_engine_get_glyph_width_t3f(const void * font, int codepoint)
{
	T3F_FONT_DATA * font_data = (T3F_FONT_DATA *)font;

	return al_get_bitmap_width(font_data->character[codepoint].bitmap);
}

static bool font_engine_get_glyph_dimensions_t3f(const void * font, int codepoint, int * bbx, int * bby, int * bbw, int * bbh)
{
	T3F_FONT_DATA * font_data = (T3F_FONT_DATA *)font;

	return al_get_bitmap_width(font_data->character[codepoint].bitmap);
}

static int font_engine_get_glyph_advance_t3f(const void * font, int codepoint1, int codepoint2)
{
	T3F_FONT_DATA * font_data = (T3F_FONT_DATA *)font;

	return al_get_bitmap_width(font_data->character[codepoint1].bitmap);
}

static void font_engine_draw_text_t3f(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, char const * text)
{
	old_draw_text(font, color, x, y, z, flags, text);
}

static void font_engine_draw_textf_t3f(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	old_draw_text(font, color, x, y, z, flags, buf);
}

static void * font_engine_load_font_f_t3f(const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	return load_font_old(fn, option, flags);
}

static void font_engine_destroy_font_t3f(void * font)
{
	destroy_font_old(font);
}
