#include "t3f/t3f.h"
#include "t3gui/theme.h"
#include "theme.h"

OMO_THEME * omo_load_theme(const char * fn, int mode, int font_size)
{
	OMO_THEME * tp;
	const char * val;
	char buf[32];
	ALLEGRO_PATH * path = NULL;
	int i;

	tp = malloc(sizeof(OMO_THEME));
	if(tp)
	{
		memset(tp, 0, sizeof(OMO_THEME));
		path = al_create_path(fn);
		if(!path)
		{
			goto fail;
		}
		if(strcmp(al_get_path_filename(path), "omo_theme.ini"))
		{
			goto fail;
		}
		memset(tp, 0, sizeof(OMO_THEME));
		tp->config = al_load_config_file(fn);
		if(tp->config)
		{
			if(mode == 0)
			{
				for(i = 0; i < OMO_THEME_MAX_BITMAPS; i++)
				{
					sprintf(buf, "bitmap_%d", i);
					val = al_get_config_value(tp->config, "Settings", buf);
					if(val)
					{
						al_set_path_filename(path, val);
						if(!t3f_load_resource((void **)(&tp->bitmap[i]), T3F_RESOURCE_TYPE_BITMAP, al_path_cstr(path, '/'), 0, 0, 0))
						{
							if(!t3f_load_resource((void **)(&tp->bitmap[i]), T3F_RESOURCE_TYPE_BITMAP, val, 0, 0, 0))
							{
								goto fail;
							}
						}
					}
				}
			}
			for(i = 0; i < OMO_THEME_MAX_GUI_THEMES; i++)
			{
				sprintf(buf, "theme_%d", i);
				val = al_get_config_value(tp->config, "Settings", buf);
				if(val)
				{
					al_set_path_filename(path, val);
					tp->gui_theme[i] = t3gui_load_theme(al_path_cstr(path, '/'), font_size);
				}
			}
			for(i = 0; i < OMO_THEME_MAX_TEXTS; i++)
			{
				strcpy(tp->text[i], "");
				sprintf(buf, "text_%d", i);
				val = al_get_config_value(tp->config, "Settings", buf);
				if(val)
				{
					strcpy(tp->text[i], val);
				}
			}
		}
		else
		{
			goto fail;
		}
		al_destroy_path(path);
	}
	return tp;

	fail:
	{
		if(path)
		{
			al_destroy_path(path);
		}
		if(tp->config)
		{
			al_destroy_config(tp->config);
		}
		if(tp)
		{
			free(tp);
		}
	}
	return NULL;
}

void omo_destroy_theme(OMO_THEME * tp)
{
	int i;

	for(i = 0; i < OMO_THEME_MAX_GUI_THEMES; i++)
	{
		if(tp->gui_theme[i])
		{
			t3gui_destroy_theme(tp->gui_theme[i]);
		}
	}
	for(i = 0; i < OMO_THEME_MAX_BITMAPS; i++)
	{
		if(tp->bitmap[i])
		{
			t3f_destroy_resource(tp->bitmap[i]);
		}
	}
	al_destroy_config(tp->config);
}
