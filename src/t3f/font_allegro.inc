/* Allegro font engine */

static void * load_bitmap_font_f(ALLEGRO_FILE * fp, const char * extension)
{
	ALLEGRO_BITMAP * bp;
	ALLEGRO_FONT * font = NULL;
	int ranges[] = {32, 126};

	bp = al_load_bitmap_f(fp, extension);
	if(bp)
	{
		font = al_grab_font_from_bitmap(bp, 1, ranges);
		al_destroy_bitmap(bp);
	}
	return font;
}

static void * font_engine_load_font_f_allegro(const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	const char * extension = t3f_get_path_extension(fn);
	bool close_file = false;
	void * ret = NULL;

	if(!fp)
	{
		fp = al_fopen(fn, "rb");
		if(!fp)
		{
			goto fail;
		}
		close_file = true;
	}
	if(!strcasecmp(extension, ".ttf") || !strcasecmp(extension, ".otf"))
	{
		ret = al_load_ttf_font_f(fp, fn, option, flags);
		close_file = false; // TTF files need to stay open
	}
	else
	{
		ret = load_bitmap_font_f(fp, extension);
	}
	if(close_file)
	{
		al_fclose(fp);
	}
	return ret;

	fail:
	{
		return NULL;
	}
}

static void font_engine_destroy_font_allegro(void * font)
{
	al_destroy_font(font);
}

static int font_engine_get_text_width_allegro(const void * font, const char * text)
{
	return al_get_text_width(font, text);
}

static int font_engine_get_font_height_allegro(const void * font)
{
	return al_get_font_line_height(font);
}

static void font_engine_draw_glyph_allegro(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int codepoint)
{
	al_draw_glyph(font, color, x, y, codepoint);
}

static int font_engine_get_glyph_width_allegro(const void * font, int codepoint)
{
	return al_get_glyph_width(font, codepoint);
}

static bool font_engine_get_glyph_dimensions_allegro(const void * font, int codepoint, int * bbx, int * bby, int * bbw, int * bbh)
{
	return al_get_glyph_dimensions(font, codepoint, bbx, bby, bbw, bbh);
}

static int font_engine_get_glyph_advance_allegro(const void * font, int codepoint1, int codepoint2)
{
	return al_get_glyph_advance(font, codepoint1, codepoint2);
}

static void font_engine_draw_text_allegro(const void * font, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, char const * text)
{
	int aflags = 0;
	if(flags & T3F_FONT_ALIGN_CENTER)
	{
		aflags |= ALLEGRO_ALIGN_CENTRE;
	}
	if(flags & T3F_FONT_ALIGN_RIGHT)
	{
		aflags |= ALLEGRO_ALIGN_RIGHT;
	}
	al_draw_text(font, color, x, y, aflags, text);
}

static void font_engine_draw_textf_allegro(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	font_engine_draw_text_allegro(font, color, x, y, 0, 1.0, flags, buf);
}
