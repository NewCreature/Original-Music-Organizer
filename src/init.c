#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "main.h"
#include "events.h"
#include "ui/menu_init.h"
#include "file_helpers.h"
#include "test.h"
#include "library_helpers.h"
#include "queue_helpers.h"
#include "library_cache.h"
#include "profile.h"

#include "archive_handlers/libarchive/libarchive.h"
#include "archive_handlers/unrar/unrar.h"

#include "codec_handlers/dumba5/dumba5.h"
#include "codec_handlers/allegro_acodec/allegro_acodec.h"
#include "codec_handlers/gme/gme.h"
#include "codec_handlers/mp3a5/mp3a5.h"
#include "codec_handlers/midia5/midia5.h"
#include "codec_handlers/adplug/adplug.h"

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codec_handlers/avmidiplayer/avmidiplayer.h"
	#include "codec_handlers/avplayer/avplayer.h"
#endif

static bool setup_temp_folder(ALLEGRO_PATH ** path, const char * name)
{
	*path = al_clone_path(t3f_data_path);
	if(*path)
	{
		al_append_path_component(*path, name);
		if(al_make_directory(al_path_cstr(*path, '/')))
		{
			return true;
		}
		al_destroy_path(*path);
		*path = NULL;
	}
	return false;
}

static bool setup_temp_folders(APP_INSTANCE * app)
{
	if(!setup_temp_folder(&app->library_temp_path, "temp/library"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->queue_temp_path, "temp/queue"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->queue_tags_temp_path, "temp/queue_tags"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->player_temp_path, "temp/player"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->cloud_temp_path, "temp/cloud"))
	{
		return false;
	}
	return true;
}

void omo_set_window_constraints(APP_INSTANCE * app)
{
	int min_width = 0;
	int min_height = 0;
	int new_width, new_height;
	int bitmap_index[6] = {OMO_THEME_BITMAP_PREVIOUS_TRACK, OMO_THEME_BITMAP_PLAY, OMO_THEME_BITMAP_STOP, OMO_THEME_BITMAP_NEXT_TRACK, OMO_THEME_BITMAP_OPEN, OMO_THEME_BITMAP_ADD};
	int i;

	/* calculate miminum width for current theme */
	for(i = 0; i < 6; i++)
	{
		if(app->ui->main_theme->bitmap[bitmap_index[i]])
		{
			min_width += al_get_bitmap_width(app->ui->main_theme->bitmap[bitmap_index[i]]) + 4;
		}
		else
		{
			if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0])
			{
				min_width += al_get_text_width(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0], app->ui->main_theme->text[bitmap_index[i]]) + 4;
			}
		}
	}
	if(app->library_view)
	{
		min_width *= 4;
	}
	min_width += 16; // borders
	if(app->library_view)
	{
		min_width += 24;
	}

	/* calculate minimum height for current theme */
	if(app->ui->main_theme->bitmap[0])
	{
		min_height += al_get_bitmap_height(app->ui->main_theme->bitmap[0]) + 4;
	}
	else
	{
		if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0])
		{
			min_height += al_get_font_line_height(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0]) + 4;
		}
	}
	if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0])
	{
		min_height += al_get_font_line_height(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0]) * 8;
	}
	min_height += 24; // borders

	al_set_window_constraints(t3f_display, min_width, min_height, 0, 0);
	if(al_get_display_width(t3f_display) < min_width || al_get_display_height(t3f_display) < min_height)
	{
		new_width = al_get_display_width(t3f_display) < min_width ? min_width : al_get_display_width(t3f_display);
		new_height = al_get_display_height(t3f_display) < min_height ? min_height : al_get_display_height(t3f_display);
		al_resize_display(t3f_display, new_width, new_height);
		omo_resize_ui(app->ui, app->library_view ? 1 : 0, new_width, new_height);
	}
}

/* initialize our app, load graphics, etc. */
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	OMO_FILE_HELPER_DATA file_helper_data;
	char buffer[1024];
	bool used_arg[1024] = {false};
	const char * val;
	int test_path;
	int test_mode = 0;
	double start_pos;
	int player_state;
	int i;

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, omo_logic, omo_render, T3F_DEFAULT | T3F_RESIZABLE | T3F_USE_MENU, app))
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

	setup_temp_folders(app);

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
	#ifdef ALLEGRO_MACOSX
