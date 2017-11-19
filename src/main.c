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
#include "ui/shortcut.h"
#include "test.h"
#include "queue_helpers.h"

static int queue_list_visible_elements(T3GUI_ELEMENT * element)
{
	return element->h / al_get_font_line_height(element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font) - 1;
}

static void update_seek_pos(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	double pos, length;

	if(app->ui->ui_seeked)
	{
		if(app->player->codec_handler->seek)
		{
			length = app->player->codec_handler->get_length(app->player->codec_data);
			pos = ((double)app->ui->ui_seek_control_element->d2 / (double)OMO_UI_SEEK_RESOLUTION) * length;
			app->player->codec_handler->seek(app->player->codec_data, pos);
		}
		app->ui->ui_seeked = false;
	}
	else
	{
		if(app->player->codec_handler)
		{
			if(app->player->codec_handler->get_length && app->player->codec_handler->get_position)
			{
				app->ui->ui_seek_control_element->flags &= ~D_DISABLED;
				if(!(app->ui->ui_seek_control_element->flags & D_TRACKMOUSE))
				{
					length = app->player->codec_handler->get_length(app->player->codec_data);
					pos = app->player->codec_handler->get_position(app->player->codec_data);
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

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int old_queue_list_pos = -1;
	int visible = 0;
	int seek_flags;

	t3f_refresh_menus();
	if(app->test_mode)
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
			if(app->library_thread && app->loading_library_file_helper_data.scan_done)
			{
				al_destroy_thread(app->library_thread);
				app->library_thread = NULL;
				if(app->library && app->player->queue)
				{
					omo_get_queue_tags(app->player->queue, app->library, app);
				}
			}
			omo_file_chooser_logic(data);
			omo_library_pre_gui_logic(data);
			seek_flags = app->ui->ui_seek_control_element->flags;
			t3gui_logic();
			if(seek_flags & D_TRACKMOUSE && !(app->ui->ui_seek_control_element->flags & D_TRACKMOUSE))
			{
				app->ui->ui_seeked = true;
			}
			if(!app->ui->tags_display)
			{
				if(app->library_view)
				{
					omo_library_logic(data);
				}
				omo_queue_list_logic(app);
				omo_shortcut_logic(app);
				if(app->library_view)
				{
					app->ui->ui_artist_list_element->id2 = app->ui->ui_artist_list_element->d1;
					app->ui->ui_album_list_element->id2 = app->ui->ui_album_list_element->d1;
				}
			}
			else
			{
				omo_tags_dialog_logic(data);
			}
			omo_player_logic(app->player, app->library, app->archive_handler_registry, app->codec_handler_registry, app->player_temp_path);
			update_seek_pos(app);
			app->ui->ui_queue_list_element->id2 = app->player->queue_pos;

			/* see if we should scroll the queue list */
			if(app->player->queue && app->player->queue_pos != old_queue_list_pos)
			{
				if(old_queue_list_pos >= app->ui->ui_queue_list_element->d2 && old_queue_list_pos < app->ui->ui_queue_list_element->d2 + visible + 1)
				{
					if(app->player->queue_pos < app->ui->ui_queue_list_element->d2)
					{
						app->ui->ui_queue_list_element->d2 -= visible + 1;
						if(app->ui->ui_queue_list_element->d2 < 0)
						{
							app->ui->ui_queue_list_element->d2 = 0;
						}
					}
					else if(app->player->queue_pos > app->ui->ui_queue_list_element->d2 + visible)
					{
						app->ui->ui_queue_list_element->d2 += visible + 1;
					}
				}
			}
			if(app->player->queue)
			{
				if(app->ui->ui_queue_list_element->d2 + visible > app->player->queue->entry_count)
				{
					app->ui->ui_queue_list_element->d2 = app->player->queue->entry_count - visible - 1;
				}
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
			t3gui_render();
			if(app->ui->tags_display)
			{
				al_set_target_bitmap(al_get_backbuffer(app->ui->tags_display));
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

	if(omo_initialize(&app, argc, argv))
	{
		t3f_run();
		if(app.player)
		{
			omo_stop_player(app.player);
		}
		if(app.library)
		{
			omo_save_library(app.library);
		}
		omo_exit(&app);
		t3gui_exit();
	}
	return 0;
}
