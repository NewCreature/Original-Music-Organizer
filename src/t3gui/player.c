#include <allegro5/allegro5.h>
#include "defines.h"
#include "unicode.h"
#include "dialog.h"
#include "element.h"
#include "theme.h"
#include "player.h"
#include "resource.h"
#include "t3gui.h"

static bool init = false;

static ALLEGRO_EVENT_SOURCE dialog_controller;

static void t3gui_event_destructor(ALLEGRO_USER_EVENT *event)
{
   return;
}

static inline void MESSAGE(T3GUI_PLAYER *player, int i, int msg, int c)
{
   if (i < 0) return;

   int r = t3gui_object_message(player->dialog+i, msg, c);
   if (r != D_O_K) {
      player->res |= r;
      player->obj = i;
   }
}

#define MAX_OBJECTS     512

typedef struct OBJ_LIST
{
   int index;
   int diff;
} OBJ_LIST;


/* Weight ratio between the orthogonal direction and the main direction
   when calculating the distance for the focus algorithm. */
#define DISTANCE_RATIO  8

/* Maximum size (in bytes) of a dialog array. */
#define MAX_SIZE  0x10000  /* 64 kb */

enum axis { X_AXIS, Y_AXIS };



/* obj_list_cmp:
 *  Callback function for qsort().
 */
static int obj_list_cmp(const void *e1, const void *e2)
{
   return (((OBJ_LIST *)e1)->diff - ((OBJ_LIST *)e2)->diff);
}



/* cmp_tab:
 *  Comparison function for tab key movement.
 */
static int cmp_tab(const T3GUI_ELEMENT *d1, const T3GUI_ELEMENT *d2)
{
   int ret = (int)((const unsigned long)d2 - (const unsigned long)d1);

   /* Wrap around if d2 is before d1 in the dialog array. */
   if (ret < 0)
      ret += MAX_SIZE;

   return ret;
}



/* cmp_shift_tab:
 *  Comparison function for shift+tab key movement.
 */
static int cmp_shift_tab(const T3GUI_ELEMENT *d1, const T3GUI_ELEMENT *d2)
{
   int ret = (int)((const unsigned long)d1 - (const unsigned long)d2);

   /* Wrap around if d2 is after d1 in the dialog array. */
   if (ret < 0)
      ret += MAX_SIZE;

   return ret;
}


/* offer_focus:
 *  Offers the input focus to a particular object.
 */
static int offer_focus(T3GUI_ELEMENT *dialog, int obj, int *focus_obj, int force)
{
    int res = D_O_K;
    assert(dialog);
    assert(focus_obj);

    if((obj == *focus_obj) || ((obj >= 0) && (dialog[obj].flags & (D_HIDDEN | D_DISABLED))))
    {
        return D_O_K;
    }

    /* check if object wants the focus */
    if(obj >= 0)
    {
        res = t3gui_object_message(dialog+obj, MSG_WANTFOCUS, 0);
        if(res & D_WANTKEYBOARD)
        {
            res ^= D_WANTKEYBOARD;
        }
        else
        {
            obj = -1;
        }
    }

    if((obj >= 0) || (force))
    {
        /* take focus away from old object */
        if(*focus_obj >= 0)
        {
            res |= t3gui_object_message(dialog+*focus_obj, MSG_LOSTFOCUS, 0);
            if(res & D_WANTKEYBOARD)
            {
                if(obj < 0)
                {
                    return D_O_K;
                }
                else
                {
                    res &= ~D_WANTKEYBOARD;
                }
            }
            dialog[*focus_obj].flags &= ~D_GOTFOCUS;
            res |= D_REDRAW_ALL;
        }

        *focus_obj = obj;

        /* give focus to new object */
        if(obj >= 0)
        {
            dialog[obj].flags |= D_GOTFOCUS;
            res |= t3gui_object_message(dialog+obj, MSG_GOTFOCUS, 0);
            res |= D_REDRAW_ALL;
        }
    }

    return res;
}

/* move_focus:
 *  Handles arrow key and tab movement through a dialog, deciding which
 *  object should be given the input focus.
 */
