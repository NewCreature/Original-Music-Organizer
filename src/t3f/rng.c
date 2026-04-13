#include "rng.h"

#define T3F_RAND_MAX 0xFFFF
#define T3F_RS_SCALE (1.0 / (1.0 + T3F_RAND_MAX))

void t3f_srand(T3F_RNG_STATE * rp, unsigned long seed)
{
	rp->state = (long)seed;
}

int t3f_rand(T3F_RNG_STATE * rp)
{
	return (((rp->state = rp->state * 214013L + 2531011L) >> 16) & T3F_RAND_MAX);
}

double t3f_drand(T3F_RNG_STATE * rp)
{
	double d;
	do
	{
		d = (((t3f_rand(rp) * T3F_RS_SCALE) + t3f_rand(rp)) * T3F_RS_SCALE + t3f_rand(rp)) * T3F_RS_SCALE;
	} while (d >= 1); /* round off */
	return d;
}

int t3f_random(T3F_RNG_STATE * rp, int max)
{
	return t3f_rand(rp) % max;
}

double t3f_drandom(T3F_RNG_STATE * rp, double max)
{
	return t3f_drand(rp) * max;
}

T3F_RNG_POOL * t3f_create_rng_pool(int size)
{
	T3F_RNG_POOL * rng_pool;

	rng_pool = malloc(sizeof(T3F_RNG_POOL));
	if(!rng_pool)
	{
		goto fail;
	}
	memset(rng_pool, 0, sizeof(T3F_RNG_POOL));
	rng_pool->pool = malloc(sizeof(int) * size);
	if(!rng_pool->pool)
	{
		goto fail;
	}
	memset(rng_pool->pool, 0, sizeof(int) * size);
	rng_pool->size = size;

	return rng_pool;

	fail:
	{
		t3f_destroy_rng_pool(rng_pool);
		return NULL;
	}
}

void t3f_destroy_rng_pool(T3F_RNG_POOL * rng_pool)
{
	if(rng_pool)
	{
		if(rng_pool->pool)
		{
			free(rng_pool->pool);
		}
		free(rng_pool);
	}
}

bool t3f_add_to_rng_pool(T3F_RNG_POOL * rng_pool, int i)
{
	if(rng_pool->count < rng_pool->size)
	{
		rng_pool->pool[rng_pool->count] = i;
		rng_pool->count++;
		return true;
	}
	return false;
}

static void _t3f_remove_from_rng_pool(T3F_RNG_POOL * rng_pool, int i)
{
	for(;i < rng_pool->count - 1; i++)
	{
		rng_pool->pool[i] = rng_pool->pool[i + 1];
	}
	rng_pool->count--;
}

int t3f_get_from_rng_pool(T3F_RNG_POOL * rng_pool, T3F_RNG_STATE * rng_state)
{
	int index = t3f_rand(rng_state) % rng_pool->count;
	int ret = rng_pool->pool[index];
	_t3f_remove_from_rng_pool(rng_pool, index);

	return ret;
}
