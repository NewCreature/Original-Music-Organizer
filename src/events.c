#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "instance.h"
#include "constants.h"
#include "library_helpers.h"
#include "ui/tags_dialog.h"
#include "ui/multi_tags_dialog.h"
#include "ui/album_tags_dialog.h"
#include "ui/tagger_key_dialog.h"
#include "ui/filter_dialog.h"
#include "ui/new_profile_dialog.h"
#include "ui/rebase_song_folder_dialog.h"
#include "ui/split_track_dialog.h"
#include "ui/about_dialog.h"

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
			else if(app->ui->multi_tags_popup_dialog && event->display.source == app->ui->multi_tags_popup_dialog->display)
			{
				omo_close_multi_tags_dialog(app->ui, data);
			}
			else if(app->ui->album_tags_popup_dialog && event->display.source == app->ui->album_tags_popup_dialog->display)
			{
				omo_close_album_tags_dialog(app->ui, data);
			}
			else if(app->ui->split_track_popup_dialog && event->display.source == app->ui->split_track_popup_dialog->display)
			{
				omo_close_split_track_dialog(app->ui, data);
			}
			else if(app->ui->tagger_key_popup_dialog && event->display.source == app->ui->tagger_key_popup_dialog->display)
			{
				omo_close_tagger_key_dialog(app->ui, data);
			}
			else if(app->ui->new_profile_popup_dialog && event->display.source == app->ui->new_profile_popup_dialog->display)
			{
				omo_close_new_profile_dialog(app->ui, data);
			}
			else if(app->ui->rebase_song_folder_popup_dialog && event->display.source == app->ui->rebase_song_folder_popup_dialog->display)
			{
				omo_close_rebase_song_folder_dialog(app->ui, data);
			}
			else if(app->ui->filter_popup_dialog && event->display.source == app->ui->filter_popup_dialog->display)
			{
				omo_close_filter_dialog(app->ui, data);
			}
			else if(app->ui->about_popup_dialog && event->display.source == app->ui->about_popup_dialog->display)
			{
				omo_close_about_dialog(app->ui, data);
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
