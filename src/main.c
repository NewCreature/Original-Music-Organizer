#include "t3f/t3f.h"
#include "t3f/file_utils.h"

#include "instance.h"
#include "defines.h"
#include "constants.h"
#include "library.h"
#include "player.h"
#include "file_chooser.h"
#include "ui/menu_init.h"
#include "ui/menu_proc.h"
#include "ui/dialog_proc.h"
#include "ui/ui.h"
#include "init.h"
#include "file_helpers.h"
#include "ui/queue_list.h"
#include "ui/library.h"
#include "ui/tags_dialog.h"
#include "ui/multi_tags_dialog.h"
#include "ui/album_tags_dialog.h"
#include "ui/split_track_dialog.h"
#include "ui/tagger_key_dialog.h"
#include "ui/new_profile_dialog.h"
#include "ui/filter_dialog.h"
#include "ui/shortcut.h"
#include "ui/player.h"
#include "test.h"
#include "queue_helpers.h"
#include "library_helpers.h"
#include "threads.h"
#include "ui/about_dialog.h"
#include "ui/rebase_song_folder_dialog.h"

static int queue_list_visible_elements(T3GUI_ELEMENT * element)
{
	return element->h / element->ed2;
}

static void update_seek_pos(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	double pos, length;

	if(app->ui->ui_seeked)
	{
		if(app->player->track->codec_handler->seek)
		{
			length = app->player->track->codec_handler->get_length(app->player->track->codec_data);
			pos = ((double)app->ui->ui_seek_control_element->d2 / (double)OMO_UI_SEEK_RESOLUTION) * length;
			app->player->track->codec_handler->seek(app->player->track->codec_data, pos);
		}
		app->ui->ui_seeked = false;
	}
	else
	{
		if(app->player->track)
		{
			if(app->player->track->codec_handler->get_length && app->player->track->codec_handler->get_position)
			{
				app->ui->ui_seek_control_element->flags &= ~D_DISABLED;
				if(!(app->ui->ui_seek_control_element->flags & D_TRACKMOUSE))
				{
					length = app->player->track->codec_handler->get_length(app->player->track->codec_data);
					pos = app->player->track->codec_handler->get_position(app->player->track->codec_data);
					app->ui->ui_seek_control_element->d2 = (pos / length) * (double)OMO_UI_SEEK_RESOLUTION;
				}
			}
			else
			{
				app->ui->ui_seek_control_element->flags |= D_DISABLED;
			}
		}
		else
		{
			app->ui->ui_seek_control_element->flags |= D_DISABLED;
		}
	}
}

