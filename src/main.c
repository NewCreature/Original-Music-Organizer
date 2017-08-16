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
#include "ui/ui.h"
#include "init.h"

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	const char * val2;
	int i, j;

	switch(app->state)
	{
		default:
		{
			omo_file_chooser_logic(data);
			t3gui_logic();
			if(!app->ui->tags_display)
			{
				if(app->player->queue && app->ui->ui_queue_list_element->id1 >= 0)
				{
					if(app->player->state == OMO_PLAYER_STATE_PLAYING)
					{
						app->player->queue_pos = app->ui->ui_queue_list_element->id1 - 1;
						omo_play_next_song(app->player);
					}
					else
					{
						app->player->queue_pos = app->ui->ui_queue_list_element->id1;
						omo_start_player(app->player);
					}
					app->ui->ui_queue_list_element->id1 = -1;
				}
				sprintf(app->ui->ui_button_text[0], "|<");
				if(app->player->state == OMO_PLAYER_STATE_PLAYING)
				{
					sprintf(app->ui->ui_button_text[1], "||");
				}
				else
				{
					sprintf(app->ui->ui_button_text[1], ">");
				}
				sprintf(app->ui->ui_button_text[2], "[]");
				sprintf(app->ui->ui_button_text[3], ">|");
				sprintf(app->ui->ui_button_text[4], "^");
				sprintf(app->ui->ui_button_text[5], "+");
				if(t3f_key[ALLEGRO_KEY_L])
				{
					t3gui_close_dialog(app->ui->ui_dialog);
					app->library_view = !app->library_view;
					omo_create_main_dialog(app->ui, app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), app);
					t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
					t3f_key[ALLEGRO_KEY_L] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_T] && app->library && app->player->queue)
				{
					j = app->ui->ui_queue_list_element->d1;
					strcpy(fullfn, app->player->queue->entry[j]->file);
					if(app->player->queue->entry[j]->sub_file)
					{
						strcat(fullfn, "/");
						strcat(fullfn, app->player->queue->entry[j]->sub_file);
					}
					app->ui->tags_entry = al_get_config_value(app->library->file_database, fullfn, "id");
					if(app->ui->tags_entry)
					{
						for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
						{
							val2 = al_get_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
							if(val2)
							{
								strcpy(app->ui->tags_text[i], val2);
							}
							else
							{
								strcpy(app->ui->tags_text[i], "");
							}
						}
						omo_open_tags_dialog(app->ui, app);
					}
					t3f_key[ALLEGRO_KEY_T] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_LEFT])
				{
					app->button_pressed = 0;
					t3f_key[ALLEGRO_KEY_LEFT] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_ENTER])
				{
					if(app->player->queue_pos != app->ui->ui_queue_list_element->d1)
					{
						omo_stop_player(app->player);
						app->player->queue_pos = app->ui->ui_queue_list_element->d1;
						omo_start_player(app->player);
					}
				}
				if(t3f_key[ALLEGRO_KEY_RIGHT])
				{
					app->button_pressed = 3;
					t3f_key[ALLEGRO_KEY_RIGHT] = 0;
				}
				switch(app->button_pressed)
				{
					case 0:
					{
						omo_play_previous_song(app->player);
						break;
					}
					case 1:
					{
						switch(app->player->state)
						{
							case OMO_PLAYER_STATE_STOPPED:
							{
								app->player->queue_pos = app->ui->ui_queue_list_element->d1;
								omo_start_player(app->player);
								break;
							}
							case OMO_PLAYER_STATE_PLAYING:
							{
								omo_pause_player(app->player);
								break;
							}
							case OMO_PLAYER_STATE_PAUSED:
							{
								omo_resume_player(app->player);
								break;
							}
						}
						break;
					}
					case 2:
					{
						omo_stop_player(app->player);
						break;
					}
					case 3:
					{
						omo_play_next_song(app->player);
						break;
					}
					case 4:
					{
						if(OMO_KEY_CTRL)
						{
							omo_menu_file_play_folder(data);
							t3f_key[ALLEGRO_KEY_COMMAND] = 0;
							t3f_key[ALLEGRO_KEY_LCTRL] = 0;
							t3f_key[ALLEGRO_KEY_RCTRL] = 0;
						}
						else
						{
							omo_menu_file_play_files(data);
						}
						break;
					}
					case 5:
					{
						if(OMO_KEY_CTRL)
						{
							omo_menu_file_queue_folder(data);
							t3f_key[ALLEGRO_KEY_COMMAND] = 0;
							t3f_key[ALLEGRO_KEY_LCTRL] = 0;
							t3f_key[ALLEGRO_KEY_RCTRL] = 0;
						}
						else
						{
							omo_menu_file_queue_files(data);
						}
						break;
					}
				}
				app->button_pressed = -1;
				omo_player_logic(app->player, app->archive_handler_registry, app->codec_handler_registry);
				break;
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
		t3gui_exit();
	}
	else
	{
		printf("Error: could not initialize T3F!\n");
	}
	return 0;
}
