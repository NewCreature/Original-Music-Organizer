#include <allegro5/events.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include "dialog.h"
#include "assert.h"

/* find_mouse_object:
 *  Finds which object the mouse is on top of.
 */
int t3gui_find_mouse_object(T3GUI_ELEMENT *d, int mouse_x, int mouse_y)
{
   int mouse_object = -1;
   int c;
   int res;
   assert(d);

   for (c=0; d[c].proc; c++) {
      if ((mouse_x >= d[c].x) && (mouse_y >= d[c].y) &&
          (mouse_x < d[c].x + d[c].w) && (mouse_y < d[c].y + d[c].h) &&
          (!(d[c].flags & (D_HIDDEN | D_DISABLED)))) {
         /* check if this object wants the mouse */
         res = t3gui_object_message(d+c, MSG_WANTMOUSE, 0);
         if (!(res & D_DONTWANTMOUSE)) {
            mouse_object = c;
         }
      }
   }

   return mouse_object;
}





/* find_dialog_focus:
 *  Searches the dialog for the object which has the input focus, returning
 *  its index, or -1 if the focus is not set. Useful after do_dialog() exits
 *  if you need to know which object was selected.
 */
int t3gui_find_dialog_focus(T3GUI_ELEMENT *dialog)
{
   int c;
   assert(dialog);

   for (c=0; dialog[c].proc; c++)
      if (dialog[c].flags & D_GOTFOCUS)
         return c;

   return -1;
}







/* object_message:
 *  Sends a message to a widget.
 *  Do NOT call this from the callback function!
 */
int t3gui_object_message(T3GUI_ELEMENT *dialog, int msg, int c)
{

   int ret = 0;
   assert(dialog);

   if (msg == MSG_DRAW) {
      if (dialog->flags & D_HIDDEN)
         return D_O_K;
      al_set_clipping_rectangle(dialog->x, dialog->y, dialog->w, dialog->h);
   }

   /* Callback? */
   if (dialog->callback) {
      ret = dialog->callback(msg, dialog, c);
      if (ret & D_CALLBACK)
         return ret & ~D_CALLBACK;
   }

   ret |= dialog->proc(msg, dialog, c);

   if (ret & D_REDRAWME) {
      dialog->flags |= D_DIRTY;
      ret &= ~D_REDRAWME;
   }

   return ret;
}


/* dialog_message:
 *  Sends a message to all the objects in a dialog. If any of the objects
 *  return values other than D_O_K, returns the value and sets obj to the
 *  object which produced it.
 */
int t3gui_dialog_message(T3GUI_ELEMENT *dialog, int msg, int c, int *obj)
{
   int count, res, r, force, try;
   assert(dialog);

   force = ((msg == MSG_START) || (msg == MSG_END) || (msg >= MSG_USER));

   res = D_O_K;

   /* If a menu spawned by a d_menu_proc object is active, the dialog engine
    * has effectively been shutdown for the sake of safety. This means that
    * we can't send the message to the other objects in the dialog. So try
    * first to send the message to the d_menu_proc object and, if the menu
    * is then not active anymore, send it to the other objects as well.
    */
   //if (active_menu_player) {
   //   try = 2;
   //   menu_dialog = active_menu_player->dialog;
   //}
   //else
      try = 1;

   for (; try > 0; try--) {
      for (count=0; dialog[count].proc; count++) {
         //if ((try == 2) && (&dialog[count] != menu_dialog))
         //   continue;

         if ((force) || (!(dialog[count].flags & D_HIDDEN))) {
            r = t3gui_object_message(dialog+count, msg, c);

            if (r != D_O_K) {
               res |= r;
               if (msg != MSG_DRAW && msg != MSG_IDLE && msg != MSG_TIMER && obj)
               {
                   printf("new object: %d\n", count);
                  *obj = count;
              }
            }

            if ((msg == MSG_IDLE) && (dialog[count].flags & (D_DIRTY | D_HIDDEN)) == D_DIRTY) {
               dialog[count].flags &= ~D_DIRTY;
               res |= D_REDRAW;
            }
         }
      }

      //if (active_menu_player)
      //   break;
   }

   return res;
}

int t3gui_do_dialog(T3GUI_ELEMENT *dialog, int focus)
{
   return t3gui_do_dialog_interval(dialog, 0.0, focus);
}

int t3gui_do_dialog_interval(T3GUI_ELEMENT *dialog, double speed_sec, int focus)
{
   T3GUI_PLAYER *player = t3gui_init_dialog(dialog, focus, 0, NULL, NULL, NULL);
   ALLEGRO_TIMER *timer = NULL;

   t3gui_listen_for_events(player, al_get_keyboard_event_source());
   t3gui_listen_for_events(player, al_get_mouse_event_source());
   t3gui_listen_for_events(player, al_get_joystick_event_source());
   t3gui_listen_for_events(player, al_get_display_event_source(al_get_current_display()));

   if (speed_sec > 0.0) {
      timer = al_create_timer(speed_sec);
      t3gui_listen_for_events(player, al_get_timer_event_source(timer));
      al_start_timer(timer);
   }

   ALLEGRO_EVENT_QUEUE *menu_queue = al_create_event_queue();
   al_register_event_source(menu_queue, t3gui_get_player_event_source(player));

   t3gui_start_dialog(player);

   bool in_dialog = true;
   bool must_draw = false;
   while (in_dialog) {
      if (must_draw) {
         must_draw = false;
         t3gui_draw_dialog(player);
         al_flip_display();
      }
      al_wait_for_event(menu_queue, NULL);
      while (!al_is_event_queue_empty(menu_queue)) {
         ALLEGRO_EVENT event;
         al_get_next_event(menu_queue, &event);
         switch (event.type) {
            case T3GUI_EVENT_REDRAW:
               //printf("Redraw request\n");
               must_draw = true;
               break;

            case T3GUI_EVENT_CLOSE:
               focus = event.user.data1;
               in_dialog = false;
               break;

            case T3GUI_EVENT_CANCEL:
               focus = -1;
               in_dialog = false;
               break;

         }
      }
   }

   t3gui_stop_dialog(player);
   al_destroy_timer(timer);

   al_destroy_event_queue(menu_queue);
   t3gui_shutdown_dialog(player);
   return focus;
}



