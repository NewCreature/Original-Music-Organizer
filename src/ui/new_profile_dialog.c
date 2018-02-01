#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3net/t3net.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../profile.h"
#include "menu_init.h"

void omo_new_profile_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_new_profile_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		if(strlen(app->ui->new_profile_text))
		{
			omo_clear_profile_menu(data);
			if(omo_add_profile(app->ui->new_profile_text))
			{
				if(omo_setup_profile(app->ui->new_profile_text))
				{
					omo_set_current_profile(omo_get_profile_count() - 1);
					app->spawn_library_thread = true;
				}
			}
			omo_update_profile_menu(data);
		}
		omo_close_new_profile_dialog(app->ui, app);
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_new_profile_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
