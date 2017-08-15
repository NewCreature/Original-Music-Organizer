#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "main.h"
#include "events.h"
#include "ui/menu_init.h"

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

static unsigned long total_files = 0;

static bool count_file(const char * fn, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	OMO_CODEC_HANDLER * codec_handler;
	int i, c;
	const char * val;
	const char * target_fn;
	char buf[32] = {0};

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		else
		{
			c = archive_handler->count_files(fn);
			sprintf(buf, "%d", c);
			al_set_config_value(app->library->file_database, fn, "archive_files", buf);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			else
			{
				target_fn = archive_handler->get_file(fn, i);
				al_set_config_value(app->library->file_database, fn, buf, target_fn);
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
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
	int i, c = 0;
	char buf[32] = {0};
	const char * val;
	const char * target_fn = NULL;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
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
	int i, c = 0;
	char buf[32] = {0};
	const char * val;
	const char * target_fn;

	archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
	if(archive_handler)
	{
		val = al_get_config_value(app->library->file_database, fn, "archive_files");
		if(val)
		{
			c = atoi(val);
		}
		for(i = 0; i < c; i++)
		{
			sprintf(buf, "entry_%d", i);
			val = al_get_config_value(app->library->file_database, fn, buf);
			if(val)
			{
				target_fn = val;
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn);
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

static bool omo_setup_library(APP_INSTANCE * app)
{
	const char * val;
	char file_db_fn[1024];
	char entry_db_fn[1024];
	int i;

	val = al_get_config_value(t3f_config, "Settings", "library_path");
	if(val)
	{
		strcpy(file_db_fn, t3f_get_filename(t3f_data_path, "files.ini"));
		strcpy(entry_db_fn, t3f_get_filename(t3f_data_path, "database.ini"));
		app->library = omo_create_library(file_db_fn, entry_db_fn);
		if(!app->library)
		{
			return false;
		}
		t3f_scan_files(val, count_file, false, app);
		omo_allocate_library(app->library, total_files);
		t3f_scan_files(val, add_file, false, app);

		/* tally up artists */
		omo_add_artist_to_library(app->library, "All");
		omo_add_artist_to_library(app->library, "Unknown");
		for(i = 0; i < app->library->entry_count; i++)
		{
			val = al_get_config_value(app->library->entry_database, app->library->entry[i]->id, "Artist");
			if(val)
			{
				omo_add_artist_to_library(app->library, val);
			}
		}

		/* tally up albums */
		omo_add_album_to_library(app->library, "All");
		omo_add_album_to_library(app->library, "Unknown");
		for(i = 0; i < app->library->entry_count; i++)
		{
			val = al_get_config_value(app->library->entry_database, app->library->entry[i]->id, "Album");
			if(val)
			{
				omo_add_album_to_library(app->library, val);
			}
		}
	}
	return true;
}

/* initialize our app, load graphics, etc. */
bool omo_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
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

	if(!omo_setup_library(app))
	{
		printf("Failed to set up library!\n");
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

	app->ui = omo_create_ui(0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), app);
	if(!app->ui)
	{
		printf("Error settings up dialogs!\n");
		return false;
	}
	t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);

	t3f_srand(&app->rng_state, time(0));
	app->state = 0;
	app->button_pressed = -1;
	return true;
}
