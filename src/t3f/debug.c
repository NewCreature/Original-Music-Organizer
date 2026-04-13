#include "t3f.h"

static FILE * t3f_debug_file = NULL;
static ALLEGRO_FONT * _t3f_debug_font = NULL;

bool t3f_open_debug_log(const char * fn)
{
	t3f_debug_file = fopen(fn, "w");
	if(t3f_debug_file)
	{
		return true;
	}
	return false;
}

void t3f_close_debug_log(void)
{
	if(t3f_debug_file)
	{
		fclose(t3f_debug_file);
		t3f_debug_file = NULL;
	}
}

void t3f_debug_message(const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	/* write to debug log file if one is open */
	if(t3f_debug_file)
	{
		fprintf(t3f_debug_file, "%s", buf);
		fflush(t3f_debug_file);
	}

	/* otherwise output to console */
	else
	{
		#ifdef ALLEGRO_ANDROID
			ALLEGRO_DEBUG_CHANNEL("main");
			ALLEGRO_DEBUG("%s", buf);
		#else
			printf("%s", buf);
		#endif
	}
}

void t3f_display_debug_message(ALLEGRO_COLOR bg, ALLEGRO_COLOR fg, float delay, const char * format, ...)
{
	char buf[1024] = {0};
	va_list vap;

	va_start(vap, format);
	vsnprintf(buf, 1024, format, vap);
	va_end(vap);

	if(t3f_display)
	{
		if(!_t3f_debug_font)
		{
			_t3f_debug_font = al_create_builtin_font();
		}
		if(_t3f_debug_font)
		{
		  al_clear_to_color(bg);
			al_draw_text(_t3f_debug_font, fg, 0, 0, 0, buf);
  		al_flip_display();
  		al_rest(delay);
		}
	}
}
