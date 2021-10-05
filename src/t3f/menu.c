#include "t3f.h"

#include <allegro5/allegro_native_dialog.h>

#include "menu.h"

static int t3f_current_menu_id = 1;
static ALLEGRO_MENU * t3f_menu[T3F_MAX_MENU_ITEMS] = {NULL}; // keep track of menu each item is attached to
static int (*t3f_menu_proc[T3F_MAX_MENU_ITEMS])(int id, void * data) = {NULL};
static int (*t3f_menu_update_proc[T3F_MAX_MENU_ITEMS])(ALLEGRO_MENU * menu, int item, void * data) = {NULL};
static bool t3f_refresh_menus_needed = false;
static bool t3f_menus_allowed = true;

void t3f_reset_menus(void)
{
    t3f_current_menu_id = 1;
}

int t3f_add_menu_item(ALLEGRO_MENU * mp, const char * text, int flags, ALLEGRO_MENU * cmp, int (*proc)(int id, void * data), int (*update_proc)(ALLEGRO_MENU * menu, int item, void * data))
{
    int ret_item = -1;

    if(t3f_current_menu_id < T3F_MAX_MENU_ITEMS || !text)
    {
        al_append_menu_item(mp, text, text ? t3f_current_menu_id : -1, flags, NULL, cmp);
        if(text)
        {
            t3f_menu[t3f_current_menu_id] = mp;
            t3f_menu_proc[t3f_current_menu_id] = proc;
            t3f_menu_update_proc[t3f_current_menu_id] = update_proc;
            ret_item = t3f_current_menu_id;
            t3f_current_menu_id++;
        }
    }
    return ret_item;
}

void t3f_set_menu_item_flags(ALLEGRO_MENU * mp, int item, int flags)
{
    int old_flags = al_get_menu_item_flags(mp, item) & ~ALLEGRO_MENU_ITEM_CHECKBOX;

    if(flags != old_flags)
    {
        al_set_menu_item_flags(mp, item, flags);
    }
}

int t3f_process_menu_click(int id, void * data)
{
    if(t3f_menus_allowed)
    {
        if(id < t3f_current_menu_id)
        {
            if(t3f_menu_proc[id])
            {
                return t3f_menu_proc[id](id, data);
            }
        }
    }
    return 0;
}

void t3f_update_menus(void * data)
{
  int i;

  if(t3f_refresh_menus_needed)
  {
    for(i = 0; i < t3f_current_menu_id; i++)
    {
      if(t3f_menu_update_proc[i])
      {
        t3f_menu_update_proc[i](t3f_menu[i], i, data);
      }
    }
    t3f_refresh_menus_needed = false;
  }
}

void t3f_refresh_menus(void)
{
  t3f_refresh_menus_needed = true;
}

bool t3f_attach_menu(ALLEGRO_MENU * mp)
{
  al_register_event_source(t3f_queue, al_get_default_menu_event_source());
  al_set_display_menu(t3f_display, mp);
  t3f_adjust_view(t3f_default_view, 0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), t3f_virtual_display_width / 2, t3f_virtual_display_height / 2, t3f_flags);
	t3f_default_view->need_update = true;
	t3f_select_view(t3f_default_view);
  return true;
}

void t3f_enable_menus(bool enabled)
{
    t3f_menus_allowed = enabled;
}

bool t3f_menus_enabled(void)
{
    return t3f_menus_allowed;
}