//		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avmidiplayer_get_codec_handler());
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avplayer_get_codec_handler());
	#endif
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_dumba5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_allegro_acodec_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_gme_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_mp3a5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_midia5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_adplug_get_codec_handler());

	/* create default profile */
	if(!omo_setup_profile("Default"))
	{
		printf("Error setting up default profile!\n");
		return false;
	}

	app->player = omo_create_player();
	if(!app->player)
	{
		printf("Error creating player!\n");
		return false;
	}
	if(argc > 1)
	{
		/* check for command line options */
		for(i = 1; i < argc; i++)
		{
			if(!strcmp(argv[i], "--test"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --test <test_files_path>\n\n");
					return false;
				}
				else
				{
					test_path = i + 1;
					app->test_mode = true;
				}
			}
			else if(!strcmp(argv[i], "--quick-test"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --quick-test <test_files_path>\n\n");
					return false;
				}
				else
				{
					test_path = i + 1;
					app->test_mode = true;
					test_mode = 1;
				}
			}
			else if(!strcmp(argv[i], "--prune-library"))
			{
				app->prune_library = true;
			}
			else if(!strcmp(argv[i], "--ignore-genre"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --ignore-genre <genre name>\n\n");
					return false;
				}
				else
				{
					if(!strcmp(argv[i + 1], "none"))
					{
						al_remove_config_key(t3f_config, "Settings", "Ignore Genre");
					}
					else
					{
						al_set_config_value(t3f_config, "Settings", "Ignore Genre", argv[i + 1]);
					}
				}
			}
		}

		/* don't add files if we are running the test suite */
		if(!app->test_mode)
		{
			omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
			for(i = 1; i < argc; i++)
			{
				if(!used_arg[i])
				{
					if(!t3f_scan_files(argv[i], omo_count_file, false, &file_helper_data))
					{
						omo_count_file(argv[i], false, &file_helper_data);
					}
				}
			}
			if(file_helper_data.file_count > 0)
			{
				app->player->queue = omo_create_queue(file_helper_data.file_count);
				if(app->player->queue)
				{
					file_helper_data.queue = app->player->queue;
					for(i = 1; i < argc; i++)
					{
						if(!used_arg[i])
						{
							if(!t3f_scan_files(argv[i], omo_queue_file, false, &file_helper_data))
							{
								omo_queue_file(argv[i], false, &file_helper_data);
							}
						}
					}
					if(app->player->queue->entry_count)
					{
						app->player->queue_pos = 0;
						omo_start_player(app->player);
					}
					app->spawn_queue_thread = true;
				}
			}
		}
	}

	if(!omo_setup_menus(app))
	{
		printf("Error setting up menus!\n");
		return false;
	}
	val = al_get_config_value(t3f_config, "Settings", "disable_menu");
	if(!val || strcmp(val, "true"))
	{
		t3f_attach_menu(app->menu[OMO_MENU_MAIN]);
	}

	app->ui = omo_create_ui();
	if(!app->ui)
	{
		printf("Error settings up dialogs!\n");
		return false;
	}
	t3f_srand(&app->rng_state, time(0));
	app->state = 0;
	app->button_pressed = -1;
	app->library_view = false;
	val = al_get_config_value(t3f_config, "Settings", "last_view");
	if(val && !strcmp(val, "library"))
	{
		app->library_view = true;
	}
	if(!omo_create_main_dialog(app->ui, app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), app))
	{
		printf("Unable to create main dialog!\n");
		return false;
	}

	omo_set_window_constraints(app);
	t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);

	/* set up library */
	if(app->test_mode)
	{
		omo_test_init(app, test_mode, argv[test_path]);
	}
	else
	{
		app->spawn_library_thread = true;
		if(!app->player->queue)
		{
			app->player->queue = omo_load_queue(t3f_get_filename(t3f_data_path, "omo.queue", buffer, 1024));
			if(app->player->queue)
			{
				val = al_get_config_value(t3f_config, "Settings", "queue_position");
				if(val)
				{
					app->player->queue_pos = atoi(val);
					app->ui->ui_queue_list_element->d1 = app->player->queue_pos;
					app->ui->ui_queue_list_element->d2 = app->ui->ui_queue_list_element->d1;
					val = al_get_config_value(t3f_config, "Settings", "queue_track_position");
					if(val)
					{
						start_pos = atof(val);
						val = al_get_config_value(t3f_config, "Settings", "player_state");
						if(val)
						{
							player_state = atoi(val);
							omo_start_player(app->player);
							omo_player_logic(app->player, app->library, app->archive_handler_registry, app->codec_handler_registry, app->player_temp_path);
							if(app->player->track && app->player->track->codec_handler->seek)
							{
								app->player->track->codec_handler->seek(app->player->track->codec_data, start_pos);
							}
							if(player_state == OMO_PLAYER_STATE_PAUSED)
							{
								omo_pause_player(app->player);
							}
						}
					}
				}
				app->spawn_queue_thread = true;
			}
		}
	}
	return true;
}

void omo_exit(APP_INSTANCE * app)
{
	char buffer[1024];
	char buf[32] = {0};

	al_remove_filename(t3f_get_filename(t3f_data_path, "omo.queue", buffer, 1024));
	al_remove_config_key(t3f_config, "Settings", "queue_position");
	if(app->player->queue)
	{
		if(omo_save_queue(app->player->queue, t3f_get_filename(t3f_data_path, "omo.queue", buffer, 1024)))
		{
			sprintf(buf, "%d", app->player->queue_pos);
			al_set_config_value(t3f_config, "Settings", "queue_position", buf);
		}
	}
	t3f_save_config();
	omo_destroy_player(app->player);
	app->player = NULL;
	omo_cancel_library_setup(app);
	if(app->cloud_thread)
	{
		al_destroy_thread(app->cloud_thread);
	}
	t3f_remove_directory(t3f_get_filename(t3f_data_path, "temp", buffer, 1024));
	if(app->ui->tags_popup_dialog)
	{
		omo_close_tags_dialog(app->ui, app);
	}
	if(app->ui->split_track_popup_dialog)
	{
		omo_close_split_track_dialog(app->ui, app);
	}
	if(app->ui->tagger_key_popup_dialog)
	{
		omo_close_tagger_key_dialog(app->ui, app);
	}
	if(app->ui->new_profile_popup_dialog)
	{
		omo_close_new_profile_dialog(app->ui, app);
	}
	if(app->ui->filter_popup_dialog)
	{
		omo_close_filter_dialog(app->ui, app);
	}
	t3gui_close_dialog(app->ui->ui_dialog);
	omo_destroy_ui(app->ui);
}