static int move_focus(T3GUI_ELEMENT *d, int keycode, bool shift, int *focus_obj)
{
   int (*cmp)(const T3GUI_ELEMENT *d1, const T3GUI_ELEMENT *d2);
   OBJ_LIST obj[MAX_OBJECTS];
   int obj_count = 0;
   int fobj, c;
   int res = D_O_K;

   /* choose a comparison function */
   switch (keycode) {
      case ALLEGRO_KEY_TAB:   cmp = (shift == false) ? cmp_tab : cmp_shift_tab; break;
/*      case ALLEGRO_KEY_RIGHT: cmp = cmp_right; break;
      case ALLEGRO_KEY_LEFT:  cmp = cmp_left;  break;
      case ALLEGRO_KEY_DOWN:  cmp = cmp_down;  break;
      case ALLEGRO_KEY_UP:    cmp = cmp_up;    break; */
      default:                return D_O_K;
   }

   /* fill temporary table */
   for (c=0; d[c].proc; c++) {
      if (((*focus_obj < 0) || (c != *focus_obj))
            && !(d[c].flags & (D_DISABLED | D_HIDDEN | D_NOFOCUS))) {
         obj[obj_count].index = c;
         if (*focus_obj >= 0)
            obj[obj_count].diff = cmp(d+*focus_obj, d+c);
         else
            obj[obj_count].diff = c;
         obj_count++;
         if (obj_count >= MAX_OBJECTS)
            break;
      }
   }

   /* sort table */
   qsort(obj, obj_count, sizeof(OBJ_LIST), obj_list_cmp);

   /* find an object to give the focus to */
   fobj = *focus_obj;
   for (c=0; c<obj_count; c++) {
      res |= offer_focus(d, obj[c].index, focus_obj, false);
      if (fobj != *focus_obj)
         break;
   }

   return res;
}

void t3gui_set_player_focus(T3GUI_PLAYER * player, int obj)
{
    offer_focus(player->dialog, obj, &player->keyboard_obj, 1);
}

static void pass_through_event(T3GUI_PLAYER *player, ALLEGRO_EVENT *event)
{
    if(player->pass_events)
    {
        al_emit_user_event(&player->event_src, event, NULL);
    }
}

static void dialog_thread_internal_event_handler(T3GUI_PLAYER * player, ALLEGRO_EVENT * event)
{
    switch (event->type)
    {
        case EGGC_STOP:
        {
            if(player == (T3GUI_PLAYER *)event->user.data4)
            {
                player->running = false;
                player->no_close_callback = true;
            }
            break;
        }

        case EGGC_PAUSE:
        {
            if(player == (T3GUI_PLAYER *)event->user.data4)
            {
                player->paused = true;
            }
            break;
        }

        case EGGC_RESUME:
        {
            if(player == (T3GUI_PLAYER *)event->user.data4)
            {
                player->paused = false;
            }
            break;
        }
    }
}

