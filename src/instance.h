#ifndef OMO_INSTANCE_H
#define OMO_INSTANCE_H

#include "t3f/t3f.h"
#include "t3gui/t3gui.h"

#include "defines.h"
#include "archive_handler_registry.h"
#include "player_registry.h"
#include "queue.h"
#include "library.h"

/* structure to hold all of our app-specific data */
typedef struct
{

	ALLEGRO_MENU * menu[OMO_MAX_MENUS];
	char last_music_filename[1024];
	int state;
	ALLEGRO_FONT * font;
	T3GUI_DIALOG * ui_dialog;
	T3GUI_ELEMENT * ui_queue_list_element;

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_PLAYER_REGISTRY * player_registry;
	OMO_PLAYER * player; // current player
	OMO_QUEUE * queue;
	OMO_LIBRARY * library;
	int queue_pos;
	T3F_RNG_STATE rng_state;

} APP_INSTANCE;

#endif