bool t3gui_listen_for_events(T3GUI_PLAYER *player, ALLEGRO_EVENT_SOURCE *src)
{
   if (!player) return false;

   al_register_event_source(player->input, src);
   return true;
}

T3GUI_ELEMENT *t3gui_find_widget_id(T3GUI_ELEMENT *dialog, uint32_t id)
{
   assert(dialog);
   int n;

   for (n = 0; dialog[n].proc; n++)
      if (dialog[n].id == id) return dialog+n;

   return NULL;
}


void t3gui_get_dialog_bounding_box(T3GUI_ELEMENT *dialog, int *min_x, int *min_y, int *max_x, int *max_y)
{
   int c;
   assert(dialog);
   assert(min_x);
   assert(min_y);
   assert(max_x);
   assert(max_x);

   *min_x = INT_MAX;
   *min_y = INT_MAX;
   *max_x = INT_MIN;
   *max_y = INT_MIN;

   for (c=0; dialog[c].proc; c++) {
      if (dialog[c].x < *min_x)
         *min_x = dialog[c].x;

      if (dialog[c].y < *min_y)
         *min_y = dialog[c].y;

      if (dialog[c].x + dialog[c].w > *max_x)
         *max_x = dialog[c].x + dialog[c].w;

      if (dialog[c].y + dialog[c].h > *max_y)
         *max_y = dialog[c].y + dialog[c].h;
   }

}



/* centre_dialog:
 *  Moves all the objects in a dialog so that the dialog is centered in
 *  the screen.
 */
void t3gui_centre_dialog(T3GUI_ELEMENT *dialog, int w, int h)
{
   int min_x = INT_MAX;
   int min_y = INT_MAX;
   int max_x = INT_MIN;
   int max_y = INT_MIN;
   int xc, yc;
   int c;
   assert(dialog);

   t3gui_get_dialog_bounding_box(dialog, &min_x, &min_y, &max_x, &max_y);

   int screen_w = w;
   int screen_h = h;

   /* how much to move by? */
   xc = (screen_w - (max_x - min_x)) / 2 - min_x;
   yc = (screen_h - (max_y - min_y)) / 2 - min_y;

   /* move it */
   for (c=0; dialog[c].proc; c++) {
      dialog[c].x += xc;
      dialog[c].y += yc;
   }
}


/* position_dialog:
 *  Moves all the objects in a dialog to the specified position.
 */
void t3gui_position_dialog(T3GUI_ELEMENT *dialog, int x, int y)
{
   int min_x = INT_MAX;
   int min_y = INT_MAX;
   int xc, yc;
   int c;
   assert(dialog);

   /* locate the upper-left corner */
   for (c=0; dialog[c].proc; c++) {
      if (dialog[c].x < min_x)
         min_x = dialog[c].x;

      if (dialog[c].y < min_y)
         min_y = dialog[c].y;
   }

   /* move the dialog */
   xc = min_x - x;
   yc = min_y - y;

   for (c=0; dialog[c].proc; c++) {
      dialog[c].x -= xc;
      dialog[c].y -= yc;
   }
}


uint32_t t3gui_index_to_id(T3GUI_ELEMENT *dialog, int index)
{
   assert(dialog);
   if (index < 0) return T3GUI_NO_ID;

   /* Make sure we are in range */
   int n = 0;
   while (dialog[n].proc) n++;

   if (index > n) return T3GUI_NO_ID;

   return dialog[index].id;
}

int t3gui_id_to_index(T3GUI_ELEMENT *dialog, uint32_t id)
{
   assert(dialog);
   if (id == T3GUI_NO_ID) return -1;

   int index = 0;
   int count = 0;
   int n = 0;
   while (dialog[n].proc) {
      if (dialog[n].id == id) {
         count++;
         index = n;
      }
      n++;
   }

   if (count == 1) return index;

   return T3GUI_NO_ID;
}

bool t3gui_id_is_unique(T3GUI_ELEMENT *dialog, uint32_t id)
{
   if (id == T3GUI_NO_ID) return false;

   int count = 0;
   int n = 0;
   while (dialog[n].proc) {
      if (dialog[n].id == id)
         count++;
      n++;
   }

   if (count == 1) return true;

   return false;
}

uint32_t get_unique_id(T3GUI_ELEMENT *dialog)
{
   uint32_t id = T3GUI_NO_ID+1;

   while (!t3gui_id_is_unique(dialog, id)) id++;

   return id;
}