static void dialog_thread_event_handler(T3GUI_PLAYER * player, ALLEGRO_EVENT * event)
{
    ALLEGRO_EVENT my_event;
    my_event.user.data4 = (intptr_t)player;
    int old_mouse_x = -1;
    int old_mouse_y = -1;
    switch (event->type)
    {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
        {
            if(!(player->flags & T3GUI_PLAYER_IGNORE_CLOSE))
            {
                player->res |= D_CLOSE;
            }
            break;
        }

        case ALLEGRO_EVENT_DISPLAY_RESIZE:
        case ALLEGRO_EVENT_DISPLAY_EXPOSE:
        {
            player->redraw = true;
            my_event.user.type = T3GUI_EVENT_REDRAW;
            al_emit_user_event(&player->event_src, &my_event, t3gui_event_destructor);
            break;
        }

        case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
        {
            t3gui_unload_resources(event->display.source, false);
            al_acknowledge_drawing_halt(event->display.source);
            player->draw_veto = true;
            break;
        }

        case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
        {
            al_acknowledge_drawing_resume(event->display.source);
            t3gui_reload_resources(event->display.source);
            player->draw_veto = false;
            player->redraw = true;
            my_event.user.type = T3GUI_EVENT_REDRAW;
            al_emit_user_event(&player->event_src, &my_event, t3gui_event_destructor);
            break;
        }
        case ALLEGRO_EVENT_DISPLAY_LOST:
        {
            t3gui_unload_resources(event->display.source, false);
            break;
        }
        case ALLEGRO_EVENT_DISPLAY_FOUND:
        {
            t3gui_reload_resources(event->display.source);
            break;
        }

        case ALLEGRO_EVENT_TIMER:
        {
            if(event->timer.source == player->dclick_timer)
            { /* Double click? */
                al_stop_timer(player->dclick_timer);

                if(player->click_obj >= 0)
                {
                    MESSAGE(player, player->click_obj, (player->click_count > 1) ? MSG_DCLICK : MSG_CLICK, player->click_button);
                    player->click_button = -1;
                }

                player->click_count = 0;
            }
            else if(event->timer.source == player->joy_timer)
            {
                al_stop_timer(player->joy_timer);

                if(player->joy_x)
                {
                    if (player->joy_x > 0)
                    {
                        player->res |= move_focus(player->dialog, ALLEGRO_KEY_RIGHT, 0, &player->keyboard_obj);
                    }
                    else
                    {
                        player->res |= move_focus(player->dialog, ALLEGRO_KEY_LEFT, 0, &player->keyboard_obj);
                    }
                }

                if(player->joy_y)
                {
                    if(player->joy_y > 0)
                    {
                        player->res |= move_focus(player->dialog, ALLEGRO_KEY_UP, 0, &player->keyboard_obj);
                    }
                    else
                    {
                        player->res |= move_focus(player->dialog, ALLEGRO_KEY_DOWN, 0, &player->keyboard_obj);
                    }
                }

                if(player->joy_b)
                {
                    if (player->keyboard_obj >= 0)
                    {
                        MESSAGE(player, player->keyboard_obj, MSG_KEY, 0);
                    }
                }

                player->joy_x = player->joy_y = player->joy_b = 0;
            }
            else
            {
                player->res |= t3gui_dialog_message(player->dialog, MSG_TIMER, event->timer.count, &player->obj);
            }
            break;
        }

        case ALLEGRO_EVENT_KEY_DOWN:
        {
            player->key[event->keyboard.keycode] = true;
            switch(event->keyboard.keycode)
            {
                case ALLEGRO_KEY_LSHIFT:
                case ALLEGRO_KEY_RSHIFT:
                {
                    player->shift = true;
                    break;
                }
            }
            if(player->keyboard_obj >= 0)
            {
                MESSAGE(player, player->keyboard_obj, MSG_KEYDOWN, event->keyboard.keycode);
            }
            if(!(player->res & D_USED_KEY))
            {
                /* Pass-through? */
                switch(event->keyboard.keycode)
                {
                    case ALLEGRO_KEY_ESCAPE:   /* Escape closes the dialog */
                    {
                        player->escape_down = true;
                        break;
                    }
                    case ALLEGRO_KEY_ENTER:    /* pass <CR> or <SPACE> to selected object */
                    case ALLEGRO_KEY_SPACE:
                    case ALLEGRO_KEY_PAD_ENTER:
                    case ALLEGRO_KEY_TAB:      /* Move focus */
                    case ALLEGRO_KEY_RIGHT:
                    case ALLEGRO_KEY_LEFT:
                    case ALLEGRO_KEY_DOWN:
                    case ALLEGRO_KEY_UP:
                    {
                        break;
                    }

                    default:
                    {
                        pass_through_event(player, event);
                    }
                }
            }
            player->res &= ~D_USED_KEY;
            break;
        }

        case ALLEGRO_EVENT_KEY_UP:
        {
            player->key[event->keyboard.keycode] = false;
            switch(event->keyboard.keycode)
            {
                case ALLEGRO_KEY_LSHIFT:   /* Shift released */
                case ALLEGRO_KEY_RSHIFT:
                {
                    player->shift = false;
                    break;
                }
            }

            /* Check if focus object wants these keys */
            if(player->keyboard_obj >= 0)
            {
                MESSAGE(player, player->keyboard_obj, MSG_KEYUP, event->keyboard.keycode);
            }

            /* If not, check hot keys */

            /* If not, check whether any other object wants it */

            /* No one else wants it, so it's ours */
            if(~player->res & D_USED_KEY)
            {
                switch(event->keyboard.keycode)
                {
                    case ALLEGRO_KEY_ESCAPE:   /* Escape closes the dialog */
                    {
                        if(!(player->flags & T3GUI_PLAYER_NO_ESCAPE) && player->escape_down)
                        {
                            player->obj = -1;
                            player->res |= D_CLOSE;
                        }
                        break;
                    }

                    case ALLEGRO_KEY_ENTER:    /* pass <CR> or <SPACE> to selected object */
                    case ALLEGRO_KEY_SPACE:
                    case ALLEGRO_KEY_PAD_ENTER:
                    {
                        if(player->keyboard_obj >= 0)
                        {
                            MESSAGE(player, player->keyboard_obj, MSG_KEY, 0);
                        }
                        break;
                    }

                    case ALLEGRO_KEY_TAB:      /* Move focus */
                    case ALLEGRO_KEY_RIGHT:
                    case ALLEGRO_KEY_LEFT:
                    case ALLEGRO_KEY_DOWN:
                    case ALLEGRO_KEY_UP:
                    {
                        player->keyboard_obj = t3gui_find_dialog_focus(player->dialog);
                        player->res |= move_focus(player->dialog, event->keyboard.keycode, player->shift, &player->keyboard_obj);
                        break;
                    }

                    default:
                    {
                        /* Pass-through */
                        pass_through_event(player, event);
                    }
                }
            }
            player->res &= ~D_USED_KEY;
            break;
        }

        case ALLEGRO_EVENT_KEY_CHAR:
        {
            if(!(event->keyboard.modifiers & (ALLEGRO_KEYMOD_CTRL | ALLEGRO_KEYMOD_COMMAND)))
            {
                if(player->keyboard_obj >= 0)
                {
                    MESSAGE(player, player->keyboard_obj, MSG_CHAR, event->keyboard.unichar);
                    if(event->keyboard.repeat)
                    {
                        MESSAGE(player, player->keyboard_obj, MSG_KEYREPEAT, event->keyboard.keycode);
                    }
                }
                if(!(player->res & D_USED_CHAR))
                {
                    /* Keyboard short-cut? */
                    int keychar = event->keyboard.unichar;
                    int c;
                    if (event->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) keychar = 8;
                    if (event->keyboard.keycode == ALLEGRO_KEY_DELETE) keychar = 127;
                    //printf("%d %d %d %d\n", event.keyboard.unichar, event.keyboard.keycode, ALLEGRO_KEY_DELETE, ALLEGRO_KEY_BACKSPACE);
                    if (keychar)
                    {
                        for (c=0; player->dialog[c].proc; c++)
                        {
                            if (utolower(player->dialog[c].key) == utolower(player->dialog[c].key) && player->dialog[c].key == keychar && (!(player->dialog[c].flags & (D_HIDDEN | D_DISABLED))))
                            {
                                MESSAGE(player, c, MSG_KEY, 0);
                                break;
                            }
                        }
                    }
                }
                if(!(player->res & D_USED_CHAR))
                {
                    /* Pass-through? */
                    switch (event->keyboard.keycode)
                    {
                        case ALLEGRO_KEY_ESCAPE:   /* Escape closes the dialog */
                        case ALLEGRO_KEY_ENTER:    /* pass <CR> or <SPACE> to selected object */
                        case ALLEGRO_KEY_SPACE:
                        case ALLEGRO_KEY_PAD_ENTER:
                        case ALLEGRO_KEY_TAB:      /* Move focus */
                        case ALLEGRO_KEY_RIGHT:
                        case ALLEGRO_KEY_LEFT:
                        case ALLEGRO_KEY_DOWN:
                        case ALLEGRO_KEY_UP:
                        {
                            break;
                        }

                        default:
                        {
                            pass_through_event(player, event);
                        }
                    }
                }
                player->res &= ~D_USED_KEY;
            }
            break;
        }

        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
        {
          player->mouse_x = event->mouse.x;
          player->mouse_y = event->mouse.y;
          player->mouse_z = event->mouse.z;
          if(event->mouse.button - 1 < 32)
          {
            player->mouse_button[event->mouse.button] = 1;
          }
            /* Reset number of mouse clicks if mouse focus has changed */
            if (player->click_obj != player->mouse_obj)
            {
                player->click_count = 0;
            }

            player->click_obj = player->mouse_obj;
            if (player->click_obj >= 0)
            {
                MESSAGE(player, player->mouse_obj, MSG_MOUSEDOWN, event->mouse.button);
                if(player->res &= D_WANTKEYBOARD)
                {
                    if(offer_focus(player->dialog, player->obj, &player->keyboard_obj, false) & D_WANTKEYBOARD)
                    {
                        if(player->keyboard_obj >= 0)
                        {
                            player->dialog[player->keyboard_obj].flags &= ~D_GOTFOCUS;
                        }
                        player->dialog[player->obj].flags |= D_GOTFOCUS;
                    }
                }
            }
            break;
        }

        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        {
          player->mouse_x = event->mouse.x;
          player->mouse_y = event->mouse.y;
          player->mouse_z = event->mouse.z;
          if(event->mouse.button - 1 < 32)
          {
            player->mouse_button[event->mouse.button] = 0;
          }
            if (player->click_obj >= 0)
            {
                if(player->dialog[player->click_obj].flags & D_TRACKMOUSE)
                {
                    MESSAGE(player, player->click_obj, MSG_MOUSEUP, event->mouse.button);
                }
                else
                {
                    MESSAGE(player, player->mouse_obj, MSG_MOUSEUP, event->mouse.button);
                }
            }

            if (player->click_obj == player->mouse_obj)
            {
                al_start_timer(player->dclick_timer);
                if(player->click_button != event->mouse.button)
                {
                    player->click_button = event->mouse.button;
                    player->click_count = 1;
                }
                else
                {
                    player->click_count++;
                }
            }
            else
            {
                player->click_count = 0;
                player->click_obj = -1;
            }
            break;
        }

        case ALLEGRO_EVENT_MOUSE_AXES:
        {
          int n;
          bool track = false;
          player->mouse_x = event->mouse.x;
          player->mouse_y = event->mouse.y;
          player->mouse_z = event->mouse.z;

          /* Are objects tracking the mouse? */
          if(event->mouse.x != old_mouse_x || event->mouse.y != old_mouse_y)
          {
              for (n=0; player->dialog[n].proc; n++)
              {
                  player->dialog[n].mousex = event->mouse.x;
                  player->dialog[n].mousey = event->mouse.y;
                  if (player->dialog[n].flags & D_TRACKMOUSE)
                  {
                      player->res |= t3gui_object_message(player->dialog+n, MSG_MOUSEMOVE, 0);
                      track = true;
                  }
              }
          }
            /* has mouse object changed? */
            if(!track)
            {
              int mouse_obj = t3gui_find_mouse_object(player->dialog, event->mouse.x, event->mouse.y);
              if (mouse_obj != player->mouse_obj)
              {
                  if (player->mouse_obj >= 0)
                  {
                      player->dialog[player->mouse_obj].flags &= ~D_GOTMOUSE;
                      MESSAGE(player, player->mouse_obj, MSG_LOSTMOUSE, 0);
                  }
                  if (mouse_obj >= 0)
                  {
                      player->dialog[mouse_obj].flags |= D_GOTMOUSE;
                      MESSAGE(player, mouse_obj, MSG_GOTMOUSE, 0);
                  }
                  player->mouse_obj = mouse_obj;

                  /* move the input focus as well? */
                  if ((player->focus_follows_mouse) && (player->mouse_obj != player->keyboard_obj))
                  {
                      player->res |= offer_focus(player->dialog, player->mouse_obj, &player->mouse_obj, true);
                  }
              }
            }

            old_mouse_x = event->mouse.x;
            old_mouse_y = event->mouse.y;
            if (event->mouse.dz)
            {
                MESSAGE(player, player->mouse_obj, MSG_WHEEL, event->mouse.dz);
            }
            if (event->mouse.dw)
            {
                MESSAGE(player, player->mouse_obj, MSG_HBALL, event->mouse.dw);
            }
            break;
        }

        case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
        {
            al_reconfigure_joysticks();
            break;
        }

        case ALLEGRO_EVENT_JOYSTICK_AXIS:
        {
            if (event->joystick.stick == player->stick)
            {
                if (event->joystick.axis == player->x_axis)
                {
                    if (event->joystick.pos >  0.95) player->joy_x =  1;
                    if (event->joystick.pos < -0.95) player->joy_x = -1;
                }
                if (event->joystick.axis == player->y_axis)
                {
                    if (event->joystick.pos >  0.95) player->joy_y = -1;
                    if (event->joystick.pos < -0.95) player->joy_y =  1;
                }
                al_start_timer(player->joy_timer);
            }
            break;
        }

        case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
        {
            break;
        }

        case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
        {
            player->joy_b = 1;
            al_start_timer(player->joy_timer);
            break;
        }
    }
}

