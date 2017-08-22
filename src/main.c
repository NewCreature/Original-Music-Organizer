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

static void queue_song_list(void * data, OMO_LIBRARY * lp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_QUEUE * new_queue;
	int i;

	new_queue = omo_create_queue(lp->song_entry_count);
	if(new_queue)
	{
		for(i = 0; i < lp->song_entry_count; i++)
		{
			omo_add_file_to_queue(new_queue, lp->entry[lp->song_entry[i]]->filename, lp->entry[lp->song_entry[i]]->sub_filename, lp->entry[lp->song_entry[i]]->track);
		}
		switch(app->player->state)
		{
			case OMO_PLAYER_STATE_PLAYING:
			case OMO_PLAYER_STATE_PAUSED:
			{
				omo_stop_player(app->player);
				break;
			}
		}
		if(app->player->queue)
		{
			omo_destroy_queue(app->player->queue);
		}
		app->player->queue = new_queue;
		app->player->queue_pos = 0;
		omo_start_player(app->player);
	}
}

/* main logic routine */
void omo_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	const char * val;
	const char * val2;
	int i, j;
	int old_artist_d1;
	int old_album_d1;

	switch(app->state)
	{
		default:
		{
			omo_file_chooser_logic(data);
			if(app->library_view)
			{
				old_artist_d1 = app->ui->ui_artist_list_element->d1;
				old_album_d1 = app->ui->ui_album_list_element->d1;
			}
			t3gui_logic();
			if(app->library_view)
			{
				if(app->ui->ui_artist_list_element->d1 != old_artist_d1)
				{
					app->ui->ui_album_list_element->d1 = 0;
					app->ui->ui_song_list_element->d1 = 0;
					val2 = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
					omo_get_library_song_list(app->library, val2, "All");
				}
				else if(app->ui->ui_album_list_element->d1 != old_album_d1)
				{
					app->ui->ui_song_list_element->d1 = 0;
					val = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
					val2 = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
					omo_get_library_song_list(app->library, val, val2);
				}
			}
			if(!app->ui->tags_display)
			{
				if(app->library_view)
				{
					if(app->ui->ui_artist_list_element->id1 >= 0)
					{
						queue_song_list(app, app->library);
						app->ui->ui_artist_list_element->id1 = -1;
					}
					else if(app->ui->ui_album_list_element->id1 >= 0)
					{
						queue_song_list(app, app->library);
						app->ui->ui_album_list_element->id1 = -1;
					}
					else if(app->ui->ui_song_list_element->id1 >= 0)
					{
						switch(app->player->state)
						{
							case OMO_PLAYER_STATE_PLAYING:
							case OMO_PLAYER_STATE_PAUSED:
							{
								omo_stop_player(app->player);
								break;
							}
						}
						if(app->player->queue)
						{
							omo_destroy_queue(app->player->queue);
						}
						app->player->queue = omo_create_queue(1);
						if(app->player->queue)
						{
							omo_add_file_to_queue(app->player->queue, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->filename, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->sub_filename, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->track);
							app->player->queue_pos = 0;
							omo_start_player(app->player);
						}
						app->ui->ui_song_list_element->id1 = -1;
					}
				}
				else if(app->player->queue && app->ui->ui_queue_list_element->id1 >= 0)
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
				if(t3f_key[ALLEGRO_KEY_Z])
				{
					app->button_pressed = 0;
					t3f_key[ALLEGRO_KEY_Z] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_X])
				{
					if(app->player->queue_pos != app->ui->ui_queue_list_element->d1)
					{
						omo_stop_player(app->player);
						app->player->queue_pos = app->ui->ui_queue_list_element->d1;
						omo_start_player(app->player);
					}
				}
				if(t3f_key[ALLEGRO_KEY_C])
				{
					if(app->player->state == OMO_PLAYER_STATE_PLAYING)
					{
						omo_pause_player(app->player);
					}
					else if(app->player->state == OMO_PLAYER_STATE_PAUSED)
					{
						omo_resume_player(app->player);
					}
					t3f_key[ALLEGRO_KEY_C] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_V])
				{
					omo_stop_player(app->player);
					t3f_key[ALLEGRO_KEY_V] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_B])
				{
					app->button_pressed = 3;
					t3f_key[ALLEGRO_KEY_B] = 0;
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
