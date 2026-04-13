#ifndef _T3F_COLOR_H
#define _T3F_COLOR_H

#include "t3f/t3f.h"

ALLEGRO_COLOR t3f_fade_color_to(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float amount);
ALLEGRO_COLOR t3f_alpha_color(ALLEGRO_COLOR c1, float new_a, int flags);

#endif
