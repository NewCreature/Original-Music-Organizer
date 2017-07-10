#ifndef T3F_RNG_H
#define T3F_RNG_H

typedef struct
{
	
	long state;

} T3F_RNG_STATE;

void t3f_srand(T3F_RNG_STATE * rp, unsigned long seed);
int t3f_rand(T3F_RNG_STATE * rp);
double t3f_drand(T3F_RNG_STATE * rp);
int t3f_random(T3F_RNG_STATE * rp, int max);
double t3f_drandom(T3F_RNG_STATE * rp, double max);

#endif
