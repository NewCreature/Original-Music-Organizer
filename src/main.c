#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include <DUMBA5/dumba5.h>

#include "instance.h"
#include "library.h"
#include "ui/init.h"

#include "archive_handlers/libarchive.h"
#include "archive_handlers/unrar.h"

#include "codecs/dumba5.h"
#include "codecs/allegro_acodec.h"
#include "codecs/gme.h"

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codecs/avmidiplayer.h"
	#include "codecs/avplayer.h"
#endif

static char extracted_filename[1024] = {0};

static void stop_player(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;

	if(app->player)
	{
		app->player->stop();
		app->player = NULL;

		/* delete previously extracted file if we played from archive */
		archive_handler = omo_get_archive_handler(app->archive_handler_registry, 	app->queue->entry[app->queue_pos]->file);
		if(archive_handler)
		{
			if(strlen(extracted_filename) > 0)
			{
				al_remove_filename(extracted_filename);
			}
		}
	}
}

void omo_event_handler(ALLEGRO_EVENT * event, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			app->ui_queue_list_box_element->w = event->display.width;
			app->ui_queue_list_box_element->h = event->display.height;
			app->ui_queue_list_element->w = event->display.width - 16;
			app->ui_queue_list_element->h = event->display.height - 16;
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
	OMO_ARCHIVE_HANDLER * archive_handler;
	const char * subfile;
	bool next_file = false;

	switch(app->state)
	{
		default:
		{
			app->ui_queue_list_element->d1 = app->queue_pos;
			t3gui_logic();
			if(app->queue)
			{
				if(app->player)
				{
					if(app->player->done_playing())
					{
						stop_player(data);
						next_file = true;
					}
				}
				else
				{
					next_file = true;
				}
				if(next_file)
				{
					while(1)
					{
						app->queue_pos++;
						if(app->queue_pos < app->queue->entry_count)
						{
							archive_handler = omo_get_archive_handler(app->archive_handler_registry, app->queue->entry[app->queue_pos]->file);
							if(archive_handler)
							{
								strcpy(extracted_filename, "");
								if(app->queue->entry[app->queue_pos]->sub_file)
								{
									app->player = omo_get_player(app->player_registry, archive_handler->get_file(app->queue->entry[app->queue_pos]->file, atoi(app->queue->entry[app->queue_pos]->sub_file)));
									if(app->player)
									{
										subfile = archive_handler->extract_file(app->queue->entry[app->queue_pos]->file, atoi(app->queue->entry[app->queue_pos]->sub_file));
										if(strlen(subfile) > 0)
										{
											strcpy(extracted_filename, subfile);
											if(app->player->load_file(extracted_filename, 0))
											{
												if(app->player->play())
												{
													break;
												}
											}
										}
									}
								}
							}
							else
							{
								app->player = omo_get_player(app->player_registry, app->queue->entry[app->queue_pos]->file);
								if(app->player)
								{
									if(app->player->load_file(app->queue->entry[app->queue_pos]->file, app->queue->entry[app->queue_pos]->sub_file))
									{
										if(app->player->play())
										{
											break;
										}
									}
								}
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			if(t3f_key[ALLEGRO_KEY_LEFT])
			{
				if(app->queue)
				{
					if(app->player)
					{
						stop_player(data);
					}
					if(app->queue_pos > 0)
					{
						app->queue_pos -= 2;
					}
					else
					{
						app->queue_pos = -1;
					}
				}
				t3f_key[ALLEGRO_KEY_LEFT] = 0;
			}
			if(t3f_key[ALLEGRO_KEY_RIGHT])
			{
				if(app->player)
				{
					stop_player(data);
				}
				t3f_key[ALLEGRO_KEY_RIGHT] = 0;
			}
			/* insert logic here, as your project grows you can add more states
			 * to deal with various parts of your app (logo, title screen, in-
			 * game, etc.) */
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
	OMO_PLAYER * player;
	int i;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		for(i = 0; i < archive_handler->count_files(fn); i++)
		{
			player = omo_get_player(app->player_registry, archive_handler->get_file(fn, i));
			if(player)
			{
				total_files++;
			}
		}
	}
	else
	{
		player = omo_get_player(app->player_registry, fn);
		if(player)
		{
			total_files += player->get_track_count(fn);
		}
	}

    return false;
}

static bool add_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_PLAYER * player;
	int i, c;
	char buf[32] = {0};

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		c = archive_handler->count_files(fn);
		for(i = 0; i < c; i++)
		{
			player = omo_get_player(app->player_registry, archive_handler->get_file(fn, i));
			if(player)
			{
				sprintf(buf, "%d", i); // reference by index instead of filename
				omo_add_file_to_library(app->library, fn, buf);
			}
		}
	}
	else
	{
		player = omo_get_player(app->player_registry, fn);
		if(player)
		{
			c = player->get_track_count(fn);
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

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	const char * val;
	char file_db_fn[1024];
	char entry_db_fn[1024];

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, app_logic, app_render, T3F_DEFAULT | T3F_RESIZABLE, app))
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

	al_set_config_value(t3f_config, "T3F", "save_window_pos", "true");

	memset(app, 0, sizeof(APP_INSTANCE));
	app->font = al_create_builtin_font();
	if(!app->font)
	{
		printf("Error loading font!\n");
		return false;
	}

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
	app->player_registry = omo_create_player_registry();
	if(!app->player_registry)
	{
		printf("Error setting up player registry!\n");
		return false;
	}
	omo_register_player(app->player_registry, omo_codec_dumba5_get_player());
	omo_register_player(app->player_registry, omo_codec_allegro_acodec_get_player());
	omo_register_player(app->player_registry, omo_codec_gme_get_player());
	#ifdef ALLEGRO_MACOSX
		omo_register_player(app->player_registry, omo_codec_avmidiplayer_get_player());
		omo_register_player(app->player_registry, omo_codec_avplayer_get_player());
	#endif

	if(argc > 1)
	{
		al_set_config_value(t3f_config, "Settings", "library_path", argv[1]);
	}
	val = al_get_config_value(t3f_config, "Settings", "library_path");
	if(val)
	{
		t3f_scan_files(val, count_file, false, app);
		strcpy(file_db_fn, t3f_get_filename(t3f_data_path, "files.ini"));
		strcpy(entry_db_fn, t3f_get_filename(t3f_data_path, "database.ini"));
		app->library = omo_create_library(file_db_fn, entry_db_fn, total_files);
		t3f_scan_files(val, add_file, false, app);
	}

	if(!omo_setup_menus(app))
	{
		printf("Error setting up menus!\n");
		return false;
	}
	t3f_attach_menu(app->menu[OMO_MENU_MAIN]);
	t3gui_show_dialog(app->ui_dialog, t3f_queue, 0, app);

	t3f_srand(&app->rng_state, time(0));
	app->state = 0;
	return true;
}

int main(int argc, char * argv[])
{
	APP_INSTANCE app;

	if(app_initialize(&app, argc, argv))
	{
		t3f_run();
		stop_player(&app);
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
