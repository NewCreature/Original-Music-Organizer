#ifndef T3F_MEMORY_H
#define T3F_MEMORY_H

extern int t3f_alloc_count;
extern unsigned long t3f_current_memory_usage;
extern unsigned long t3f_max_memory_usage;

void t3f_setup_memory_interface(void);

#endif
