#include "t3f/t3f.h"
#include "t3f/file_utils.h"

#include "instance.h"
#include "library.h"
#include "player.h"
#include "file_chooser.h"
#include "ui/menu_init.h"
#include "ui/menu_proc.h"
#include "ui/dialog_init.h"

#include "archive_handlers/libarchive/libarchive.h"
#include "archive_handlers/unrar/unrar.h"

#include "codec_handlers/dumba5/dumba5.h"
#include "codec_handlers/allegro_acodec/allegro_acodec.h"
#include "codec_handlers/gme/gme.h"

#ifndef ALLEGRO_MACOSX
	#include "codec_handlers/mp3a5/mp3a5.h"
#endif

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codec_handlers/avmidiplayer/avmidiplayer.h"
	#include "codec_handlers/avplayer/avplayer.h"
#endif

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			omo_resize_dialogs(data);
			break;
		}
		default:
		{
			t3f_event_handler(event);
		}
	}
}

/* main logic routine */
void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
			omo_file_chooser_logic(data);
			t3gui_logic();
			if(app->ui_queue_list_element->id1 >= 0)
			{
				if(app->player->state == OMO_PLAYER_STATE_PLAYING)
				{
					app->player->queue_pos = app->ui_queue_list_element->id1 - 1;
					omo_play_next_song(app->player);
				}
				else
				{
					app->player->queue_pos = app->ui_queue_list_element->id1;
					omo_start_player(app->player);
				}
				app->ui_queue_list_element->id1 = -1;
			}
			sprintf(app->ui_button_text[0], "|<");
			if(app->player->state == OMO_PLAYER_STATE_PLAYING)
			{
				sprintf(app->ui_button_text[1], "||");
			}
			else
			{
				sprintf(app->ui_button_text[1], ">");
			}
			sprintf(app->ui_button_text[2], "[]");
			sprintf(app->ui_button_text[3], ">|");
			sprintf(app->ui_button_text[4], "^");
			sprintf(app->ui_button_text[5], "+");
/*			if(t3f_key[ALLEGRO_KEY_UP])
			{
				app->ui_queue_list_element->d1--;
				if(app->ui_queue_list_element->d1 < 0)
				{
					app->ui_queue_list_element->d1 = 0;
				}
				t3f_key[ALLEGRO_KEY_UP] = 0;
			}
			if(t3f_key[ALLEGRO_KEY_DOWN])
			{
				app->ui_queue_list_element->d1++;
				if(app->ui_queue_list_element->d1 >= app->player->queue->entry_count)
				{
					app->ui_queue_list_element->d1 = app->player->queue->entry_count - 1;
				}
				if(app->ui_queue_list_element->d1 < app->ui_queue_list_element->d2)
				{
					app->ui_queue_list_element->d2 = app->ui_queue_list_element->d1;
				}
				else if(app->ui_queue_list_element->d1 > app->ui_queue_list_element->d2 + visible_elements)
				{
					app->ui_queue_list_element->d2 = app->ui_queue_list_element->d1;
				}
				t3f_key[ALLEGRO_KEY_DOWN] = 0;
			} */
			if(t3f_key[ALLEGRO_KEY_LEFT])
			{
				app->button_pressed = 0;
				t3f_key[ALLEGRO_KEY_LEFT] = 0;
			}
			if(t3f_key[ALLEGRO_KEY_ENTER])
			{
				if(app->player->queue_pos != app->ui_queue_list_element->d1)
				{
					omo_stop_player(app->player);
					app->player->queue_pos = app->ui_queue_list_element->d1;
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
							app->player->queue_pos = app->ui_queue_list_element->d1;
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

/* main rendering routine */
void app_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(app->state)
	{
		default:
		{
			al_clear_to_color(t3f_color_black);
			t3gui_render();
			break;
		}
	}
}

static unsigned long total_files = 0;

static bool count_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		for(i = 0; i < archive_handler->count_files(fn); i++)
		{
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(fn, i));
			if(codec_handler)
			{
				total_files++;
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			total_files += codec_handler->get_track_count(fn);
		}
	}

    return false;
}

