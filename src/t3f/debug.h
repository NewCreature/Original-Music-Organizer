#ifndef T3F_DEBUG_H
#define T3F_DEBUG_H

bool t3f_open_debug_log(const char * fn);
void t3f_close_debug_log(void);

void t3f_debug_message(const char * format, ...);
void t3f_display_debug_message(ALLEGRO_COLOR bg, ALLEGRO_COLOR fg, float delay, const char * format, ...);

#endif
