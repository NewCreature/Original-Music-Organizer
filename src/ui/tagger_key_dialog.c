#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3net/t3net.h"
#include "../instance.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../library_helpers.h"
#include "../cloud.h"

void omo_tagger_key_dialog_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(t3f_key[ALLEGRO_KEY_ESCAPE])
	{
		omo_close_tagger_key_dialog(app->ui, app);
		t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
	}
	if(app->button_pressed == 0)
	{
		if(strlen(app->ui->tagger_key_text))
		{
			if(strcmp(app->ui->original_tagger_key_text, app->ui->tagger_key_text))
			{
				omo_get_tagger_key(app->ui->tagger_key_text);
			}
		}
		omo_close_tagger_key_dialog(app->ui, app);
		app->button_pressed = -1;
		t3f_key[ALLEGRO_KEY_ENTER] = 0;
	}
	else if(app->button_pressed == 1)
	{
		omo_close_tagger_key_dialog(app->ui, app);
		app->button_pressed = -1;
	}
}
