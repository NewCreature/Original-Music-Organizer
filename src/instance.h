#ifndef OMO_INSTANCE_H
#define OMO_INSTANCE_H

#include "t3f/t3f.h"

#include "defines.h"
#include "player_registry.h"

/* structure to hold all of our app-specific data */
typedef struct
{

	ALLEGRO_MENU * menu[OMO_MAX_MENUS];
	char last_music_filename[1024];
	int state;

	OMO_PLAYER_REGISTRY player_registry;

} APP_INSTANCE;

#endif