static void update_dialog(T3GUI_PLAYER * player)
{
    ALLEGRO_EVENT my_event;
    int n;
    int c = 0;
    my_event.user.data4 = (intptr_t)player;

    /* Check for objects that need to be redrawn */
    for(n = 0; player->dialog[n].proc; n++)
    {
        if(player->dialog[n].flags & D_DIRTY)
        {
            player->dialog[n].flags &= ~D_DIRTY;
            player->res |= D_REDRAW_ALL;
        }
    }

    /* remove focus from disabled items */
    for(n = 0; player->dialog[n].proc; n++)
    {
        if(player->dialog[n].flags & D_GOTFOCUS)
        {
            if(player->dialog[n].flags & D_DISABLED)
            {
                player->dialog[n].flags &= ~D_GOTFOCUS;
            }
            else
            {
                c++;
            }
        }
    }
    if(c <= 0)
    {
        player->keyboard_obj = -1;
    }

    /* remove focus from disabled items */
    if(player->dialog[player->mouse_obj].flags & D_DISABLED)
    {
        player->dialog[player->mouse_obj].flags &= ~D_GOTMOUSE;
        player->mouse_obj = -1;
    }

    if(player->res & D_REDRAW_ANY)
    {
        player->redraw = true;
    }

    if(player->res & D_CLOSE)
    {
        my_event.user.type = T3GUI_EVENT_CLOSE;
        if (player->obj == -1)
        {
            my_event.user.type = T3GUI_EVENT_CANCEL;
        }
        my_event.user.data1 = player->obj;
        my_event.user.data2 = 0;
        if(player->obj >= 0)
        {
            my_event.user.data2 = (intptr_t)&player->dialog[player->obj];
        }
        al_emit_user_event(&player->event_src, &my_event, t3gui_event_destructor);
        if(player->user_queue)
        {
            al_emit_user_event(t3gui_get_event_source(), &my_event, t3gui_event_destructor);
        }
        player->delete = true;
    }
    else
    {
      /* Clear some status flags that should not be retained */
      player->res &= ~D_USED_CHAR;
    }
}

