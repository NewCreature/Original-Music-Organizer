#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "main.h"
#include "events.h"
#include "ui/menu_init.h"
#include "file_helpers.h"

#include "archive_handlers/libarchive/libarchive.h"
#include "archive_handlers/unrar/unrar.h"

#include "codec_handlers/dumba5/dumba5.h"
#include "codec_handlers/allegro_acodec/allegro_acodec.h"
#include "codec_handlers/gme/gme.h"
#include "codec_handlers/mp3a5/mp3a5.h"

/* Mac OS X codecs */
#ifdef ALLEGRO_MACOSX
	#include "codec_handlers/avmidiplayer/avmidiplayer.h"
	#include "codec_handlers/avplayer/avplayer.h"
#endif

void omo_library_setup_update_proc(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	ALLEGRO_FONT * font;
	ALLEGRO_PATH * path;
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	int clipx, clipy, clipw, cliph;

	t3f_render(false);
	al_store_state(&old_state, ALLEGRO_STATE_TRANSFORM);
	al_get_clipping_rectangle(&clipx, &clipy, &clipw, &cliph);
	al_set_clipping_rectangle(0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display));
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_draw_filled_rectangle(0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	font = app->ui->ui_button_theme->state[0].font;
	path = al_create_path(fn);
	if(path)
	{
		al_draw_textf(font, t3f_color_white, 8, 8, 0, "add (%lu/%lu): %s", app->library->entry_count, app->library->entry_size, al_get_path_filename(path));
		al_destroy_path(path);
	}
	al_flip_display();
	al_set_clipping_rectangle(clipx, clipy, clipw, cliph);
	al_restore_state(&old_state);
}

static int sort_names(const void *e1, const void *e2)
{
	char ** s1 = (char **)e1;
	char ** s2 = (char **)e2;
    return strcasecmp(*s1, *s2);
}

bool omo_setup_library(APP_INSTANCE * app, void (*update_proc)(const char * fn, void * data))
{
	const char * val;
	char file_db_fn[1024];
	char entry_db_fn[1024];
	char buffer[64];
	int c, i, j;

	/* load the library databases */
	strcpy(file_db_fn, t3f_get_filename(t3f_data_path, "files.ini"));
	strcpy(entry_db_fn, t3f_get_filename(t3f_data_path, "database.ini"));
	app->library = omo_create_library(file_db_fn, entry_db_fn);
	if(!app->library)
	{
		return false;
	}

	/* scan library paths */
	val = al_get_config_value(t3f_config, "Settings", "library_folders");
	if(val)
	{
		c = atoi(val);
		omo_reset_file_count();
		for(j = 0; j < c; j++)
		{
			sprintf(buffer, "library_folder_%d", j);
			val = al_get_config_value(t3f_config, "Settings", buffer);
			if(val)
			{
				t3f_scan_files(val, omo_count_file, false, NULL, app);
				omo_save_library(app->library);
			}
		}
		omo_allocate_library(app->library, omo_get_file_count());
		for(j = 0; j < c; j++)
		{
			sprintf(buffer, "library_folder_%d", j);
			val = al_get_config_value(t3f_config, "Settings", buffer);
			t3f_scan_files(val, omo_add_file, false, update_proc, app);
			omo_save_library(app->library);
		}

		/* tally up artists */
		omo_add_artist_to_library(app->library, "All Artists");
		omo_add_artist_to_library(app->library, "Unknown Artist");
		for(i = 0; i < app->library->entry_count; i++)
		{
			val = al_get_config_value(app->library->entry_database, app->library->entry[i]->id, "Artist");
			if(val)
			{
				omo_add_artist_to_library(app->library, val);
			}
		}
		if(app->library->artist_entry_count > 2)
		{
			qsort(&app->library->artist_entry[2], app->library->artist_entry_count - 2, sizeof(char *), sort_names);
		}

		/* tally up albums */
		omo_add_album_to_library(app->library, "All Albums");
		omo_add_album_to_library(app->library, "Unknown Album");
		for(i = 0; i < app->library->entry_count; i++)
		{
			val = al_get_config_value(app->library->entry_database, app->library->entry[i]->id, "Album");
			if(val)
			{
				omo_add_album_to_library(app->library, val);
			}
		}
		if(app->library->album_entry_count > 2)
		{
			qsort(&app->library->album_entry[2], app->library->album_entry_count - 2, sizeof(char *), sort_names);
		}

		/* make song list */
		omo_get_library_song_list(app->library, "All Artists", "All Albums");
	}
	return true;
}

/* initialize our app, load graphics, etc. */
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	const char * val;
	int i;

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, 640, 480, 60.0, omo_logic, omo_render, T3F_DEFAULT | T3F_RESIZABLE | T3F_USE_OPENGL | T3F_USE_MENU, app))
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
	#ifdef ALLEGRO_MACOSX
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avmidiplayer_get_codec_handler());
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avplayer_get_codec_handler());
	#endif
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_dumba5_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_allegro_acodec_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_gme_get_codec_handler());
	omo_register_codec_handler(app->codec_handler_registry, omo_codec_mp3a5_get_codec_handler());

	app->player = omo_create_player();
	if(!app->player)
	{
		printf("Error creating player!\n");
		return false;
	}
	if(argc > 1)
	{
		omo_reset_file_count();
		for(i = 1; i < argc; i++)
		{
			omo_count_file(argv[i], app);
		}
		app->player->queue = omo_create_queue(omo_get_file_count());
		if(app->player->queue)
		{
			for(i = 1; i < argc; i++)
			{
				omo_queue_file(argv[i], app);
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

	t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);


	/* set up library */
	if(!omo_setup_library(app, omo_library_setup_update_proc))
	{
		printf("Failed to set up library!\n");
		return false;
	}
	return true;
}

void omo_exit(APP_INSTANCE * app)
{
	if(app->ui->tags_display)
	{
		omo_close_tags_dialog(app->ui, app);
	}
	t3gui_close_dialog(app->ui->ui_dialog);
	omo_destroy_ui(app->ui);
}
