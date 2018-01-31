#ifndef T3F_MENU_H
#define T3F_MENU_H

#include <allegro5/allegro_native_dialog.h>

#define T3F_MAX_MENU_ITEMS 512

void t3f_reset_menus(void);
int t3f_add_menu_item(ALLEGRO_MENU * mp, const char * text, int flags, ALLEGRO_MENU * cmp, int (*proc)(int id, void * data), int (*update_proc)(ALLEGRO_MENU * menu, int item, void * data));
void t3f_set_menu_item_flags(ALLEGRO_MENU * mp, int item, int flags);
int t3f_process_menu_click(int id, void * data);
void t3f_update_menus(void * data);
void t3f_refresh_menus(void);
bool t3f_attach_menu(ALLEGRO_MENU * mp);
void t3f_enable_menus(bool enabled);
bool t3f_menus_enabled(void);

#endif