/* TODO: secondary click, mouse wheel scrolling.
 * Keep track of key shifts.
 */
static void *dialog_thread_func(ALLEGRO_THREAD *thread, void *arg)
{
    T3GUI_PLAYER *player = arg;

    while(player->running)
    {
        ALLEGRO_EVENT my_event;
        my_event.user.data4 = (intptr_t)player;
        if(player->redraw)
        {
            my_event.user.type = T3GUI_EVENT_REDRAW;
            al_emit_user_event(&player->event_src, &my_event, t3gui_event_destructor);
        }

        ALLEGRO_EVENT event;
        bool received_event = al_wait_for_event_timed(player->input, &event, 0.02);
        if(received_event)
        {
            dialog_thread_internal_event_handler(player, &event);
            if(!player->paused)
            {
                dialog_thread_event_handler(player, &event);
            }
            else
            { /* Send idle messages */
                player->res |= t3gui_dialog_message(player->dialog, MSG_IDLE, 0, &player->obj);
            }
        }
        update_dialog(player);
    }

    return NULL;
}

T3GUI_PLAYER *t3gui_init_dialog(T3GUI_ELEMENT *dialog, int focus_obj, int flags, ALLEGRO_EVENT_QUEUE * qp, void * user_data, void (*close_proc)(T3GUI_ELEMENT * ep))
{
    T3GUI_PLAYER *player = calloc(1, sizeof *player);
    int c;

    /* ensure we initialized the dialog controller */
    if(!init)
    {
        al_init_user_event_source(&dialog_controller);
        init = true;
    }

    if(!player)
    {
        return NULL;
    }
    player->dialog = dialog;

    /* Initialise event channels */
    player->input = al_create_event_queue();
    al_init_user_event_source(&player->event_src);
    if(qp)
    {
        player->user_queue = true;
    }
    else
    {
        player->user_queue = false;
    }

    /* Initialise focus and other bookkeeping */
    player->pass_events = true;
    player->focus_follows_mouse = true;
    player->obj = -1;
    player->mouse_obj = -1;
    player->click_obj = -1;
    player->escape_down = false;

    player->stick = 0;
    player->x_axis = 0;
    player->y_axis = 1;
    player->joy_x = 0;
    player->joy_y = 0;
    player->joy_b = 0;
    player->user_data = user_data;
    player->close_proc = close_proc;
    player->flags = flags;

    /* Set display for all dialog items */
    for (c=0; dialog[c].proc; c++)
        dialog[c].display = al_get_current_display();

    /* Set root widget for all dialog items */
    for (c=0; dialog[c].proc; c++)
        dialog[c].root = dialog;

    /* Notify all objects that the dialog is starting */
    player->res |= t3gui_dialog_message(dialog, MSG_START, 0, &player->obj);
    player->redraw = true;

    /* Clear focus flag from all objects */
    for (c=0; dialog[c].proc; c++)
        dialog[c].flags &= ~(D_GOTFOCUS | D_GOTMOUSE | D_TRACKMOUSE | D_INTERNAL);

    /* Find object that has the mouse */
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    player->mouse_obj = t3gui_find_mouse_object(dialog, mouse_state.x, mouse_state.y);
    if (player->mouse_obj >= 0)
        dialog[player->mouse_obj].flags |= D_GOTMOUSE;

    /* If no focus object has been specified it defaults to the mouse object */
    if (focus_obj < 0) focus_obj = player->mouse_obj;
    player->keyboard_obj = focus_obj;

    /* Offer focus to the focus object */
    if ((focus_obj >= 0) && ((t3gui_object_message(dialog+focus_obj, MSG_WANTFOCUS, 0)) & D_WANTKEYBOARD))
    {
        dialog[focus_obj].flags |= (D_GOTFOCUS);
    }

    /* Set default "grey-out" colour */
    for (c=0; dialog[c].proc; c++)
    {
//        ALLEGRO_COLOR grey = t3gui_silver;
//      dialog[c].mg = grey;
    }
    player->paused = false;

    return player;
}

