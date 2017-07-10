#ifndef T3F_DEBUG_H
#define T3F_DEBUG_H

bool t3f_open_debug_log(const char * fn);
void t3f_close_debug_log(void);

void t3f_debug_message(const char * format, ...);

#endif
