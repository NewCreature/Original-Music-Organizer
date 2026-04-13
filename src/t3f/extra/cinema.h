#ifndef T3F_CINEMA_H
#define T3F_CINEMA_H

#include "t3f/t3f.h"
#include "t3f/view.h"
#include "t3f/animation.h"

#define T3F_CINEMA_MAX_SCRIPT_EVENTS 10000
#define T3F_CINEMA_MAX_BITMAPS  256
#define T3F_CINEMA_MAX_EVENTS  1024
#define T3F_CINEMA_MAX_DATA     256
#define T3F_CINEMA_MAX_ENTITIES  64

#define T3F_CINEMA_EVENT_CLEAR          0
#define T3F_CINEMA_EVENT_ADD_ENTITY     1
#define T3F_CINEMA_EVENT_REMOVE_ENTITY  2
#define T3F_CINEMA_EVENT_MOVE_ENTITY    3
#define T3F_CINEMA_EVENT_MOVE_CAMERA    4
#define T3F_CINEMA_EVENT_FADE_ENTITY    5
#define T3F_CINEMA_EVENT_STOP_FADE      6
#define T3F_CINEMA_EVENT_COLOR_ENTITY   7
#define T3F_CINEMA_EVENT_LOAD_SCENE     8
#define T3F_CINEMA_EVENT_ACCEL_ENTITY   9
#define T3F_CINEMA_EVENT_STOP_ENTITY   16
#define T3F_CINEMA_EVENT_STOP_CAMERA   17
#define T3F_CINEMA_EVENT_CHANGE_ENTITY 18
#define T3F_CINEMA_EVENT_PLAY_MUSIC    19
#define T3F_CINEMA_EVENT_PLAY_SOUND    20
#define T3F_CINEMA_EVENT_NONE          21
#define T3F_CINEMA_EVENT_END          255

#define T3F_CINEMA_FLAG_INTEGER_RENDERING (1 << 0)
#define T3F_CINEMA_FLAG_CONSTRAIN_CAMERA  (1 << 1)
#define T3F_CINEMA_FLAG_DEVELOPER_MODE    (1 << 2)

#define T3F_CINEMA_EVENT_FLAG_BREAK (1 << 0)
#define T3F_CINEMA_EVENT_FLAG_WAIT  (1 << 1)

#define T3F_CINEMA_STATE_PLAY 0
#define T3F_CINEMA_STATE_WAIT 1

typedef struct
{

	const char * name;
	int bitmap;
	float x, y, z, a, s;
	int tick;
	ALLEGRO_COLOR color;
	
} T3F_CINEMA_EVENT_DATA_ADD_ENTITY;

typedef struct
{
	
	const char * name;
	int entity;
	
} T3F_CINEMA_EVENT_DATA_REMOVE_ENTITY;

typedef struct
{
	
	const char * name;
	int entity;
	unsigned long tick;
	float x, y, z, a;
	float jy, gy;
	bool ux, uy, uz;
	
} T3F_CINEMA_EVENT_DATA_MOVE_ENTITY;

typedef struct
{
	
	const char * name;
	int entity;
	unsigned long tick;
	float vx, vy, vz, va;
	bool uvx, uvy, uvz, uva;
	
} T3F_CINEMA_EVENT_DATA_ACCEL_ENTITY;

typedef struct
{
	
	unsigned long tick;
	float x, y, z;
	
} T3F_CINEMA_EVENT_DATA_MOVE_CAMERA;

typedef struct
{
	
	const char * name;
	int entity;
	
} T3F_CINEMA_EVENT_DATA_STOP_ENTITY;

typedef struct
{
	
	const char * name;
	int entity;
	unsigned long tick;
	ALLEGRO_COLOR color;
	
} T3F_CINEMA_EVENT_DATA_FADE_ENTITY;

typedef struct
{
	
	const char * name;
	int entity;
	ALLEGRO_COLOR color;
	
} T3F_CINEMA_EVENT_DATA_COLOR_ENTITY;

typedef struct
{
	
	int scene;
	
} T3F_CINEMA_EVENT_DATA_LOAD_SCENE;

typedef struct
{
	
	const char * name;
	int entity;
	int bitmap;
	
} T3F_CINEMA_EVENT_DATA_CHANGE_ENTITY;

typedef struct
{
	
	const char * filename;
	int fade_ticks;
	
} T3F_CINEMA_EVENT_DATA_PLAY_MUSIC;