void t3gui_shutdown_dialog(T3GUI_PLAYER *player)
{
   if (!player) return;

   if (player->thread) t3gui_stop_dialog_thread(player);

   player->res |= t3gui_dialog_message(player->dialog, MSG_END, 0, &player->obj);

   al_destroy_user_event_source(&player->event_src);
   al_destroy_event_queue(player->input);
   free(player);
}

ALLEGRO_EVENT_SOURCE *t3gui_get_player_event_source(T3GUI_PLAYER *player)
{
   if (!player) return NULL;

   return &player->event_src;
}

bool t3gui_start_dialog(T3GUI_PLAYER * player)
{
    /* Special timer to keep track of double clicks:
     * When a click is detected the timer is started and the clicked object
     * is recorded. Once the timer triggers it will check how many mouse
     * clicks there have been since it was started and then send a CLICK or
     * DCLICK message.
     * The timer disables itself immediately after it triggered.
     */
     player->dclick_timer = al_create_timer(0.3);
     al_register_event_source(player->input, al_get_timer_event_source(player->dclick_timer));

     /* Special timer for joystick events, which need to be on a delay. */
     player->joy_timer = al_create_timer(0.3);
     al_register_event_source(player->input, al_get_timer_event_source(player->joy_timer));

     player->cursor_timer = al_create_timer(0.25);
     al_register_event_source(player->input, al_get_timer_event_source(player->cursor_timer));
     al_start_timer(player->cursor_timer);
     player->display = al_get_current_display();

     al_register_event_source(player->input, &dialog_controller);

     player->running = true;
     player->no_close_callback = false;
     player->shift = false;           /* Keep track of shift state, for passing around widget focus */
     player->click_count = 0;
     player->redraw = true;
     player->threaded = false;

     return true;
}

