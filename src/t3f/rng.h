#ifndef T3F_RNG_H
#define T3F_RNG_H

#include <allegro5/allegro5.h>

typedef struct
{
	
	long state;

} T3F_RNG_STATE;

typedef struct
{

	int * pool;
	int size;
	int count;

} T3F_RNG_POOL;

void t3f_srand(T3F_RNG_STATE * rp, unsigned long seed);
int t3f_rand(T3F_RNG_STATE * rp);
double t3f_drand(T3F_RNG_STATE * rp);
int t3f_random(T3F_RNG_STATE * rp, int max);
double t3f_drandom(T3F_RNG_STATE * rp, double max);

T3F_RNG_POOL * t3f_create_rng_pool(int size);
void t3f_destroy_rng_pool(T3F_RNG_POOL * rng_pool);
bool t3f_add_to_rng_pool(T3F_RNG_POOL * rng_pool, int i);
int t3f_get_from_rng_pool(T3F_RNG_POOL * rng_pool, T3F_RNG_STATE * rng_state);

#endif
