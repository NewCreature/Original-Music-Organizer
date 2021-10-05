#include "t3f.h"

static FILE * t3f_debug_file = NULL;

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
