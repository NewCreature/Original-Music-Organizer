#ifndef T3F_BITMAP_H
#define T3F_BITMAP_H

bool t3f_resize_bitmap(ALLEGRO_BITMAP ** bp, int w, int h, bool hq, int flags);
bool t3f_squeeze_bitmap(ALLEGRO_BITMAP ** bp, int * ow, int * oh);

#endif