static void update_volume_pos(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	double volume;
	char buf[128];
	const char * val;

	if(app->ui->ui_volume_changed)
	{
		volume = 1.0 - ((double)app->ui->ui_volume_control_element->d2 / (double)OMO_UI_VOLUME_RESOLUTION);
		if(app->player->track && app->player->track->codec_handler->set_volume)
		{
			app->player->track->codec_handler->set_volume(app->player->track->codec_data, volume);
		}
		sprintf(buf, "%f", volume);
		al_set_config_value(t3f_config, "Settings", "volume", buf);
		app->ui->ui_volume_changed = false;
	}
	else
	{
		val = al_get_config_value(t3f_config, "Settings", "volume");
		if(val)
		{
			app->ui->ui_volume_control_element->d2 = (1.0 - atof(val)) * OMO_UI_VOLUME_RESOLUTION;
		}
	}
	if(app->player->track && !app->player->track->codec_handler->set_volume)
	{
		app->ui->ui_volume_control_element->flags |= D_DISABLED;
	}
	else
	{
		app->ui->ui_volume_control_element->flags &= ~D_DISABLED;
	}
}

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int old_queue_list_pos = -1;
	int visible = 0;
	int seek_flags;
	int volume_pos;
	bool disable_shortcuts = false;

	t3f_refresh_menus();
	if(app->test_mode >= 0)
	{
		if(!omo_test_logic(app))
		{
			printf("Testing ended prematurely!\n");
			t3f_exit();
		}
	}
	switch(app->state)
	{
		default:
		{
			if(app->player->queue)
			{
				old_queue_list_pos = app->player->queue_pos;
				visible = queue_list_visible_elements(app->ui->ui_queue_list_element);
				if(visible > app->player->queue->entry_count)
				{
					visible = app->player->queue->entry_count;
				}
			}
			omo_file_chooser_logic(data);
			omo_library_pre_gui_logic(data);
			seek_flags = app->ui->ui_seek_control_element->flags;
			volume_pos = app->ui->ui_volume_control_element->d2;
			t3gui_logic();
			if(seek_flags & D_TRACKMOUSE && !(app->ui->ui_seek_control_element->flags & D_TRACKMOUSE))
			{
				app->ui->ui_seeked = true;
			}
			if(volume_pos != app->ui->ui_volume_control_element->d2)
			{
				app->ui->ui_volume_changed = true;
			}
			if(app->ui->tags_popup_dialog)
			{
				omo_tags_dialog_logic(data);
			}
			else if(app->ui->multi_tags_popup_dialog)
			{
				omo_multi_tags_dialog_logic(data);
			}
			else if(app->ui->album_tags_popup_dialog)
			{
				omo_album_tags_dialog_logic(data);
			}
			else if(app->ui->split_track_popup_dialog)
			{
				omo_split_track_dialog_logic(data);
			}
			else if(app->ui->tagger_key_popup_dialog)
			{
				omo_tagger_key_dialog_logic(data);
			}
			else if(app->ui->new_profile_popup_dialog)
			{
				omo_new_profile_dialog_logic(data);
			}
			else if(app->ui->rebase_song_folder_popup_dialog)
			{
				omo_rebase_song_folder_dialog_logic(data);
			}
			else if(app->ui->filter_popup_dialog)
			{
				omo_filter_dialog_logic(data);
			}
			else if(app->ui->about_popup_dialog)
			{
				omo_about_dialog_logic(data);
			}
			else
			{
				if(app->library_view)
				{
					if(app->library && app->library->loaded)
					{
						omo_library_logic(data);
					}
					if(app->ui->ui_artist_search_element->flags & D_GOTFOCUS)
					{
						disable_shortcuts = true;
					}
					else if(app->ui->ui_album_search_element->flags & D_GOTFOCUS)
					{
						disable_shortcuts = true;
					}
					else if(app->ui->ui_song_search_element->flags & D_GOTFOCUS)
					{
						disable_shortcuts = true;
					}
				}
				omo_queue_list_logic(app);
				if(!disable_shortcuts)
				{
					omo_shortcut_logic(app);
				}
				omo_player_ui_logic(app);
				if(app->library_view)
				{
					app->ui->ui_artist_list_element->id2 = app->ui->ui_artist_list_element->d1;
					app->ui->ui_album_list_element->id2 = app->ui->ui_album_list_element->d1;
					app->ui->ui_song_list_element->id2 = app->ui->ui_song_list_element->d1;
				}
			}
			omo_player_logic(app->player, app->library, app->archive_handler_registry, app->codec_handler_registry, app->player_temp_path);
			if(app->player->new_tags)
			{
				omo_spawn_cloud_thread(app);
				app->player->new_tags = false;
			}
			update_seek_pos(app);
			update_volume_pos(app);
			app->ui->ui_queue_list_element->id2 = app->player->queue_pos;

			/* see if we should scroll the queue list */
			if(app->player->queue && app->player->queue_pos != old_queue_list_pos)
			{
				if(old_queue_list_pos >= app->ui->ui_queue_list_element->d2 && old_queue_list_pos < app->ui->ui_queue_list_element->d2 + visible)
				{
					/* go to previous page */
					if(app->player->queue_pos < app->ui->ui_queue_list_element->d2)
					{
						app->ui->ui_queue_list_element->d2 -= visible;
						if(app->ui->ui_queue_list_element->d2 < 0)
						{
							app->ui->ui_queue_list_element->d2 = 0;
						}
					}

					/* go to next page */
					if(app->player->queue_pos > app->ui->ui_queue_list_element->d2 + visible - 1)
					{
						app->ui->ui_queue_list_element->d2 += visible;
						if(app->ui->ui_queue_list_element->d2 + visible > app->player->queue->entry_count)
						{
							app->ui->ui_queue_list_element->d2 = app->player->queue->entry_count - visible;
						}
					}
				}
			}
			if(app->player->queue)
			{
				if(app->ui->ui_queue_list_element->d2 + visible > app->player->queue->entry_count)
				{
					app->ui->ui_queue_list_element->d2 = app->player->queue->entry_count - visible - 1;
					if(app->ui->ui_queue_list_element->d2 < 0)
					{
						app->ui->ui_queue_list_element->d2 = 0;
					}
				}
			}
			omo_threads_logic(app);
			if(app->library && !app->library_thread)
			{
				strcpy(app->status_bar_text, "Library ready.");
			}
			if(app->library && app->library_thread && !app->cloud_thread)
			{
				strcpy(app->status_bar_text, "Generating library lists...");
			}
		}
	}
}

/* main rendering routine */
void omo_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
			t3gui_render(NULL);
			if(app->ui->tags_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->tags_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->multi_tags_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->multi_tags_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->album_tags_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->album_tags_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->split_track_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->split_track_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->tagger_key_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->tagger_key_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->new_profile_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->new_profile_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->rebase_song_folder_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->rebase_song_folder_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->filter_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->filter_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			if(app->ui->about_popup_dialog)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->about_popup_dialog->display));
				al_flip_display();
				al_set_target_bitmap(al_get_backbuffer(t3f_display));
			}
			break;
		}
	}
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;
	char buf[256];

	if(omo_initialize(&app, argc, argv))
	{
		t3f_run();
		al_remove_config_key(t3f_config, "Settings", "queue_track_position");
		al_remove_config_key(t3f_config, "Settings", "player_state");
		if(app.player)
		{
			if(app.player->state == OMO_PLAYER_STATE_PLAYING || app.player->state == OMO_PLAYER_STATE_PAUSED)
			{
				if(app.player->track && app.player->track->codec_handler->get_position)
				{
					sprintf(buf, "%f", app.player->track->codec_handler->get_position(app.player->track->codec_data));
					al_set_config_value(t3f_config, "Settings", "queue_track_position", buf);
					sprintf(buf, "%d", app.player->state);
					al_set_config_value(t3f_config, "Settings", "player_state", buf);
				}
			}
			omo_stop_player(app.player);
		}
		if(app.library)
		{
			omo_save_library(app.library);
		}
		omo_exit(&app);
		t3gui_exit();
		t3f_finish();
	}
	return 0;
}
