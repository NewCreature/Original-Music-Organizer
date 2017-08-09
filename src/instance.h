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

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry;
	OMO_LIBRARY * library;
	OMO_PLAYER * player;
	T3F_RNG_STATE rng_state;

} APP_INSTANCE;

#endif
