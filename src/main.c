#include "t3f/t3f.h"
#include <DUMBA5/dumba5.h>

#include "instance.h"
#include "ui_init.h"

#include "codecs/dumba5.h"
#include "codecs/allegro_acodec.h"

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codecs/avmidiplayer.h"
	#include "codecs/avplayer.h"
#endif

/* main logic routine */
void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
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

	switch(app->state)
	{
		default:
		{
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
	app->player_registry.players = 0;
	omo_register_player(&app->player_registry, omo_codec_dumba5_get_player());
	omo_register_player(&app->player_registry, omo_codec_allegro_acodec_get_player());
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