static bool add_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i, c;
	char buf[32] = {0};

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		c = archive_handler->count_files(fn);
		for(i = 0; i < c; i++)
		{
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(fn, i));
			if(codec_handler)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				omo_add_file_to_library(app->library, fn, buf);
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			c = codec_handler->get_track_count(fn);
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_library(app->library, fn, c > 1 ? buf : NULL);
				total_files++;
			}
		}
	}

    return false;
}

static bool add_file_to_queue(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i, c;
	char buf[32] = {0};

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		c = archive_handler->count_files(fn);
		for(i = 0; i < c; i++)
		{
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(fn, i));
			if(codec_handler)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				omo_add_file_to_queue(app->player->queue, fn, buf);
			}
		}
	}
	else
	{
		codec_handler = omo_get_codec_handler(app->codec_handler_registry, fn);
		if(codec_handler)
		{
			c = codec_handler->get_track_count(fn);
			for(i = 0; i < c; i++)
			{
				sprintf(buf, "%d", i);
				omo_add_file_to_queue(app->player->queue, fn, c > 1 ? buf : NULL);
			}
		}
	}

    return false;
}

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	const char * val;
	char file_db_fn[1024];
	char entry_db_fn[1024];
	int i;

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, app_logic, app_render, T3F_DEFAULT | T3F_RESIZABLE | T3F_USE_OPENGL | T3F_USE_MENU, app))
	{
		printf("Error initializing T3F!\n");
		return false;
	}
	t3f_set_event_handler(omo_event_handler);
	if(!t3gui_init())
	{
		printf("Error initializing T3GUI!\n");
		return false;
	}
	al_set_mouse_wheel_precision(120);

	al_set_config_value(t3f_config, "T3F", "save_window_pos", "true");

	memset(app, 0, sizeof(APP_INSTANCE));

	/* register archive handlers */
	app->archive_handler_registry = omo_create_archive_handler_registry();
	if(!app->archive_handler_registry)
	{
		printf("Error setting up archive handlers!\n");
		return false;
	}
	omo_register_archive_handler(app->archive_handler_registry, omo_get_libarchive_archive_handler());
	omo_register_archive_handler(app->archive_handler_registry, omo_get_unrar_archive_handler());

	/* register players */
	app->codec_handler_registry = omo_create_codec_handler_registry();
	if(!app->codec_handler_registry)
	{
		printf("Error setting up player registry!\n");
		return false;
	}
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_dumba5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_allegro_acodec_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_gme_get_codec_handler());
	#ifndef ALLEGRO_MACOSX
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_mp3a5_get_codec_handler());
	#endif
	#ifdef ALLEGRO_MACOSX
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avmidiplayer_get_codec_handler());
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avplayer_get_codec_handler());
	#endif

	val = al_get_config_value(t3f_config, "Settings", "library_path");
	if(val)
	{
		t3f_scan_files(val, count_file, false, app);
		strcpy(file_db_fn, t3f_get_filename(t3f_data_path, "files.ini"));
		strcpy(entry_db_fn, t3f_get_filename(t3f_data_path, "database.ini"));
		app->library = omo_create_library(file_db_fn, entry_db_fn, total_files);
		t3f_scan_files(val, add_file, false, app);
	}
	app->player = omo_create_player();
	if(!app->player)
	{
		printf("Error creating player!\n");
		return false;
	}
	if(argc > 1)
	{
		total_files = 0;
		for(i = 1; i < argc; i++)
		{
			count_file(argv[i], app);
		}
		app->player->queue = omo_create_queue(total_files);
		if(app->player->queue)
		{
			for(i = 1; i < argc; i++)
			{
				add_file_to_queue(argv[i], app);
			}
			if(app->player->queue->entry_count)
			{
				app->player->queue_pos = 0;
				omo_start_player(app->player);
			}
		}
	}

	if(!omo_setup_menus(app))
	{
		printf("Error setting up menus!\n");
		return false;
	}
	t3f_attach_menu(app->menu[OMO_MENU_MAIN]);

	if(!omo_setup_dialogs(app))
	{
		printf("Error settings up dialogs!\n");
		return false;
	}
	t3gui_show_dialog(app->ui_dialog, t3f_queue, 0, app);

	t3f_srand(&app->rng_state, time(0));
	app->state = 0;
	app->button_pressed = -1;
	return true;
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;

	if(app_initialize(&app, argc, argv))
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
