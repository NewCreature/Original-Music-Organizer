#ifndef OMO_INSTANCE_H
#define OMO_INSTANCE_H

#include "t3f/t3f.h"

#include "defines.h"
#include "archive_handler_registry.h"
#include "player_registry.h"
#include "queue.h"

/* structure to hold all of our app-specific data */
typedef struct
{

	ALLEGRO_MENU * menu[OMO_MAX_MENUS];
	char last_music_filename[1024];
	int state;
	ALLEGRO_FONT * font;

	OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry;
	OMO_PLAYER_REGISTRY * player_registry;
	OMO_PLAYER * player; // current player
	OMO_QUEUE * queue;
	int queue_pos;

} APP_INSTANCE;

#endif
