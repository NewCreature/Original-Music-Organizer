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
