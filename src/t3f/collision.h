/* todo: fix tilemap collision for when x movement causes the y position to be
         past the slope, this causes a situation where the collision doesn't occur
         and the sprite will fall through the floor */

#ifndef T3F_COLLISION_H
#define T3F_COLLISION_H

#ifdef __cplusplus
   extern "C" {
#endif

#define T3F_MAX_COLLISION_POINTS    32
#define T3F_COLLISION_TILE_MAX_DATA 16

#define T3F_COLLISION_TILEMAP_FLAG_USER_DATA 1
#define T3F_COLLISION_TILEMAP_FLAG_SLOPES    2

#define T3F_COLLISION_FLAG_SOLID_TOP         1
#define T3F_COLLISION_FLAG_SOLID_BOTTOM      2
#define T3F_COLLISION_FLAG_SOLID_LEFT        4
#define T3F_COLLISION_FLAG_SOLID_RIGHT       8
#define T3F_COLLISION_FLAG_SLOPE_TOP        16
#define T3F_COLLISION_FLAG_SLOPE_BOTTOM     32
#define T3F_COLLISION_FLAG_SLOPE_LEFT       64
#define T3F_COLLISION_FLAG_SLOPE_RIGHT     128
#define T3F_COLLISION_FLAG_USER            256

typedef struct
{

	float x, y;

} T3F_COLLISION_POINT;

typedef struct
{

	T3F_COLLISION_POINT point[T3F_MAX_COLLISION_POINTS];
	int points;

} T3F_COLLISION_LIST;

typedef struct
{

	T3F_COLLISION_LIST top;
	T3F_COLLISION_LIST bottom;
	T3F_COLLISION_LIST left;
	T3F_COLLISION_LIST right;
	int flags;

} T3F_COLLISION_MAP;

/* -a collision object is separate from your program's objects
   -your program's objects should track their own positions and then update
    the position of the collision object
    -if a collision occurs, copy the collision object's position into your own object */
typedef struct
{

	T3F_COLLISION_MAP map;

	float x, y;
	float ox, oy;
	float vx, vy;

	int flags;

} T3F_COLLISION_OBJECT;

typedef struct
{

	int * user_data; // user data
  int user_data_size;
	char * slope; // allocate this when using slope
	int flags;

} T3F_COLLISION_TILE;

typedef struct
{

	T3F_COLLISION_TILE ** data;
	int width;
	int height;
	int tile_width;
	int tile_height;

	int flags;

} T3F_COLLISION_TILEMAP;

T3F_COLLISION_OBJECT * t3f_create_collision_object(float rx, float ry, float w, float h, int tw, int th, int flags);
void t3f_recreate_collision_object(T3F_COLLISION_OBJECT * cp, float rx, float ry, float w, float h, int tw, int th, int flags);
void t3f_destroy_collision_object(T3F_COLLISION_OBJECT * cp);
T3F_COLLISION_OBJECT * t3f_load_collision_object_f(ALLEGRO_FILE * fp, int tw, int th);
T3F_COLLISION_OBJECT * t3f_load_collision_object(const char * fn, int tw, int th);
bool t3f_save_collision_object_f(T3F_COLLISION_OBJECT * op, ALLEGRO_FILE * fp);
bool t3f_save_collision_object(T3F_COLLISION_OBJECT * op, const char * fn);

T3F_COLLISION_TILEMAP * t3f_create_collision_tilemap(int w, int h, int tw, int th);
void t3f_destroy_collision_tilemap(T3F_COLLISION_TILEMAP * tmp);
T3F_COLLISION_TILEMAP * t3f_load_collision_tilemap_f(ALLEGRO_FILE * fp);
T3F_COLLISION_TILEMAP * t3f_load_collision_tilemap(char * fn);
bool t3f_save_collision_tilemap_f(T3F_COLLISION_TILEMAP * tmp, ALLEGRO_FILE * fp);
bool t3f_save_collision_tilemap(T3F_COLLISION_TILEMAP * tmp, char * fn);

/* collision object movement */
void t3f_move_collision_object_x(T3F_COLLISION_OBJECT * cp, float x);
void t3f_move_collision_object_y(T3F_COLLISION_OBJECT * cp, float y);
void t3f_move_collision_object_xy(T3F_COLLISION_OBJECT * cp, float x, float y);
float t3f_get_collision_object_left_x(T3F_COLLISION_OBJECT * cp);
float t3f_get_collision_object_right_x(T3F_COLLISION_OBJECT * cp);
float t3f_get_collision_object_top_x(T3F_COLLISION_OBJECT * cp);
float t3f_get_collision_object_bottom_x(T3F_COLLISION_OBJECT * cp);

/* collision object to collision object collisions */
int t3f_check_object_collision(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_left_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_right_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_collision_x(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_top_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_bottom_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);
float t3f_get_object_collision_y(T3F_COLLISION_OBJECT * cp1, T3F_COLLISION_OBJECT * cp2);

/* access tilemap collision data more easily */
T3F_COLLISION_TILE * t3f_get_collision_tile(T3F_COLLISION_TILEMAP * tmp, float x, float y);
int t3f_get_collision_tile_x(T3F_COLLISION_TILEMAP * tmp, float x);
int t3f_get_collision_tile_y(T3F_COLLISION_TILEMAP * tmp, float y);
int t3f_get_collision_tilemap_flag(T3F_COLLISION_TILEMAP * tmp, float x, float y, int flags);
int t3f_get_collision_tilemap_data(T3F_COLLISION_TILEMAP * tmp, float x, float y, int i);
int t3f_check_collision_tilemap_flag(T3F_COLLISION_TILEMAP * tmp, float x, float y, int inflags, int exflags);

/* collision object to collision tilemap collisions */
int t3f_check_tilemap_collision_top(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
int t3f_check_tilemap_collision_bottom(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
int t3f_check_tilemap_collision_left(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
int t3f_check_tilemap_collision_right(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
int t3f_check_tilemap_collision_slope(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
int t3f_check_tilemap_collision(T3F_COLLISION_TILEMAP * tmp, T3F_COLLISION_OBJECT * cp);
float t3f_get_tilemap_collision_x(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_get_tilemap_collision_y(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_get_tilemap_slope_x(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_get_tilemap_slope_y(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);

float t3f_get_tilemap_walk_position(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp, int flags);

float t3f_find_edge_top(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_find_edge_bottom(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_find_edge_left(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);
float t3f_find_edge_right(T3F_COLLISION_OBJECT * cp, T3F_COLLISION_TILEMAP * tmp);

#ifdef __cplusplus
   }
#endif

#endif