/* start_dialog:
 * begin processing input and generating events from the selected dialog.
 */
bool t3gui_start_dialog_thread(T3GUI_PLAYER *player)
{
   if (player->thread) return true; /* Already started */

   player->thread = al_create_thread(dialog_thread_func, (void *)player);
   if (!player->thread) return false;

   /* Listen to the controller */
   al_register_event_source(player->input, &dialog_controller);

   t3gui_start_dialog(player);
   player->threaded = true;
   al_start_thread(player->thread);
   return true;
}

bool t3gui_stop_dialog(T3GUI_PLAYER * player)
{
    al_unregister_event_source(player->input, al_get_timer_event_source(player->dclick_timer));
    al_unregister_event_source(player->input, al_get_timer_event_source(player->joy_timer));
    al_destroy_timer(player->joy_timer);
    al_destroy_timer(player->dclick_timer);
    if(player->threaded && !player->no_close_callback && player->close_proc)
    {
        player->close_proc(player->dialog);
    }
    return true;
}


/* stop_dialog:
 * stop processing input and generating events from the selected dialog.
 */
bool t3gui_stop_dialog_thread(T3GUI_PLAYER *player)
{
    assert(player);
    assert(player->thread);

    t3gui_stop_dialog(player);
    al_set_thread_should_stop(player->thread);
    ALLEGRO_EVENT event;
    event.user.data4 = (intptr_t)player;
    event.user.type = EGGC_STOP;
    al_emit_user_event(&dialog_controller, &event, NULL);
    al_join_thread(player->thread, NULL);
    al_destroy_thread(player->thread);
    al_unregister_event_source(player->input, &dialog_controller);
    player->thread = NULL;

    return true;
}

