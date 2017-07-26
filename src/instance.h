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

/* structure to hold all of our app-specific data */
typedef struct
{

	ALLEGRO_MENU * menu[OMO_MAX_MENUS];
	char last_music_filename[1024]; // keep track of where we were last browsing for files
	int state;
	ALLEGRO_FONT * font;
	T3GUI_DIALOG * ui_dialog;
	T3GUI_ELEMENT * ui_queue_list_box_element;
	T3GUI_ELEMENT * ui_queue_list_element;
	T3GUI_ELEMENT * ui_button_element[6];
	int button_pressed;

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry;
	OMO_LIBRARY * library;
	OMO_PLAYER * player;
	T3F_RNG_STATE rng_state;

} APP_INSTANCE;

#endif
