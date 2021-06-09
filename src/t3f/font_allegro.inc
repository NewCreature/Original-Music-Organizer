/* Allegro font engine */

/* make "magic pink" transparent and grays different levels of alpha */
static void convert_grey_to_alpha(ALLEGRO_BITMAP * bitmap)
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

static void * load_bitmap_font_f(ALLEGRO_FILE * fp, const char * extension)
{
	ALLEGRO_BITMAP * bp;
	ALLEGRO_FONT * font = NULL;
	int ranges[] = {32, 126};

	bp = al_load_bitmap_f(fp, extension);
	if(bp)
	{
		convert_grey_to_alpha(bp);
		font = al_grab_font_from_bitmap(bp, 1, ranges);
		al_destroy_bitmap(bp);
	}
	return font;
}

static void * font_engine_load_font_f_allegro(const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	const char * extension = t3f_get_path_extension(fn);

	if(!strcasecmp(extension, ".ttf"))
	{
		return al_load_ttf_font_f(fp, fn, option, flags);
	}
	return load_bitmap_font_f(fp, extension);
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

static void font_engine_draw_text_allegro(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, char const * text)
{
	al_draw_text(font, color, x, y, flags, text);
}

static void font_engine_draw_textf_allegro(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	al_draw_text(font, color, x, y, flags, buf);
}