/* pause_dialog:
 * stop processing input and generating events from the selected dialog.
 */
bool t3gui_pause_dialog(T3GUI_PLAYER *player)
{
   assert(player);
//   assert(player->thread);

   ALLEGRO_EVENT event;
   event.user.data4 = (intptr_t)player;
   event.user.type = EGGC_PAUSE;
   al_emit_user_event(&dialog_controller, &event, NULL);
   t3gui_process_dialog(player);

   return true;
}

/* resume_dialog:
 * stop processing input and generating events from the selected dialog.
 */
bool t3gui_resume_dialog(T3GUI_PLAYER *player)
{
   assert(player);
//   assert(player->thread);

   ALLEGRO_EVENT event;
   event.user.data4 = (intptr_t)player;
   event.user.type = EGGC_RESUME;
   al_emit_user_event(&dialog_controller, &event, NULL);

   return true;
}

T3GUI_ELEMENT * t3gui_get_player_click_element(T3GUI_PLAYER * player)
{
    if(player->click_obj >= 0)
    {
        return &player->dialog[player->click_obj];
    }
    return NULL;
}

void t3gui_process_dialog(T3GUI_PLAYER * player)
{
    ALLEGRO_EVENT event;

    update_dialog(player);
    t3gui_dialog_message(player->dialog, MSG_IDLE, 0, &player->obj);
    while(al_get_next_event(player->input, &event))
	{
        dialog_thread_internal_event_handler(player, &event);
        if(!player->paused)
        {
            dialog_thread_event_handler(player, &event);
        }
        update_dialog(player);
        if(player->delete)
        {
          t3gui_close_dialog_by_element(player->dialog);
          break;
        }
    }
}

bool t3gui_draw_dialog(T3GUI_PLAYER *player)
{
    ALLEGRO_STATE old_state;
    ALLEGRO_TRANSFORM identity;
    int n;
   if (player->threaded && !player->thread) return false;
   if (player->draw_veto) return false;

   for (n=0; player->dialog[n].proc; n++)
      player->dialog[n].flags &= ~D_DIRTY;


   int clip_x, clip_y, clip_w, clip_h;

   //printf("Post redraw message\n");
   player->redraw = false;
   al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
   al_set_target_bitmap(al_get_backbuffer(player->display));
   al_get_clipping_rectangle(&clip_x, &clip_y, &clip_w, &clip_h);
   al_identity_transform(&identity);
   al_use_transform(&identity);
   if(player->flags & T3GUI_PLAYER_CLEAR)
   {
       al_clear_to_color(al_map_rgb(0, 0, 0));
   }
   player->res |= t3gui_dialog_message(player->dialog, MSG_DRAW, 0, &player->obj);
   al_set_clipping_rectangle(clip_x, clip_y, clip_w, clip_h);
   al_restore_state(&old_state);
   player->res &= ~D_REDRAW_ANY;


   return true;
}
