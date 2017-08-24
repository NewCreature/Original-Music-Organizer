#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "instance.h"
#include "constants.h"

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{
			if(event->display.source == app->ui->tags_display)
			{
				omo_close_tags_dialog(app->ui, data);
			}
			t3f_event_handler(event);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			omo_resize_ui(app->ui, app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display));
			break;
		}
		default:
		{
			t3f_event_handler(event);
		}
	}
}
