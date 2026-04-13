/* T3F font engine */

typedef struct
{

	ALLEGRO_BITMAP * bitmap;
	int x, y, width, height;

} T3F_FONT_CHARACTER;

typedef struct
{

	ALLEGRO_FONT * source_font;
	ALLEGRO_BITMAP * character_sheet;
	T3F_FONT_CHARACTER character[T3F_FONT_MAX_CHARACTERS];
	float size_multiplier; // multiply the user-sepecified font size by this
	float scale;         // scale the rendering by this factor
	float outline_size;    // size of outline

} T3F_FONT_DATA;

static void _draw_scaled_character(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float z, float w, float h, int flags)
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

static ALLEGRO_FONT * load_bitmap_font(const char * fn)
{
	ALLEGRO_FONT * font = NULL;
	ALLEGRO_FILE * fp;
	const char * extension = t3f_get_path_extension(fn);

	fp = al_fopen(fn, "rb");
	if(fp)
	{
		font = load_bitmap_font_f(fp, extension);
		al_fclose(fp);
	}
	return font;
}

static void destroy_font_old(T3F_FONT_DATA * fp)
{
	int i;

	if(fp)
	{
		for(i = 0; i < 256; i++)
		{
			if(fp->character[i].bitmap)
			{
				al_destroy_bitmap(fp->character[i].bitmap);
			}
		}
		if(fp->character_sheet)
		{
			al_destroy_bitmap(fp->character_sheet);
		}
		if(fp->source_font)
		{
			al_destroy_font(fp->source_font);
		}
		al_free(fp);
	}
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

static void _draw_single_outline(ALLEGRO_BITMAP * bp, ALLEGRO_COLOR color, float x, float y, float scale)
{
	al_draw_tinted_bitmap(bp, color, x - sqrt(scale), y - sqrt(scale), 0);
	al_draw_tinted_bitmap(bp, color, x, y - scale, 0);
	al_draw_tinted_bitmap(bp, color, x + sqrt(scale), y - sqrt(scale), 0);
	al_draw_tinted_bitmap(bp, color, x - scale, y, 0);
	al_draw_tinted_bitmap(bp, color, x, y, 0);
	al_draw_tinted_bitmap(bp, color, x + scale, y, 0);
	al_draw_tinted_bitmap(bp, color, x - sqrt(scale), y + sqrt(scale), 0);
	al_draw_tinted_bitmap(bp, color, x, y + scale, 0);
	al_draw_tinted_bitmap(bp, color, x + sqrt(scale), y + sqrt(scale), 0);
}

static void _draw_outline_text(ALLEGRO_BITMAP * bp, ALLEGRO_FONT * font, ALLEGRO_COLOR color, ALLEGRO_COLOR outline_color, float outline_size, float x, float y, const char * text)
{
	ALLEGRO_BITMAP * temp_bp;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	int i;
	float sub_render;
	int sub_offset = 0;

	temp_bp = al_create_bitmap(al_get_bitmap_width(bp), al_get_bitmap_height(bp));
	if(temp_bp)
	{
		/* set up fractional render */
		sub_render = outline_size - (int)outline_size;
		if(sub_render > 0.05)
		{
			sub_offset = 1;
		}

		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
		al_identity_transform(&identity);

		al_set_target_bitmap(temp_bp);
		al_use_transform(&identity);
		al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));

		/* seed 'bloom' effect with initial text render */
		al_set_target_bitmap(bp);
		al_use_transform(&identity);
		al_draw_text(font, outline_color, x + outline_size + sub_offset, y + outline_size + sub_offset, 0, text);

		for(i = 0; i < (int)outline_size; i++)
		{
			al_set_target_bitmap(temp_bp);
			al_use_transform(&identity);
			_draw_single_outline(bp, color, x, y, 1.0);
			al_set_target_bitmap(bp);
			al_use_transform(&identity);
			al_draw_bitmap(temp_bp, 0, 0, 0);
		}
		if(sub_offset)
		{
			al_set_target_bitmap(temp_bp);
			al_use_transform(&identity);
			_draw_single_outline(bp, color, x, y, sub_render);
			al_set_target_bitmap(bp);
			al_use_transform(&identity);
			al_draw_bitmap(temp_bp, 0, 0, 0);
		}
		al_set_target_bitmap(bp);
		al_use_transform(&identity);
		al_draw_text(font, color, x + outline_size + sub_offset, y + outline_size + sub_offset, 0, text);
		al_destroy_bitmap(temp_bp);
		al_restore_state(&old_state);
	}
}

/* detect bitmap/ttf and load accordingly */
static T3F_FONT_DATA * generate_font(const char * fn, int size, float size_multiplier, float outline, ALLEGRO_COLOR outline_color)
{
	T3F_FONT_DATA * fp;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	int ox = 1, oy = 1;
	int w, h;
	char buf[2] = {0};
	int i, j;
	int space;
	int cx, cy, cw, ch;
	int page_size = 256;

	/* how much space between glyphs */
	space = 2.0 * (ceil(outline * size_multiplier) + 2.0);

	fp = al_malloc(sizeof(T3F_FONT_DATA));
	if(!fp)
	{
		goto fail;
	}
	memset(fp, 0, sizeof(T3F_FONT_DATA));
	if(t3f_font_file_is_true_type(fn))
	{
		fp->source_font = al_load_ttf_font(fn, (float)size * size_multiplier, 0);
	}
	else
	{
		fp->source_font = load_bitmap_font(fn);
	}
	if(!fp->source_font)
	{
		goto fail;
	}
	fp->outline_size = outline;
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
		buf[0] = i;
		w = al_get_text_width(fp->source_font, buf);
		w += space;
		h = al_get_font_line_height(fp->source_font) + space;
		al_get_text_dimensions(fp->source_font, buf, &cx, &cy, &cw, &ch);
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
			fp->character[i].x = ox;
			fp->character[i].y = oy;
			fp->character[i].width = w - 1;
			fp->character[i].height = h - 1;
			fp->character[i].bitmap = al_create_sub_bitmap(fp->character_sheet, fp->character[i].x, fp->character[i].y, fp->character[i].width, fp->character[i].height);
			if(!fp->character[i].bitmap)
			{
				printf("could not create sub-bitmap\n");
				goto fail;
			}
			_draw_outline_text(fp->character[i].bitmap, fp->source_font, t3f_color_white, outline_color, outline * size_multiplier, 0, 0, buf);
			ox += w + 1;
		}
	}
	al_restore_state(&old_state);

	fp->size_multiplier = size_multiplier;
	fp->scale = 1.0 / size_multiplier;
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

