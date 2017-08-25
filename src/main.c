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

static void omo_toggle_library_view(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * v_x;
	const char * v_y;
	const char * v_width;
	const char * v_height;
	int c_x, c_y, c_width, c_height, c_old_width;
	char buf[32] = {0};
	ALLEGRO_MONITOR_INFO monitor_info;

	t3gui_close_dialog(app->ui->ui_dialog);

	al_get_window_position(t3f_display, &c_x, &c_y);
	c_width = al_get_display_width(t3f_display);
	c_height = al_get_display_height(t3f_display);

	app->library_view = !app->library_view;
	if(app->library_view)
	{
		v_x = al_get_config_value(t3f_config, "Settings", "library_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "library_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "library_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "library_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "basic_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "basic_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "basic_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "basic_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "library");
	}
	else
	{
		v_x = al_get_config_value(t3f_config, "Settings", "basic_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "basic_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "basic_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "basic_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "library_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "library_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "library_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "library_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "basic");
	}
	if(v_x && v_y && v_width && v_height)
	{
		c_x = atoi(v_x);
		c_y = atoi(v_y);
		c_width = atoi(v_width);
		c_height = atoi(v_height);
	}
	else
	{
		al_get_monitor_info(0, &monitor_info);
		if(app->library_view)
		{
			c_old_width = c_width;
			c_width *= 4;
			if(c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_width = monitor_info.x2 - monitor_info.x1;
			}
			c_x -= c_width - c_old_width;
			if(c_x < 0)
			{
				c_x = 0;
			}
		}
		else
		{
			c_old_width = c_width;
			c_width /= 4;
			c_x += c_old_width - c_width;
			if(c_x + c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_x = monitor_info.x2 - c_width;
			}
		}
	}
	al_set_window_position(t3f_display, c_x, c_y);
	al_resize_display(t3f_display, c_width, c_height);
	omo_create_main_dialog(app->ui, app->library_view ? 1 : 0, c_width, c_height, app);
	t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
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
					omo_get_library_song_list(app->library, val2, "All Albums");
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
					if(!app->library_view)
					{
						val = al_get_config_value(t3f_config, "Settings", "library_folders");
						if(!val || atoi(val) < 1)
						{
							omo_start_file_chooser(data, "Select library folder.", al_get_config_value(t3f_config, "Settings", "last_music_folder"), ALLEGRO_FILECHOOSER_FOLDER, false);
							if(app->file_chooser && al_get_native_file_dialog_count(app->file_chooser))
							{
		                        al_set_config_value(t3f_config, "Settings", "library_folder_0", al_get_native_file_dialog_path(app->file_chooser, 0));
		                        al_set_config_value(t3f_config, "Settings", "library_folders", "1");
		                        omo_destroy_library(app->library);
		                        omo_setup_library(app, omo_library_setup_update_proc);
							}
						}
					}
					val = al_get_config_value(t3f_config, "Settings", "library_folders");
					if(val && atoi(val) > 0)
					{
						omo_toggle_library_view(app);
					}
					t3f_key[ALLEGRO_KEY_L] = 0;
				}
				if(t3f_key[ALLEGRO_KEY_T] && app->library)
				{
					if(app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
					{
						j = app->ui->ui_queue_list_element->d1;
						strcpy(fullfn, app->player->queue->entry[j]->file);
						if(app->player->queue->entry[j]->sub_file)
						{
							strcat(fullfn, "/");
							strcat(fullfn, app->player->queue->entry[j]->sub_file);
						}
					}
					else if(app->ui->ui_song_list_element->flags & D_GOTFOCUS)
					{
						j = app->ui->ui_song_list_element->d1;
						strcpy(fullfn, app->library->entry[app->library->song_entry[j]]->filename);
						if(app->library->entry[app->library->song_entry[j]]->sub_filename)
						{
							strcat(fullfn, "/");
							strcat(fullfn, app->library->entry[app->library->song_entry[j]]->sub_filename);
						}
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
				if(t3f_key[ALLEGRO_KEY_DELETE])
				{
					if(app->player->queue && app->ui->ui_queue_list_element->d1 < app->player->queue->entry_count)
					{
						if(app->ui->ui_queue_list_element->d1 == app->player->queue_pos)
						{
							omo_stop_player(app->player);
						}
						omo_delete_queue_item(app->player->queue, app->ui->ui_queue_list_element->d1);
						if(app->player->queue_pos > app->ui->ui_queue_list_element->d1)
						{
							app->player->queue_pos--;
						}
						if(app->player->queue->entry_count > 0)
						{
							omo_start_player(app->player);
						}
						else
						{
							omo_destroy_queue(app->player->queue);
							app->player->queue = NULL;
						}
					}
					t3f_key[ALLEGRO_KEY_DELETE] = 0;

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
				if(app->player->state == OMO_PLAYER_STATE_PLAYING || app->player->state == OMO_PLAYER_STATE_PAUSED)
				{
					app->ui->ui_queue_list_element->id2 = app->player->queue_pos;
				}
				else
				{
					app->ui->ui_queue_list_element->id2 = -1;
				}
				if(app->library_view)
				{
					app->ui->ui_artist_list_element->id2 = app->ui->ui_artist_list_element->d1;
					app->ui->ui_album_list_element->id2 = app->ui->ui_album_list_element->d1;
				}
				break;
			}
			else
			{
				if(t3f_key[ALLEGRO_KEY_ESCAPE])
				{
					omo_close_tags_dialog(app->ui, app);
					t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
				}
				if(app->button_pressed == 0)
				{
					for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				    {
				        if(omo_tag_type[i] && strcmp(app->ui->tags_text[i], app->ui->original_tags_text[i]))
				        {
				            al_set_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i], app->ui->tags_text[i]);
				        }
				    }
					omo_close_tags_dialog(app->ui, app);
					app->button_pressed = -1;
					t3f_key[ALLEGRO_KEY_ENTER] = 0;
				}
				else if(app->button_pressed == 1)
				{
					omo_close_tags_dialog(app->ui, app);
					app->button_pressed = -1;
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
	else
	{
		printf("Error: could not initialize T3F!\n");
	}
	return 0;
}
