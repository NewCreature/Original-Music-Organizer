#ifndef T3F_H
#define T3F_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include <allegro5/allegro_memfile.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#ifndef ALLEGRO_ANDROID
    #include <allegro5/allegro_native_dialog.h>
#endif
#include <stdio.h>
#include <math.h>

#define T3F_USE_KEYBOARD          (1 << 0)
#define T3F_USE_MOUSE             (1 << 1)
#define T3F_USE_JOYSTICK          (1 << 2)
#define T3F_USE_SOUND             (1 << 3)
#define T3F_USE_FULLSCREEN        (1 << 4)
#define T3F_RESIZABLE             (1 << 5)
#define T3F_FORCE_ASPECT          (1 << 6)
#define T3F_NO_DISPLAY            (1 << 7)
#define T3F_USE_TOUCH             (1 << 8)
#define T3F_USE_OPENGL            (1 << 9)
#define T3F_FILL_SCREEN          (1 << 10)
#define T3F_USE_MENU             (1 << 11)
#define T3F_NO_SCALE             (1 << 12)
#define T3F_USE_FIXED_PIPELINE   (1 << 13)
#define T3F_ANY_ORIENTATION      (1 << 14)
#define T3F_RESET_DISPLAY        (1 << 15)
#define T3F_ANTIALIASING         (1 << 16)
#define T3F_DEFAULT (T3F_USE_KEYBOARD | T3F_USE_MOUSE | T3F_USE_JOYSTICK | T3F_USE_TOUCH | T3F_USE_SOUND | T3F_FORCE_ASPECT)

#define T3F_MAX_OPTIONS                 64
#define T3F_OPTION_RENDER_MODE           0
	#define T3F_RENDER_MODE_NORMAL       0
	#define T3F_RENDER_MODE_ALWAYS_CLEAR 1

#define T3F_MAX_JOYSTICKS 16

#define T3F_MAX_STACK     16

/* include all T3F modules */
#include "android.h"
#include "animation.h"
#include "atlas.h"
#include "bitmap.h"
#include "collision.h"
#include "controller.h"
#include "debug.h"
#include "draw.h"
#include "font.h"
#include "gui.h"
#include "memory.h"
#ifndef ALLEGRO_ANDROID
    #include "menu.h"
#endif
#include "music.h"
#include "primitives.h"
#include "resource.h"
#include "rng.h"
#include "sound.h"
#include "tilemap.h"
#include "vector.h"
#include "view.h"
#include "achievements.h"
#include "steam.h"
#include "async.h"
#include "mouse.h"
#include "keyboard.h"
#include "touch.h"

extern int t3f_virtual_display_width;
extern int t3f_virtual_display_height;

extern bool t3f_quit;
extern int t3f_flags;
extern int t3f_option[T3F_MAX_OPTIONS];

extern ALLEGRO_JOYSTICK * t3f_joystick[T3F_MAX_JOYSTICKS];
extern ALLEGRO_JOYSTICK_STATE t3f_joystick_state[T3F_MAX_JOYSTICKS];
extern bool t3f_joystick_state_updated[T3F_MAX_JOYSTICKS];
extern ALLEGRO_DISPLAY * t3f_display;
extern ALLEGRO_TIMER * t3f_timer;
extern ALLEGRO_EVENT_QUEUE * t3f_queue;
extern ALLEGRO_CONFIG * t3f_config;
extern ALLEGRO_CONFIG * t3f_user_data;
extern ALLEGRO_PATH * t3f_data_path;
extern ALLEGRO_PATH * t3f_config_path;
extern ALLEGRO_PATH * t3f_temp_path;

extern ALLEGRO_COLOR t3f_color_white;
extern ALLEGRO_COLOR t3f_color_black;

int t3f_initialize(const char * name, int w, int h, double fps, void (*logic_proc)(void * data), void (*render_proc)(void * data), int flags, void * data);
void t3f_set_option(int option, int value);
bool t3f_option_is_set(int option);
int t3f_set_gfx_mode(int w, int h, int flags);
void t3f_set_clipping_rectangle(int x, int y, int w, int h);
void t3f_set_event_handler(void (*proc)(ALLEGRO_EVENT * event, void * data));
void t3f_exit(void);
bool t3f_save_config(void);
bool t3f_save_user_data(void);
void t3f_event_handler(ALLEGRO_EVENT * event);
void t3f_process_events(bool ignore);
void t3f_render(bool flip);
void t3f_run(void);
void t3f_finish(void);

float t3f_distance(float x1, float y1, float x2, float y2);

bool t3f_push_state(int flags);
bool t3f_pop_state(void);

int t3f_get_joystick_number(ALLEGRO_JOYSTICK * jp);

ALLEGRO_FILE * t3f_open_file(ALLEGRO_PATH * pp, const char * fn, const char * m);
unsigned long t3f_checksum_file(const char * fn);
bool t3f_copy_file(const char * src, const char * dest);
void t3f_setup_directories(ALLEGRO_PATH * final);
char * t3f_get_filename(ALLEGRO_PATH * path, const char * fn, char * buffer, int buffer_size);

/* threading */
bool t3f_queue_call(void (*proc)(void * data), void * data);

#ifdef __cplusplus
   }
#endif

#endif
