#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "instance.h"
#include "constants.h"
#include "init.h"

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{
			if(app->ui->tags_popup_dialog && event->display.source == app->ui->tags_popup_dialog->display)
			{
				omo_close_tags_dialog(app->ui, data);
			}
			if(app->ui->split_track_popup_dialog && event->display.source == app->ui->split_track_popup_dialog->display)
			{
				omo_close_split_track_dialog(app->ui, data);
			}
			else
			{
				omo_cancel_library_setup(app);
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
