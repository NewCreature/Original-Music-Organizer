#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "t3net/t3net.h"
#include "instance.h"
#include "main.h"
#include "events.h"
#include "ui/menu_init.h"
#include "ui/tags_dialog.h"
#include "ui/split_track_dialog.h"
#include "ui/tagger_key_dialog.h"
#include "ui/new_profile_dialog.h"
#include "ui/filter_dialog.h"
#include "file_helpers.h"
#include "test.h"
#include "library_helpers.h"
#include "queue_helpers.h"
#include "library_cache.h"
#include "profile.h"
#include "command_line.h"
#include "cloud.h"

#include "archive_handlers/unzip/unzip.h"
#include "archive_handlers/unrar/unrar.h"

#include "codec_handlers/dumba5/dumba5.h"
#include "codec_handlers/allegro_acodec/allegro_acodec.h"
#include "codec_handlers/gme/gme.h"
#include "codec_handlers/mp3a5/mp3a5.h"
#include "codec_handlers/midia5/midia5.h"
#include "codec_handlers/adplug/adplug.h"
#include "codec_handlers/stsound/stsound.h"

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

void omo_configure_codec_handlers(APP_INSTANCE * app)
{
	char section[256];
	const char * val;

	val = al_get_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section), "disabled_codec_handlers");
	omo_reconfigure_codec_handlers(app->codec_handler_registry, val);
}

static void provide_default_urls(void)
{
	const char * val;

	val = al_get_config_value(t3f_config, "Settings", "get_tagger_key_url");
	if(!val)
	{
		al_set_config_value(t3f_config, "Settings", "get_tagger_key_url", "https://www.tcubedsoftware.com/scripts/omo/get_tagger_key.php");
	}
	val = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
	if(!val)
	{
		al_set_config_value(t3f_config, "Settings", "get_track_tags_url", "https://www.tcubedsoftware.com/scripts/omo/get_track_tags.php");
	}
	val = al_get_config_value(t3f_config, "Settings", "tag_track_url");
	if(!val)
	{
		al_set_config_value(t3f_config, "Settings", "tag_track_url", "https://www.tcubedsoftware.com/scripts/omo/tag_track.php");
	}
}

static void migrate_databases(void)
{
	ALLEGRO_PATH * pp = NULL;
	ALLEGRO_PATH * migrate_from_path = NULL;
	const char * old_org_name;
	const char * old_app_name;
	ALLEGRO_PATH * migrate_to_path = NULL;
	ALLEGRO_CONFIG * config = NULL;

	old_org_name = al_get_org_name();
	old_app_name = al_get_app_name();
	if(old_org_name && old_app_name)
	{
		migrate_to_path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);
		if(migrate_to_path)
		{
			/* see if we need to migrate before proceeding */
			al_set_org_name("t3-i");
			al_set_app_name("OMO");
			migrate_from_path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);
			if(migrate_from_path)
			{
				al_set_path_filename(migrate_from_path, "files.ini");
				al_set_path_filename(migrate_to_path, "files.ini");
				if(!al_filename_exists(al_path_cstr(migrate_to_path, '/')))
				{
					config = al_load_config_file(al_path_cstr(migrate_from_path, '/'));
					if(config)
					{
						al_save_config_file(al_path_cstr(migrate_to_path, '/'), config);
						al_destroy_config(config);
					}
				}
				al_set_path_filename(migrate_from_path, "database.ini");
				al_set_path_filename(migrate_to_path, "database.ini");
				if(!al_filename_exists(al_path_cstr(migrate_to_path, '/')))
				{
					config = al_load_config_file(al_path_cstr(migrate_from_path, '/'));
					if(config)
					{
						al_save_config_file(al_path_cstr(migrate_to_path, '/'), config);
						al_destroy_config(config);
					}
				}
				al_destroy_path(migrate_from_path);
			}
			al_destroy_path(migrate_to_path);
		}
	}
}

/* initialize our app, load graphics, etc. */
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	char buffer[1024];
	const char * val;
	double start_pos;
	int player_state;

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, omo_logic, omo_render, T3F_DEFAULT | T3F_RESIZABLE | T3F_USE_MENU | T3F_USE_OPENGL, app))
	{
		printf("Error initializing T3F!\n");
		return false;
	}
	t3f_set_event_handler(omo_event_handler);

	t3net_setup(NULL, al_path_cstr(t3f_temp_path, '/'));
	provide_default_urls();
	migrate_databases();
	if(!omo_init_cloud())
	{
		printf("Failed to initialize cloud module!\n");
		return false;
	}

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
	omo_register_archive_handler(app->archive_handler_registry, omo_get_unzip_archive_handler());
	omo_register_archive_handler(app->archive_handler_registry, omo_get_unrar_archive_handler());

	/* register players */
	app->codec_handler_registry = omo_create_codec_handler_registry();
	if(!app->codec_handler_registry)
	{
		printf("Error setting up player registry!\n");
		return false;
	}
	#ifdef ALLEGRO_MACOSX
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avplayer_get_codec_handler());
	#endif
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_dumba5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_allegro_acodec_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_gme_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_mp3a5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_midia5_get_codec_handler());
	#ifdef ALLEGRO_MACOSX
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avmidiplayer_get_codec_handler());
	#endif
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_adplug_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_stsound_get_codec_handler());
	omo_configure_codec_handlers(app);

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
	if(!omo_process_command_line_arguments(app, argc, argv))
	{
		return false;
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
	val = al_get_config_value(t3f_config, "Settings", "prefetch_tags");
	if(val && !strcmp(val, "false"))
	{
		app->prefetch_tags = false;
	}
	else
	{
		app->prefetch_tags = true;
	}
	val = al_get_config_value(t3f_config, "Settings", "disable_cloud_syncing");
	if(val && !strcmp(val, "true"))
	{
		app->disable_cloud_syncing = true;
	}
	else
	{
		app->disable_cloud_syncing = false;
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
	if(app->test_mode >= 0)
	{
		omo_test_init(app, app->test_mode, argv[app->test_path_arg]);
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
					val = al_get_config_value(t3f_config, "Settings", "queue_scroll_position");
					if(val)
					{
						app->ui->ui_queue_list_element->d2 = atoi(val);
					}
					else
					{
						app->ui->ui_queue_list_element->d2 = app->ui->ui_queue_list_element->d1;
					}
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

	/* get tagger key if we don't have one */
	val = al_get_config_value(t3f_config, "Settings", "tagger_id");
	if(!val)
	{
		omo_get_tagger_key("Anonymous");
	}
	return true;
}

void omo_exit(APP_INSTANCE * app)
{
	char buffer[1024];
	char buf[32] = {0};

	al_remove_filename(t3f_get_filename(t3f_data_path, "omo.queue", buffer, 1024));
	al_remove_config_key(t3f_config, "Settings", "queue_position");
	al_remove_config_key(t3f_config, "Settings", "queue_scroll_position");
	if(app->player->queue)
	{
		if(omo_save_queue(app->player->queue, t3f_get_filename(t3f_data_path, "omo.queue", buffer, 1024)))
		{
			sprintf(buf, "%d", app->player->queue_pos);
			al_set_config_value(t3f_config, "Settings", "queue_position", buf);
			sprintf(buf, "%d", app->ui->ui_queue_list_element->d2);
			al_set_config_value(t3f_config, "Settings", "queue_scroll_position", buf);
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
	if(app->ui->multi_tags_popup_dialog)
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
