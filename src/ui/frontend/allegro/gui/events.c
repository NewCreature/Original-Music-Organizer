#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "instance.h"
#include "constants.h"
#include "library_helpers.h"
#include "tags_dialog.h"
#include "multi_tags_dialog.h"
#include "album_tags_dialog.h"
#include "tagger_key_dialog.h"
#include "filter_dialog.h"
#include "new_profile_dialog.h"
#include "rebase_song_folder_dialog.h"
#include "split_track_dialog.h"
#include "about_dialog.h"

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{
			if(uip->tags_popup_dialog && event->display.source == uip->tags_popup_dialog->display)
			{
				omo_close_tags_dialog(uip, data);
			}
			else if(uip->multi_tags_popup_dialog && event->display.source == uip->multi_tags_popup_dialog->display)
			{
				omo_close_multi_tags_dialog(uip, data);
			}
			else if(uip->album_tags_popup_dialog && event->display.source == uip->album_tags_popup_dialog->display)
			{
				omo_close_album_tags_dialog(uip, data);
			}
			else if(uip->split_track_popup_dialog && event->display.source == uip->split_track_popup_dialog->display)
			{
				omo_close_split_track_dialog(uip, data);
			}
			else if(uip->tagger_key_popup_dialog && event->display.source == uip->tagger_key_popup_dialog->display)
			{
				omo_close_tagger_key_dialog(uip, data);
			}
			else if(uip->new_profile_popup_dialog && event->display.source == uip->new_profile_popup_dialog->display)
			{
				omo_close_new_profile_dialog(uip, data);
			}
			else if(uip->rebase_song_folder_popup_dialog && event->display.source == uip->rebase_song_folder_popup_dialog->display)
			{
				omo_close_rebase_song_folder_dialog(uip, data);
			}
			else if(uip->filter_popup_dialog && event->display.source == uip->filter_popup_dialog->display)
			{
				omo_close_filter_dialog(uip, data);
			}
			else if(uip->about_popup_dialog && event->display.source == uip->about_popup_dialog->display)
			{
				omo_close_about_dialog(uip, data);
			}
			else
			{
				omo_cancel_library_setup(uip->app);
			}
			t3f_event_handler(event);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			omo_resize_ui(uip, uip->app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display));
			break;
		}
		default:
		{
			t3f_event_handler(event);
		}
	}
}