typedef struct
{
	
	const char * filename;
	float pan;
	
} T3F_CINEMA_EVENT_DATA_PLAY_SOUND;

typedef union
{
	
	T3F_CINEMA_EVENT_DATA_ADD_ENTITY add_entity;
	T3F_CINEMA_EVENT_DATA_REMOVE_ENTITY remove_entity;
	T3F_CINEMA_EVENT_DATA_MOVE_ENTITY move_entity;
	T3F_CINEMA_EVENT_DATA_ACCEL_ENTITY accel_entity;
	T3F_CINEMA_EVENT_DATA_MOVE_CAMERA move_camera;
	T3F_CINEMA_EVENT_DATA_STOP_ENTITY stop_entity;
	T3F_CINEMA_EVENT_DATA_FADE_ENTITY fade_entity;
	T3F_CINEMA_EVENT_DATA_COLOR_ENTITY color_entity;
	T3F_CINEMA_EVENT_DATA_LOAD_SCENE load_scene;
	T3F_CINEMA_EVENT_DATA_CHANGE_ENTITY change_entity;
	T3F_CINEMA_EVENT_DATA_PLAY_MUSIC play_music;
	T3F_CINEMA_EVENT_DATA_PLAY_SOUND play_sound;
	
} T3F_CINEMA_EVENT_DATA;

typedef struct
{
	
	unsigned long id;
	unsigned long tick;
	int type;
	T3F_CINEMA_EVENT_DATA data;
	int flags;
	
} T3F_CINEMA_EVENT;

typedef struct
{
	
	const char * name;
	int bitmap;
	float x, y, z, a, s;
	float vx, vy, vz, va;
	float by, jy, gy;
	ALLEGRO_COLOR color;
	float cr, cg, cb, ca;
	float cvr, cvg, cvb, cva;
	int tick;
	
} T3F_CINEMA_ENTITY;

typedef struct
{
	
	float x, y, z;
	float vx, vy, vz;
	float width, height;
	float render_x, render_y;
	
} T3F_CINEMA_CAMERA;

typedef struct
{

	ALLEGRO_CONFIG * script;
	char script_path[1024];
	ALLEGRO_PATH * path;
	T3F_ANIMATION * bitmap[T3F_CINEMA_MAX_BITMAPS];
	float graphics_scale;
	int flags;
	
	T3F_CINEMA_EVENT * event[T3F_CINEMA_MAX_EVENTS];
	int events;
	
	void (*view_callback)(void * data);
	void (*view_logic_callback)(void * data);
	void * view_callback_data;
	bool (*event_callback)(T3F_CINEMA_EVENT * event, void * data);
	void * event_callback_data;
	T3F_VIEW * view;
	
	/* cinema playback data */
	int state;
	unsigned long tick;
	unsigned long length;
	T3F_CINEMA_ENTITY entity[T3F_CINEMA_MAX_ENTITIES];
	int entities;
	T3F_CINEMA_CAMERA camera;
	int scene;
	bool first_frame;
	int scene_start_event;
	bool wait_skipped;

} T3F_CINEMA;

T3F_CINEMA * t3f_create_cinema(void);
void t3f_destroy_cinema(T3F_CINEMA * cp);

bool t3f_cinema_add_bitmap(T3F_CINEMA * cp, ALLEGRO_BITMAP * bp);
void t3f_cinema_remove_bitmap(T3F_CINEMA * cp, int bitmap);
bool t3f_cinema_add_event(T3F_CINEMA * cp, unsigned long id, unsigned long tick, int type, T3F_CINEMA_EVENT_DATA data, int flags);
void t3f_cinema_remove_event(T3F_CINEMA * cp, int event);

T3F_CINEMA * t3f_load_cinema(const char * fn);
T3F_CINEMA * t3f_load_cinema_script(const char * fn, float graphics_scale, int flags);
bool t3f_save_cinema(T3F_CINEMA * cp, const char * fn);
void t3f_set_cinema_view_callbacks(T3F_CINEMA * cp, void (*logic_callback)(void * data), void (*render_callback)(void * data), void * data);
void t3f_set_cinema_event_callback(T3F_CINEMA * cp, bool (*callback)(T3F_CINEMA_EVENT * event, void * data), void * data);

bool t3f_process_cinema(T3F_CINEMA * cp);
void t3f_render_cinema(T3F_CINEMA * cp);

void t3f_handle_cinema_skip(T3F_CINEMA * cp);

#endif