static T3F_FONT_DATA * load_font_old(const char * fn, int size, int flags)
{
	T3F_FONT_DATA * fp = NULL;
	ALLEGRO_PATH * pp;
	ALLEGRO_CONFIG * cp;
	const char * val;
	const char * source_path;
	bool generate = false;
	float outline = 0.0;
	float size_multiplier = 1.0;
	ALLEGRO_COLOR outline_color = t3f_color_black;
	char * base_fn;
	char * section;

	base_fn = strdup(fn);
	if(!base_fn)
	{
		goto fail;
	}
	section = strtok(base_fn, "#");
	section = strtok(NULL, "#");

	cp = al_load_config_file(base_fn);
	if(!cp)
	{
		goto fail;
	}

	pp = al_create_path(base_fn);
	if(!pp)
	{
		goto fail;
	}

	/* get source file */
	source_path = _get_config_value_fallback(cp, section, "Font", "filename");
	if(!source_path)
	{
		goto fail;
	}
	val = _get_config_value_fallback(cp, section, "Font", "outline");
	if(val)
	{
		outline = atof(val);
	}
	val = _get_config_value_fallback(cp, section, "Font", "outline_color");
	if(val)
	{
		outline_color = al_color_html(val);
	}
	generate = true;
	al_set_path_filename(pp, source_path);

	if(generate)
	{
		val = _get_config_value_fallback(cp, section, "Font", "size_multiplier");
		if(val)
		{
			size_multiplier = atof(val);
		}
		fp = generate_font(al_path_cstr(pp, '/'), size, size_multiplier, outline, outline_color);
		if(!fp)
		{
			goto fail;
		}
		val = _get_config_value_fallback(cp, section, "Font", "scale");
		if(val)
		{
			fp->scale = atof(val);
		}
	}
	free(base_fn);
	al_destroy_path(pp);
	al_destroy_config(cp);
	return fp;

	fail:
	{
		if(fp)
		{
			destroy_font_old(fp);
		}
		if(base_fn)
		{
			free(base_fn);
		}
		return NULL;
	}
}

void _t3f_font_engine_t3f_update(void * data)
{
	T3F_FONT * font = (T3F_FONT *)data;
	T3F_FONT_DATA * font_data = (T3F_FONT_DATA *)font->font;
	
	if(font_data->character_sheet)
	{
		al_convert_bitmap(font_data->character_sheet);
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
	return al_get_text_width(fp->source_font, text) * fp->scale;
}

float old_get_font_line_height(const T3F_FONT_DATA * fp)
{
	return al_get_font_line_height(fp->source_font) * fp->scale;
}

void old_draw_text(const T3F_FONT_DATA * fp, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, const char * text)
{
	const unsigned char * utext = (const unsigned char *)text;
	unsigned int i;
	int l;
	float pos = x - (fp->outline_size * fp->size_multiplier) * fp->scale;
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
		pos -= (old_get_text_width(fp, text) * scale) / 2.0;
	}
	else if(flags & T3F_FONT_ALIGN_RIGHT)
	{
		pos -= old_get_text_width(fp, text) * scale;
	}
	l = strlen(text);
	for(i = 0; i < l; i++)
	{
		/* draw the current character */
		fw = ((float)al_get_bitmap_width(fp->character[(int)utext[i]].bitmap) * fp->scale) * scale;
		fh = ((float)al_get_bitmap_height(fp->character[(int)utext[i]].bitmap) * fp->scale) * scale;
		if(utext[i] != '\n')
		{
			_draw_scaled_character(fp->character[(int)utext[i]].bitmap, color, pos, posy, z, fw, fh, 0);

			/* advance position correctly using */
			pos += (al_get_glyph_advance(fp->source_font, text[i], (i < l - 1) ? (text[i + 1]) : ALLEGRO_NO_KERNING) * fp->scale) * scale;
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
		_draw_scaled_character(font_data->character[codepoint].bitmap, color, x, y, z, al_get_bitmap_width(font_data->character[codepoint].bitmap), al_get_bitmap_height(font_data->character[codepoint].bitmap), codepoint);
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

static void font_engine_draw_text_t3f(const void * font, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, char const * text)
{
	old_draw_text(font, color, x, y, z, scale, flags, text);
}

static void font_engine_draw_textf_t3f(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	old_draw_text(font, color, x, y, z, 1.0, flags, buf);
}

static void * font_engine_load_font_f_t3f(const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	return load_font_old(fn, option, flags);
}

static void font_engine_destroy_font_t3f(void * font)
{
	destroy_font_old(font);
}
