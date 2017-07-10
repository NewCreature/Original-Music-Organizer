/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * dumba5.h - The user header file for DUMB with      / / \  \
 *            Allegro 5.                             | <  /   \_
 *                                                   |  \/ /\   /
 * Include this file if you wish to use DUMB          \_  /  > /
 * with Allegro 5. It will include dumb.h for you,      | \ / /
 * and provide extra functionality such as audio        |  ' /
 * stream and datafile integration.                      \__/
 */

#ifndef DUMBA5_H
#define DUMBA5_H

#include <allegro5/allegro5.h>
#include <dumb.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

typedef struct DUMBA5_PLAYER DUMBA5_PLAYER;

/* Initialize DUMBA5 with the specified resampler quality. 
   Choose from DUMB_RQ_ALIASING, DUMB_RQ_LINEAR, and DUMB_RQ_CUBIC. */
bool dumba5_init(int resampler);

/**********************************
* simple module playing functions *
**********************************/

/* Load the specified module. */
DUH * dumba5_load_module(const char * fn);

/* Play the specified module. Uses DUMBA5's internal player and hooks up to the
   default mixer. */
bool dumba5_play_module(DUH * dp, int pattern, bool loop, int frequency, bool stereo);

/* Load and play the specified module. This is a high-level function which does
   a dumba5_load_module()/dumba5_play_module() combo for you. If you need more
   control, use the functions in the advanced section. */
bool dumba5_load_and_play_module(const char * fn, int pattern, bool loop, int frequency, bool stereo);

/* Set the module pattern. */
bool dumba5_set_module_pattern(int pattern);

/* Stop the module and free all memory. */
void dumba5_stop_module(void);

/* Pause the module. */
void dumba5_pause_module(void);

/* Resume playback of the module. */
void dumba5_resume_module(void);

/* Set the volume of the module. */
void dumba5_set_module_volume(float volume);

/* Get the volume of the module. */
float dumba5_get_module_volume(void);

/* Get the position of the module. */
long dumba5_get_module_position(void);


/*********************
* advanced functions *
*********************/

/* Create a module player from the specified module. */
DUMBA5_PLAYER * dumba5_create_player(DUH * dp, int pattern, bool loop, int bufsize, int frequency, bool stereo);

/* Destroy the specified player. */
void dumba5_destroy_player(DUMBA5_PLAYER * pp);

/* Start the player. */
void dumba5_start_player(DUMBA5_PLAYER * pp);

/* Stop the player. */
void dumba5_stop_player(DUMBA5_PLAYER * pp);

/* Set the player pattern. */
bool dumba5_set_player_pattern(DUMBA5_PLAYER * pp, int pattern);

/* Pause the player. */
void dumba5_pause_player(DUMBA5_PLAYER * pp);

/* Resume playback. */
void dumba5_resume_player(DUMBA5_PLAYER * pp);

/* Set the volume of the player. */
void dumba5_set_player_volume(DUMBA5_PLAYER * pp, float volume);

/* Get the volume of the player. */
float dumba5_get_player_volume(DUMBA5_PLAYER * pp);

/* Get the position of the player. */
long dumba5_get_player_position(DUMBA5_PLAYER * pp);

#ifdef __cplusplus
	}
#endif

#endif /* DUMBA5_H */
