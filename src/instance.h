#ifndef OMO_INSTANCE_H
#define OMO_INSTANCE_H

#include "t3f/t3f.h"
#include "t3gui/t3gui.h"

#include "defines.h"
#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "queue.h"
#include "library.h"
#include "player.h"
#include "ui/ui.h"
#include "file_helpers.h"

/* structure to hold all of our app-specific data */
typedef struct
{

	OMO_UI * ui;
	ALLEGRO_MENU * menu[OMO_MAX_MENUS];
	ALLEGRO_FILECHOOSER * file_chooser;
	int file_chooser_mode;
	bool file_chooser_done;
	ALLEGRO_THREAD * file_chooser_thread;
	int state;
	int button_pressed;
	bool library_view;
	bool test_mode;

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry;
	char file_database_fn[1024];
	char entry_database_fn[1024];
	char test_path[1024];
	ALLEGRO_CONFIG * library_config;
	OMO_LIBRARY * library;
	OMO_LIBRARY * loading_library;
	OMO_FILE_HELPER_DATA loading_library_file_helper_data;
	ALLEGRO_THREAD * library_thread;
	char status_bar_text[1024];
	OMO_PLAYER * player;
	T3F_RNG_STATE rng_state;

	ALLEGRO_PATH * library_temp_path;
	ALLEGRO_PATH * queue_temp_path;
	ALLEGRO_PATH * queue_tags_temp_path;
	ALLEGRO_PATH * player_temp_path;

} APP_INSTANCE;

#endif
