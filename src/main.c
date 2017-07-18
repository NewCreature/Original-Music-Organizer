#include "t3f/t3f.h"
#include <DUMBA5/dumba5.h>

#include "instance.h"
#include "ui_init.h"

#include "codecs/dumba5.h"
#include "codecs/allegro_acodec.h"
#include "codecs/gme.h"

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codecs/avmidiplayer.h"
	#include "codecs/avplayer.h"
#endif

/* main logic routine */
void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	bool next_file = false;

	switch(app->state)
	{
		default:
		{
			if(app->queue)
			{
				if(app->player)
				{
					if(app->player->done_playing())
					{
						app->player->stop();
						app->player = NULL;
						next_file = true;
					}
				}
				else
				{
					next_file = true;
				}
				if(next_file)
				{
					while(1)
					{
						app->queue_pos++;
						if(app->queue_pos < app->queue->file_count)
						{
							app->player = omo_get_player(&app->player_registry, app->queue->file[app->queue_pos]);
							if(app->player)
							{
								if(app->player->load_file(app->queue->file[app->queue_pos]))
								{
									if(app->player->play())
									{
										break;
									}
								}
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			if(t3f_key[ALLEGRO_KEY_LEFT])
			{
				if(app->queue)
				{
					if(app->player)
					{
						app->player->stop();
						app->player = NULL;
					}
					if(app->queue_pos > 0)
					{
						app->queue_pos -= 2;
					}
					else
					{
						app->queue_pos = -1;
					}
				}
				t3f_key[ALLEGRO_KEY_LEFT] = 0;
			}
			if(t3f_key[ALLEGRO_KEY_RIGHT])
			{
				if(app->player)
				{
					app->player->stop();
					app->player = NULL;
				}
				t3f_key[ALLEGRO_KEY_RIGHT] = 0;
			}
			/* insert logic here, as your project grows you can add more states
			 * to deal with various parts of your app (logo, title screen, in-
			 * game, etc.) */
			break;
		}
	}
}

/* main rendering routine */
void app_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_COLOR color;
	int i;

	switch(app->state)
	{
		default:
		{
			al_clear_to_color(t3f_color_black);
			if(app->queue)
			{
				for(i = 0; i < app->queue->file_count; i++)
				{
					if(i == app->queue_pos)
					{
						color = al_map_rgba_f(1.0, 1.0, 0.0, 1.0);
					}
					else
					{
						color = t3f_color_white;
					}
					al_draw_textf(app->font, color, 0, i * al_get_font_line_height(app->font), 0, "%3d. %s", i + 1, app->queue->file[i]);
				}
			}
			/* insert rendering code here, see app_logic() for more info */
			break;
		}
	}
}

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, app_logic, app_render, T3F_DEFAULT | T3F_RESIZABLE, app))
	{
		printf("Error initializing T3F!\n");
		return false;
	}
	memset(app, 0, sizeof(APP_INSTANCE));
	app->font = al_create_builtin_font();
	if(!app->font)
	{
		printf("Error loading font!\n");
		return false;
	}
	app->player_registry.players = 0;
	omo_register_player(&app->player_registry, omo_codec_dumba5_get_player());
	omo_register_player(&app->player_registry, omo_codec_allegro_acodec_get_player());
	omo_register_player(&app->player_registry, omo_codec_gme_get_player());
	#ifdef ALLEGRO_MACOSX
		omo_register_player(&app->player_registry, omo_codec_avmidiplayer_get_player());
		omo_register_player(&app->player_registry, omo_codec_avplayer_get_player());
	#endif
	if(!omo_setup_menus(app))
	{
		printf("Error setting up menus!\n");
		return false;
	}
	t3f_attach_menu(app->menu[OMO_MENU_MAIN]);

	app->state = 0;
	return true;
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;

	if(app_initialize(&app, argc, argv))
	{
		t3f_run();
	}
	else
	{
		printf("Error: could not initialize T3F!\n");
	}
	return 0;
}
