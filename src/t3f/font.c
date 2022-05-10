#include "t3f.h"
#include "font.h"
#include "draw.h"
#include "file_utils.h"

/* include font engines */
#include "font_allegro.inc"
#include "font_t3f.inc"

static T3F_FONT_ENGINE font_engine[] =
{
	{
		font_engine_load_font_f_allegro,
		font_engine_destroy_font_allegro,

		font_engine_get_text_width_allegro,
		font_engine_get_font_height_allegro,
		font_engine_draw_glyph_allegro,
		font_engine_get_glyph_width_allegro,
		font_engine_get_glyph_dimensions_allegro,
		font_engine_get_glyph_advance_allegro,
		font_engine_draw_text_allegro,
		font_engine_draw_textf_allegro
	},
	{
		font_engine_load_font_f_t3f,
		font_engine_destroy_font_t3f,

		font_engine_get_text_width_t3f,
		font_engine_get_font_height_t3f,
		font_engine_draw_glyph_t3f,
		font_engine_get_glyph_width_t3f,
		font_engine_get_glyph_dimensions_t3f,
		font_engine_get_glyph_advance_t3f,
		font_engine_draw_text_t3f,
		font_engine_draw_textf_t3f
	},
	{
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL
	}
};

float t3f_get_text_width(T3F_FONT * fp, const char * text)
{
	return fp->engine->get_text_width(fp->font, text);
}

float t3f_get_font_line_height(T3F_FONT * fp)
{
	return fp->engine->get_font_height(fp->font);
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
		t3f_draw_text(lines->font, color, px, py, z, 0, lines->line[i].text);
		px = x + lines->tab;
		py += t3f_get_font_line_height(lines->font);
	}
}

void t3f_draw_multiline_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * text)
{
	T3F_TEXT_LINE_DATA line_data;
	float pos = x;
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
		if(!held)
		{
			al_hold_bitmap_drawing(false);
		}
	}
	else
	{
		fp->engine->draw_text(fp->font, color, x, y, z, flags, text);
		if(!held)
		{
			al_hold_bitmap_drawing(false);
		}
		return;
	}
	if(!held)
	{
		al_hold_bitmap_drawing(false);
	}
}

void t3f_draw_multiline_textf(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	t3f_draw_multiline_text(vf, color, x, y, z, w, tab, flags, buf);
}

void t3f_draw_text(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * text)
{
	t3f_draw_multiline_text(vf, color, x, y, z, 0, 0, flags, text);
}

void t3f_draw_textf(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, int flags, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	t3f_draw_text(vf, color, x, y, z, flags, buf);
}

void t3f_draw_glyph(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, int cp)
{
	vf->engine->draw_glyph(vf->font, color, x, y, z, cp);
}

int t3f_get_glyph_advance(T3F_FONT * vf, int cp1, int cp2)
{
	return vf->engine->get_glyph_advance(vf->font, cp1, cp2);
}

static int detect_font_type(const char * fn)
{
	const char * extension;

	extension = t3f_get_path_extension(fn);
	if(!strcmp(extension, ".ini"))
	{
		return T3F_FONT_TYPE_T3F;
	}
	return T3F_FONT_TYPE_ALLEGRO;
}

T3F_FONT * t3f_load_font_with_engine_f(T3F_FONT_ENGINE * engine, const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	T3F_FONT * font;

	font = malloc(sizeof(T3F_FONT));
	if(!font)
	{
		goto fail;
	}
	memset(font, 0, sizeof(T3F_FONT));

	font->engine = engine;
//	font->font = al_load_font(fn, option, flags);
	font->font = font->engine->load(fn, fp, option, flags);
	if(!font->font)
	{
		goto fail;
	}

	return font;

	fail:
	{
		t3f_destroy_font(font);
	}
	return NULL;
}

T3F_FONT * t3f_load_font_with_engine(T3F_FONT_ENGINE * engine, const char * fn, int option, int flags)
{
	ALLEGRO_FILE * fp;
	T3F_FONT * font = NULL;
	const char * extension = t3f_get_path_extension(fn);
	bool is_ttf = false;

	if(!strcasecmp(extension, ".ttf"))
	{
		is_ttf = true;
	}

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		goto fail;
	}
	font = t3f_load_font_with_engine_f(engine, fn, fp, option, flags);
	if(!is_ttf)
	{
		al_fclose(fp);
	}

	return font;

	fail:
	{
		if(fp)
		{
			if(!is_ttf)
			{
				al_fclose(fp);
			}
		}
		t3f_destroy_font(font);
	}
	return NULL;
}

T3F_FONT * t3f_load_font_f(const char * fn, ALLEGRO_FILE * fp, int type, int option, int flags)
{
	if(type == T3F_FONT_TYPE_AUTO)
	{
		type = detect_font_type(fn);
	}
	return t3f_load_font_with_engine_f(&font_engine[type], fn, fp, option, flags);
}

T3F_FONT * t3f_load_font(const char * fn, int type, int option, int flags)
{
	ALLEGRO_FILE * fp;
	T3F_FONT * font = NULL;
	const char * extension = t3f_get_path_extension(fn);
	bool is_ttf = false;

	if(!strcasecmp(extension, ".ttf"))
	{
		is_ttf = true;
	}

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		goto fail;
	}
	font = t3f_load_font_f(fn, fp, type, option, flags);
	if(!is_ttf)
	{
		al_fclose(fp);
	}

	return font;

	fail:
	{
		if(fp)
		{
			al_fclose(fp);
		}
		if(font)
		{
			t3f_destroy_font(font);
		}
	}
	return NULL;
}

void t3f_destroy_font(T3F_FONT * fp)
{
	if(fp)
	{
		if(fp->font)
		{
			fp->engine->destroy(fp->font);
		}
		free(fp);
	}
}
