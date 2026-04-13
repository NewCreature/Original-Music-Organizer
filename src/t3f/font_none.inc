/* empty font engine */

static int font_engine_get_text_width_none(const void * font, const char * text)
{
  return 0;
}

static int font_engine_get_font_height_none(const void * font)
{
	return 0;
}

static void font_engine_draw_glyph_none(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int codepoint)
{
}

static int font_engine_get_glyph_width_none(const void * font, int codepoint)
{
  return 0;
}

static bool font_engine_get_glyph_dimensions_none(const void * font, int codepoint, int * bbx, int * bby, int * bbw, int * bbh)
{
  return 0;
}

static int font_engine_get_glyph_advance_none(const void * font, int codepoint1, int codepoint2)
{
  return 0;
}

static void font_engine_draw_text_none(const void * font, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, char const * text)
{
}

static void font_engine_draw_textf_none(const void * font, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
}

static void * font_engine_load_font_f_none(const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	return NULL;
}

static void font_engine_destroy_font_none(void * font)
{
}
