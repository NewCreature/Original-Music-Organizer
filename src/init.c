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

#ifdef ALLEGRO_WINDOWS
	#include "codec_handlers/midiout/midiout.h"
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
	font = app->ui->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font;
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

bool omo_setup_library_helper(APP_INSTANCE * app)
{
	const char * val;
	char file_db_fn[1024];
	char entry_db_fn[1024];
	char buffer[64];
	int c, i, j;

	/* load the library databases */
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->player->queue, NULL, app->library_loading_message);
	strcpy(file_db_fn, t3f_get_filename(t3f_data_path, "files.ini"));
	strcpy(entry_db_fn, t3f_get_filename(t3f_data_path, "database.ini"));
	sprintf(app->library_loading_message, "Loading library databases...");
	app->loading_library = omo_create_library(file_db_fn, entry_db_fn);
	if(!app->loading_library)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}

	/* scan library paths */
	val = al_get_config_value(t3f_config, "Settings", "library_folders");
	if(!val || atoi(val) < 1)
	{
		sprintf(app->library_loading_message, "No Library Folders");
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	c = atoi(val);
	omo_setup_file_helper_data(&app->loading_library_file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->loading_library, app->player->queue, app->library_temp_path, app->library_loading_message);
	for(j = 0; j < c; j++)
	{
		sprintf(app->library_loading_message, "Scanning folder %d of %d...", j + 1, c);
		sprintf(buffer, "library_folder_%d", j);
		val = al_get_config_value(t3f_config, "Settings", buffer);
		if(val)
		{
			t3f_scan_files(val, omo_count_file, false, NULL, &app->loading_library_file_helper_data);
			sprintf(app->library_loading_message, "Saving progress...");
			omo_save_library(app->loading_library);
		}
	}
	omo_allocate_library(app->loading_library, app->loading_library_file_helper_data.file_count);
	for(j = 0; j < c; j++)
	{
		sprintf(buffer, "library_folder_%d", j);
		val = al_get_config_value(t3f_config, "Settings", buffer);
		if(app->loading_library_file_helper_data.cancel_scan)
		{
			app->loading_library_file_helper_data.scan_done = true;
			return false;
		}
		sprintf(app->library_loading_message, "Scanning folder %d of %d...", j + 1, c);
		t3f_scan_files(val, omo_add_file, false, NULL, &app->loading_library_file_helper_data);
		omo_save_library(app->loading_library);
		sprintf(app->library_loading_message, "Saving progress...");
	}

	/* tally up artists */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->library_loading_message, "Creating artist list...");
	omo_add_artist_to_library(app->loading_library, "All Artists");
	omo_add_artist_to_library(app->loading_library, "Unknown Artist");
	for(i = 0; i < app->loading_library->entry_count; i++)
	{
		val = al_get_config_value(app->loading_library->entry_database, app->loading_library->entry[i]->id, "Artist");
		if(val)
		{
			omo_add_artist_to_library(app->loading_library, val);
		}
	}
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->library_loading_message, "Sorting artist list...");
	if(app->loading_library->artist_entry_count > 2)
	{
		qsort(&app->loading_library->artist_entry[2], app->loading_library->artist_entry_count - 2, sizeof(char *), sort_names);
	}

	/* tally up albums */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->library_loading_message, "Creating album list...");
	omo_add_album_to_library(app->loading_library, "All Albums");
	omo_add_album_to_library(app->loading_library, "Unknown Album");
	for(i = 0; i < app->loading_library->entry_count; i++)
	{
		val = al_get_config_value(app->loading_library->entry_database, app->loading_library->entry[i]->id, "Album");
		if(val)
		{
			omo_add_album_to_library(app->loading_library, val);
		}
	}
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	if(app->loading_library->album_entry_count > 2)
	{
		sprintf(app->library_loading_message, "Sorting album list...");
		qsort(&app->loading_library->album_entry[2], app->loading_library->album_entry_count - 2, sizeof(char *), sort_names);
	}

	/* make song list */
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		app->loading_library_file_helper_data.scan_done = true;
		return false;
	}
	sprintf(app->library_loading_message, "Creating song list...");
	omo_get_library_song_list(app->loading_library, "All Artists", "All Albums");

	app->loading_library_file_helper_data.scan_done = true;
	if(app->loading_library_file_helper_data.cancel_scan)
	{
		return false;
	}
	return true;
}

static void * library_setup_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(omo_setup_library_helper(app))
	{
		app->library = app->loading_library;
	}
	return NULL;
}

void omo_cancel_library_setup(APP_INSTANCE * app)
{
	if(app->library_thread)
	{
		app->loading_library_file_helper_data.cancel_scan = true;
		al_join_thread(app->library_thread, NULL);
		app->library_thread = NULL;
	}
}

void omo_setup_library(APP_INSTANCE * app)
{
	strcpy(app->library_loading_message, "");
	app->library_thread = al_create_thread(library_setup_thread_proc, app);
	if(app->library_thread)
	{
		al_start_thread(app->library_thread);
	}
}

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
	if(!setup_temp_folder(&app->library_temp_path, "library_temp"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->queue_temp_path, "queue_temp"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->queue_tags_temp_path, "queue_tags_temp"))
	{
		return false;
	}
	if(!setup_temp_folder(&app->player_temp_path, "player_temp"))
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
			if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font)
			{
				min_width += al_get_text_width(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font, app->ui->main_theme->text[bitmap_index[i]]) + 4;
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
		if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font)
		{
			min_height += al_get_font_line_height(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font) + 4;
		}
	}
	if(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font)
	{
		min_height += al_get_font_line_height(app->ui->ui_queue_list_element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font) * 8;
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
	const char * val;
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
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avmidiplayer_get_codec_handler());
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_avplayer_get_codec_handler());
	#endif
	#ifdef ALLEGRO_WINDOWS
		omo_register_codec_handler(app->codec_handler_registry, omo_codec_midiout_get_codec_handler());
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
		omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path, NULL);
		for(i = 1; i < argc; i++)
		{
			if(!t3f_scan_files(argv[i], omo_count_file, false, NULL, &file_helper_data))
			{
				omo_count_file(argv[i], &file_helper_data);
			}
		}
		app->player->queue = omo_create_queue(file_helper_data.file_count);
		if(app->player->queue)
		{
			file_helper_data.queue = app->player->queue;
			for(i = 1; i < argc; i++)
			{
				if(!t3f_scan_files(argv[i], omo_queue_file, false, NULL, &file_helper_data))
				{
					omo_queue_file(argv[i], &file_helper_data);
				}
			}
			if(app->player->queue->entry_count)
			{
				app->player->queue_pos = 0;
				omo_start_player(app->player);
			}
			omo_get_queue_tags(app->player->queue, app->library, app);
		}
	}

	if(!omo_setup_menus(app))
	{
		printf("Error setting up menus!\n");
		return false;
	}
	val = al_get_config_value(t3f_config, "App Settings", "disable_menu");
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
	omo_setup_library(app);
	return true;
}

void omo_exit(APP_INSTANCE * app)
{
	omo_cancel_library_setup(app);
	if(app->ui->tags_display)
	{
		omo_close_tags_dialog(app->ui, app);
	}
	t3gui_close_dialog(app->ui->ui_dialog);
	omo_destroy_ui(app->ui);
}
