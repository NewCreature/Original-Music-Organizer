#include "t3f.h"
#include "font.h"
#include "draw.h"
#include "file_utils.h"

static const char * _get_config_value_fallback(ALLEGRO_CONFIG * cp, const char * section, const char * fallback_section, const char * key)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(!val)
	{
		val = al_get_config_value(cp, fallback_section, key);
	}
	return val;
}

/* include font engines */
#include "font_allegro.inc"
#include "font_t3f.inc"
#include "font_none.inc"

static T3F_FONT_ENGINE font_engine[] =
{
	{
		font_engine_load_font_f_allegro,
		NULL,
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
		_t3f_font_engine_t3f_update,
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
		font_engine_load_font_f_none,
		NULL,
		font_engine_destroy_font_none,

		font_engine_get_text_width_none,
		font_engine_get_font_height_none,
		font_engine_draw_glyph_none,
		font_engine_get_glyph_width_none,
		font_engine_get_glyph_dimensions_none,
		font_engine_get_glyph_advance_none,
		font_engine_draw_text_none,
		font_engine_draw_textf_none
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

bool t3f_init_text_lines(T3F_TEXT_LINES * text_lines)
{
	memset(text_lines, 0, sizeof(T3F_TEXT_LINES));

	return true;
}

static bool _add_text_line(T3F_TEXT_LINES * text_lines, const char * text, bool last)
{
	T3F_TEXT_LINE * text_line;

	if(!text_lines->line)
	{
		text_lines->line = malloc(sizeof(T3F_TEXT_LINE));
		if(!text_lines->line)
		{
			goto fail;
		}
		memset(text_lines->line, 0, sizeof(T3F_TEXT_LINE));
	}
	text_line = text_lines->line;
	while(text_line->next_line)
	{
		text_line = text_line->next_line;
	}

	text_line->text = strdup(text);
	if(!text_line->text)
	{
		goto fail;
	}
	if(!last)
	{
		text_line->next_line = malloc(sizeof(T3F_TEXT_LINE));
		if(!text_line->next_line)
		{
			goto fail;
		}
		memset(text_line->next_line, 0, sizeof(T3F_TEXT_LINE));
	}
	return true;

	fail:
	{
		return false;
	}
}

/* need to make this not rely on spaces, sometimes there might be long stretches with no space which need to be broken up 'mid-word' */
bool t3f_create_text_lines(T3F_TEXT_LINES * text_lines, T3F_FONT * fp, float w, float tab, const char * text)
{
	char * current_text = NULL;
	int current_text_pos = 0;
	int last_space = 0;
	int line_spaces = 0;
	int i;
	float wi = w;

	i = strlen(text) + 1;
	if(i <= 1)
	{
		goto fail;
	}
	current_text = malloc(i);
	if(!current_text)
	{
		goto fail;
	}
	memset(current_text, 0, i);

	/* divide text into lines */
	for(i = 0; i < (int)strlen(text); i++)
	{
		if(text[i] != '\r' && text[i] != '\n')
		{
			current_text[current_text_pos] = text[i];
			current_text[current_text_pos + 1] = '\0';
			if(text[i] == ' ')
			{
				last_space = current_text_pos;
				line_spaces++;
			}
			current_text_pos++;
		}

		/* copy line since we encountered a manual new line */
		if(text[i] == '\n')
		{
			current_text[current_text_pos] = '\0';
			_add_text_line(text_lines, current_text, false);
			line_spaces = 0;
			current_text_pos = 0;
			current_text[current_text_pos] = '\0';
			wi = w - tab;
		}

		/* copy this line to our list of lines because it is long enough */
		else if(t3f_get_text_width(fp, current_text) > wi && line_spaces > 0)
		{
			current_text[last_space] = '\0';
			_add_text_line(text_lines, current_text, false);
			line_spaces = 0;
			while(text[i] != ' ' && i >= 0)
			{
				i--;
			}
			current_text_pos = 0;
			current_text[current_text_pos] = '\0';
			wi = w - tab;
		}
	}
	_add_text_line(text_lines, current_text, true);

	return true;

	fail:
	{
		t3f_free_text_lines(text_lines);
		return false;
	}
}

void t3f_free_text_lines(T3F_TEXT_LINES * text_lines)
{
	T3F_TEXT_LINE * current_line = text_lines->line;
	T3F_TEXT_LINE * next_line;

	while(current_line)
	{
		next_line = current_line->next_line;
		if(current_line->text)
		{
			free(current_line->text);
		}
		free(current_line);
		current_line = next_line;
	}
	t3f_init_text_lines(text_lines);
}

void t3f_draw_text_lines(T3F_TEXT_LINES * text_lines, T3F_FONT * font, ALLEGRO_COLOR color, float x, float y, float z, float tab)
{
	T3F_TEXT_LINE * current_line = text_lines->line;
	float px = x;
	float py = y;

	while(current_line)
	{
		if(current_line->text)
		{
			t3f_draw_text(font, color, px, py, z, 0, current_line->text);
			px = x + tab;
			py += t3f_get_font_line_height(font);
		}
		current_line = current_line->next_line;
	}
}

void t3f_draw_multiline_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float w, float tab, int flags, const char * text)
{
	t3f_draw_scaled_multiline_text(fp, color, x, y, z, 1.0, w, tab, flags, text);
}

void t3f_draw_scaled_multiline_text(T3F_FONT * fp, ALLEGRO_COLOR color, float x, float y, float z, float scale, float w, float tab, int flags, const char * text)
{
	T3F_TEXT_LINES text_lines;
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
	if(w > 0.0)
	{
		t3f_init_text_lines(&text_lines);
		t3f_create_text_lines(&text_lines, fp, w, tab, text);
		t3f_draw_text_lines(&text_lines, fp, color, x, y, z, tab);
		t3f_free_text_lines(&text_lines);
		if(!held)
		{
			al_hold_bitmap_drawing(false);
		}
	}
	else
	{
		fp->engine->draw_text(fp->font, color, x, y, z, scale, flags, text);
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

void t3f_draw_scaled_text(T3F_FONT * vf, ALLEGRO_COLOR color, float x, float y, float z, float scale, int flags, const char * text)
{
	t3f_draw_scaled_multiline_text(vf, color, x, y, z, scale, 0, 0, flags, text);
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
	if(strstr(fn, ".ini"))
	{
		return T3F_FONT_TYPE_T3F;
	}
	return T3F_FONT_TYPE_ALLEGRO;
}

void * t3f_load_font_data_with_engine_f(T3F_FONT_ENGINE * engine, const char * fn, ALLEGRO_FILE * fp, int option, int flags)
{
	void * font_data;

	font_data = engine->load(fn, fp, option, flags);
	if(!font_data)
	{
		goto fail;
	}

	return font_data;

	fail:
	{
		engine->destroy(font_data);
	}
	return NULL;
}

void * t3f_load_font_data_with_engine(T3F_FONT_ENGINE * engine, const char * fn, int option, int flags)
{
	ALLEGRO_FILE * fp;
	void * font_data = NULL;
	const char * extension = t3f_get_path_extension(fn);
	bool is_ttf = false;

	if(!strcasecmp(extension, ".ttf") && engine != &(font_engine[T3F_FONT_TYPE_NONE]))
	{
		is_ttf = true;
	}
	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		goto fail;
	}
	font_data = t3f_load_font_data_with_engine_f(engine, fn, fp, option, flags);
	if(!is_ttf)
	{
		al_fclose(fp);
	}

	return font_data;

	fail:
	{
		if(fp)
		{
			if(!is_ttf)
			{
				al_fclose(fp);
			}
		}
		engine->destroy(font_data);
	}
	return NULL;
}

void * t3f_load_font_data_f(const char * fn, ALLEGRO_FILE * fp, int type, int option, int flags)
{
	return NULL;
}

void * t3f_load_font_data(const char * fn, int type, int option, int flags)
{
	if(type == T3F_FONT_TYPE_AUTO)
	{
		type = detect_font_type(fn);
	}
	return t3f_load_font_data_with_engine_f(&font_engine[type], fn, NULL, option, flags);
}

typedef struct
{

	char * fn;
	int type;
	int size;
	void ** font_data;

} T3F_FONT_LOADER_DATA;

static void _t3f_destroy_font_loader_data(T3F_FONT_LOADER_DATA * dp)
{
	if(dp)
	{
		if(dp->fn)
		{
			free(dp->fn);
		}
		free(dp);
	}
}

static T3F_FONT_LOADER_DATA * _t3f_get_font_loader_data(const char * fn, int type, int size, void ** font_data)
{
	T3F_FONT_LOADER_DATA * dp;

	dp = malloc(sizeof(T3F_FONT_LOADER_DATA));
	if(!dp)
	{
		goto fail;
	}
	memset(dp, 0, sizeof(T3F_FONT_LOADER_DATA));
	dp->fn = strdup(fn);
	dp->type = type;
	dp->size = size;
	dp->font_data = font_data;

	return dp;

	fail:
	{
		_t3f_destroy_font_loader_data(dp);
		return NULL;
	}
}

static void * _t3f_really_load_font(void * arg)
{
	T3F_FONT_LOADER_DATA * font_loader = (T3F_FONT_LOADER_DATA *)arg;
	void * font_data = NULL;

	if(arg)
	{
		switch(font_loader->type)
		{
			case T3F_FONT_TYPE_ALLEGRO:
			{
				font_data = t3f_load_resource(font_loader->font_data, t3f_font_allegro_resource_handler_proc, font_loader->fn, font_loader->size, 0, 0);
				break;
			}
			case T3F_FONT_TYPE_T3F:
			{
				font_data = t3f_load_resource(font_loader->font_data, t3f_font_t3f_resource_handler_proc, font_loader->fn, font_loader->size, 0, 0);
				break;
			}
		}
		_t3f_destroy_font_loader_data(font_loader);
	}
	else
	{
		font_data = t3f_load_font_data(NULL, T3F_FONT_TYPE_NONE, 0, 0);
	}

	return font_data;
}

T3F_FONT * t3f_load_font(const char * fn, int type, int option, int flags, bool threaded)
{
	T3F_FONT * font = NULL;
	T3F_FONT_LOADER_DATA * font_loader = NULL;

	font = malloc(sizeof(T3F_FONT));
	if(!font)
	{
		goto fail;
	}
	memset(font, 0, sizeof(T3F_FONT));
	font->type = detect_font_type(fn); // need to select engine after loading
	font->engine = &font_engine[T3F_FONT_TYPE_NONE];

	font_loader = _t3f_get_font_loader_data(fn, font->type, option, &font->font);
	if(!font_loader)
	{
		goto fail;
	}

	font->object_loader = t3f_create_object_loader();
	if(!font->object_loader)
	{
		goto fail;
	}

	font->font = t3f_load_object(font->object_loader, _t3f_really_load_font, NULL, font_loader, threaded);
	if(font->font)
	{
		font->engine = &font_engine[font->type];
	}

	return font;

	fail:
	{
		t3f_destroy_font(font);
		if(font_loader)
		{
			_t3f_destroy_font_loader_data(font_loader);
		}
	}
	return NULL;
}

bool t3f_update_font(T3F_FONT * fp)
{
	if(t3f_object_ready(fp->object_loader))
	{
		fp->font = t3f_fetch_object(fp->object_loader);
		if(fp->font)
		{
			if(font_engine[fp->type].update)
			{
				font_engine[fp->type].update(fp);
			}
			fp->engine = &font_engine[fp->type];
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void t3f_destroy_font_data(void * data, int type)
{
	if(data)
	{
		font_engine[type].destroy(data);
	}
}

void t3f_destroy_font(T3F_FONT * fp)
{
	if(fp)
	{
		if(fp->object_loader)
		{
			t3f_destroy_object_loader(fp->object_loader);
		}
		if(fp->font)
		{
			if(!t3f_destroy_resource(fp->font))
			{
				fp->engine->destroy(fp->font);
			}
		}
		free(fp);
	}
}
