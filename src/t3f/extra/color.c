#include "t3f/t3f.h"

ALLEGRO_COLOR t3f_fade_color_to(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float amount)
{
	float r1, g1, b1, a1;
	float r2, g2, b2, a2;

	al_unmap_rgba_f(c1, &r1, &g1, &b1, &a1);
	al_unmap_rgba_f(c2, &r2, &g2, &b2, &a2);

  return al_map_rgba_f(r1 + (r2 - r1) * amount, g1 + (g2 - g1) * amount, b1 + (b2 - b1) * amount, a1 + (a2 - a1) * amount);
}

ALLEGRO_COLOR t3f_alpha_color(ALLEGRO_COLOR c1, float new_a, int flags)
{
	float r, g, b, a;

	al_unmap_rgba_f(c1, &r, &g, &b, &a);
	if(!(flags & ALLEGRO_NO_PREMULTIPLIED_ALPHA))
	{
		r /= a;
		g /= a;
		b /= a;
	}
	a *= new_a;
	if(!(flags & ALLEGRO_NO_PREMULTIPLIED_ALPHA))
	{
		r *= a;
		g *= a;
		b *= a;
	}
	return al_map_rgba_f(r, g, b, a);
}
