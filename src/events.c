#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "instance.h"
#include "constants.h"

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{
			if(event->display.source == app->ui->tags_display)
			{
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					al_set_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i], app->ui->tags_text[i]);
				}
				t3gui_close_dialog(app->ui->tags_dialog);
				t3gui_destroy_dialog(app->ui->tags_dialog);
				t3gui_destroy_theme(app->ui->tags_box_theme);
				al_destroy_display(app->ui->tags_display);
				app->ui->tags_display = NULL;
			}
			t3f_event_handler(event);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			omo_resize_ui(app->ui, al_get_display_width(t3f_display), al_get_display_height(t3f_display));
			break;
		}
		default:
		{
			t3f_event_handler(event);
		}
	}
}
