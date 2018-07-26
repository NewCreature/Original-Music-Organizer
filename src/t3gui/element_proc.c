#include <ctype.h>
#include "dialog.h"
#include "unicode.h"

static int min(int x, int y) { return (x<y)?x:y; }
static int max(int x, int y) { return (x>y)?x:y; }
static int clamp(int x, int y, int z) {return max(x, min(y, z));}

static void render_split_text(const ALLEGRO_FONT * fp, ALLEGRO_COLOR color, float x, float y, int max_width, int min_space, const char * text)
{
    const char * right_text = NULL;
    char full_text[1024];
    int text_width;
    int cx, cy, cw, ch;
    int i;

    al_get_clipping_rectangle(&cx, &cy, &cw, &ch);

    /* split the text */
    strcpy(full_text, text);
    for(i = 0; i < strlen(full_text); i++)
    {
        if(full_text[i] == '\t')
        {
            right_text = &full_text[i + 1];
            full_text[i] = '\0';
            break;
        }
    }

    if(right_text)
    {
        text_width = max_width - al_get_text_width(fp, right_text) - min_space;
        al_set_clipping_rectangle(x, cy, text_width, ch);
    }
    al_draw_text(fp, color, x, y, 0, full_text);
    if(right_text)
    {
        text_width = al_get_text_width(fp, right_text);
        al_set_clipping_rectangle(x + max_width - text_width, cy, text_width, ch);
        al_draw_text(fp, color, x + max_width, y, ALLEGRO_ALIGN_RIGHT, right_text);
    }
    al_set_clipping_rectangle(cx, cy, cw, ch);
}

void initialise_vertical_scroll(const T3GUI_ELEMENT *parent, T3GUI_ELEMENT *scroll, int w)
{
   scroll->proc   = t3gui_scroll_proc;

   scroll->x      = parent->x + parent->w - w;
   scroll->y      = parent->y;
   scroll->w      = w;
   scroll->h      = parent->h;

   scroll->theme = parent->theme;

   scroll->flags  = parent->flags;
   scroll->d1     = parent->d1 - parent->h;
   scroll->d2     = parent->d2;

   scroll->mousex = parent->mousex;
   scroll->mousey = parent->mousey;
}

void initialise_horizontal_scroll(const T3GUI_ELEMENT *parent, T3GUI_ELEMENT *scroll, int h)
{
   scroll->proc   = t3gui_scroll_proc;

   scroll->x      = parent->x;
   scroll->y      = parent->y + parent->h - h;
   scroll->w      = parent->w;
   scroll->h      = h;

   scroll->theme = parent->theme;

   scroll->flags  = parent->flags;
   scroll->d1     = parent->d1 - parent->w;
   scroll->d2     = parent->d2;

   scroll->mousex = parent->mousex;
   scroll->mousey = parent->mousey;
}

/*
 * Sample dialog routines/widgets
 */


int t3gui_window_frame_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   assert(d);
   int ret = D_O_K;
   const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
   const char *text = d->dp;
   int title_height = 0;
   if (text == NULL) text = "";
   title_height = al_get_font_line_height(font);

   if (msg == MSG_START) {
      int min_x, min_y, max_x, max_y;
      t3gui_get_dialog_bounding_box(d, &min_x, &min_y, &max_x, &max_y);
      d->x = min_x-4;
      d->w = max_x - min_x+8;
      d->y = min_y - 4;
      d->h = max_y - min_y+8;

      d->y -= title_height;
      d->h += title_height;
   }

   if (msg == MSG_DRAW) {
      int flags = d->d1;
      int x = d->x;

      if (flags == ALLEGRO_ALIGN_CENTRE) x += d->w/2;
      if (flags == ALLEGRO_ALIGN_RIGHT)  x += d->w;

      t3gui_box_proc(msg, d, c);

//      al_draw_filled_rectangle(d->x+0.5, d->y+0.5, d->x+d->w-0.5, d->y+title_height-0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG]);
      al_draw_text(font, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], x, d->y, d->d1, text);
   }

   if (msg == MSG_MOUSEDOWN) {
      if (d->mousey < d->y+title_height) {
         d->d2 = d->mousex;
         d->d3 = d->mousey;
         d->flags |= D_TRACKMOUSE;
      }
   }

   if (msg == MSG_MOUSEUP)
      d->flags &= ~D_TRACKMOUSE;

   if (msg == MSG_MOUSEMOVE && d->flags&D_TRACKMOUSE) {
      int dx = d->mousex - d->d2;
      int dy = d->mousey - d->d3;
      int c;
      d->d2 = d->mousex;
      d->d3 = d->mousey;
      for (c=0; d->root[c].proc; c++) {
         d->root[c].x += dx;
         d->root[c].y += dy;
      }
      ret |= D_REDRAW;
   }
   return ret;
}

int t3gui_frame_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   assert(d);
   if (d->id == T3GUI_NO_ID) return D_O_K;

   /* Find out how many children there are and how we're supposed to tile
    * them.
    */

   if (msg == MSG_START) {
      bool vertical = d->d1;
      int h_align = d->d2;    /* Left/centre/right */
      int v_align = d->d3;    /* Top/centre/bottom */
      int n;

      int child_count = 0;
      int required_space = 0;
      int available_space = vertical ? d->h : d->w;
      for (n = 0; d->root[n].proc; n++) {
         if (d->root[n].parent == d->id) {
            child_count++;
            required_space += vertical ? d->root[n].h
                                       : d->root[n].w;
         }
      }

      if (child_count == 0) return D_O_K;

      /* TODO: resize widgets if space is too small */
      if (required_space > available_space) required_space = available_space;

      int ds = (available_space - required_space) / (1 + child_count);
      int x = d->x;
      int y = d->y;

      if (vertical) {
         if (v_align != T3GUI_WALIGN_CENTRE) ds = (available_space - required_space) / child_count;
         if (v_align != T3GUI_WALIGN_LEFT)   y += ds;
         for (n = 0; d->root[n].proc; n++) {
            if (d->root[n].parent == d->id) {
               d->root[n].y = y;
               switch (h_align) {
                  case T3GUI_WALIGN_CENTRE:
                     d->root[n].x = x + (d->w - d->root[n].w)/2;
                     break;
                  case T3GUI_WALIGN_LEFT:
                     d->root[n].x = x;
                     break;
                  case T3GUI_WALIGN_RIGHT:
                     d->root[n].x = x + d->w - d->root[n].w;
                     break;
               }
               y += d->root[n].h + ds;
            }
         }
      } else {
         if (h_align != T3GUI_WALIGN_CENTRE) ds = (available_space - required_space) / child_count;
         if (h_align != T3GUI_WALIGN_LEFT)   x += ds;
         for (n = 0; d->root[n].proc; n++) {
            if (d->root[n].parent == d->id) {
               switch (v_align) {
                  case T3GUI_WALIGN_CENTRE:
                     d->root[n].y = y + (d->h - d->root[n].h)/2;
                     break;
                  case T3GUI_WALIGN_TOP:
                     d->root[n].y = y;
                     break;
                  case T3GUI_WALIGN_BOTTOM:
                     d->root[n].y = y + d->h - d->root[n].h;
                     break;
               }
               d->root[n].x = x;
               x += d->root[n].w + ds;
            }
         }
      }
   }

   return D_O_K;
}

int t3gui_box_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;
    assert(d);
    int i;

    //printf("t3gui_box received message 0x%04x\n", msg);
    if (msg==MSG_DRAW)
    {
        for(i = 0; i < 2; i++)
        {
            NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[i];
            if(p9)
            {
                int w = max(d->w, get_nine_patch_bitmap_min_width(p9));
                int h = max(d->h, get_nine_patch_bitmap_min_height(p9));
                draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG], d->x, d->y, w, h);
            }
        }
    }

    return ret;
}

/* t3gui_button_proc:
 *  A button object (the dp field points to the text string). This object
 *  can be selected by clicking on it with the mouse or by pressing its
 *  keyboard shortcut. If the D_EXIT flag is set, selecting it will close
 *  the dialog, otherwise it will toggle on and off.
 */
int t3gui_button_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;
    ALLEGRO_COLOR c1, c2;
    int g;
    assert(d);

    const char *text = d->dp;
    const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
    if (text == NULL) text = "";

    int select = D_INTERACT;
    int hover = D_GOTMOUSE | D_GOTFOCUS;

    switch(msg)
    {
        case MSG_DRAW:
        {
            if(d->flags & hover)
            {
                c1 = d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_FG];
                c2 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_BG];
            }
            else
            {
                c1 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_DISABLED].color[T3GUI_THEME_COLOR_FG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
                c2 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];
            }
            if(d->flags & select)
            {
                g = 1;
            }
            else
            {
                g = 0;
            }

            NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0];

            if (d->flags & D_GOTFOCUS && d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0])
            {
                p9 = d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0];
            }
            if (d->flags & select && d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0])
            {
                p9 = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0];
            }

            int w = max(d->w, get_nine_patch_bitmap_min_width(p9));
            int h = max(d->h, get_nine_patch_bitmap_min_height(p9));
            draw_nine_patch_bitmap(p9, c2, d->x, d->y, w, h);
            if(d->dp3)
            {
                al_draw_tinted_bitmap(d->dp3, c1, d->x + d->w / 2 - al_get_bitmap_width(d->dp3) / 2 + g, d->y + d->h / 2 - al_get_bitmap_height(d->dp3) / 2 + g, 0);
            }
            else
            {
                al_draw_text(font, c1, d->x+d->w/2+g, d->y+d->h/2-al_get_font_line_height(font)/2+g, ALLEGRO_ALIGN_CENTRE, text);
            }
            break;
        }

        case MSG_WANTFOCUS:
        {
            ALLEGRO_MOUSE_STATE mouse_state;
            al_get_mouse_state(&mouse_state);
            if(mouse_state.buttons)
            {
                d->flags |= D_INTERACT;
            }
            return ret | D_WANTKEYBOARD;
        }

        case MSG_LOSTFOCUS:
        {
            d->flags &= ~D_INTERACT;
            break;
        }

        case MSG_KEY:
        {
            /* close dialog? */
            if (d->flags & D_EXIT)
            {
                return D_CLOSE;
            }

            /* or just toggle */
            d->flags ^= D_SELECTED;
            ret |= D_REDRAWME;
            break;
        }

        case MSG_MOUSEDOWN:
        {
            if(c == 1)
            {
                d->flags |= D_INTERACT;
                ret |= D_REDRAWME;
            }
            break;
        }

        case MSG_MOUSEUP:
        {
            if(c == 1)
            {
                if (d->flags & D_TRACKMOUSE)
                {
                    ret |= D_REDRAWME;
                }
                d->flags &= ~D_INTERACT;

                ALLEGRO_MOUSE_STATE mouse_state;
                al_get_mouse_state(&mouse_state);
                int state1, state2;
                int swap;

                /* what state was the button originally in? */
                state1 = d->flags & D_SELECTED;
                if (d->flags & D_EXIT)
                {
                    swap = false;
                }
                else
                {
                    swap = state1;
                }
                state2 = ((mouse_state.x >= d->x) && (mouse_state.y >= d->y) && (mouse_state.x < d->x + d->w) && (mouse_state.y < d->y + d->h));
                if (swap)
                {
                    state2 = !state2;
                }
                if (((state1) && (!state2)) || ((state2) && (!state1)))
                {
                    d->flags ^= D_SELECTED;
                }

                if ((d->flags & D_SELECTED) && (d->flags & D_EXIT))
                {
                    d->flags ^= D_SELECTED;
                    return D_CLOSE;
                }
            }
            break;
        }
    }

    return ret;
}


/* t3gui_button_proc:
 *  A button object (the dp field points to the text string). This object
 *  can be selected by clicking on it with the mouse or by pressing its
 *  keyboard shortcut. If the D_EXIT flag is set, selecting it will close
 *  the dialog, otherwise it will toggle on and off.
 */
int t3gui_rounded_button_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int ret = D_O_K;
   assert(d);

   const char *text = d->dp;
   const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
   if (text == NULL) text = "";

   int select = D_SELECTED | D_TRACKMOUSE;

   if (msg == MSG_DRAW) {
      ALLEGRO_COLOR c1, c2;
      float rx = 4;
      float ry = 4;

      if (d->flags & select) {
         c1 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];
         c2 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
      }
      else {
         c1 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
         c2 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];
      }
      if (d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0]) {
         NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0];

         if (d->flags & D_GOTFOCUS && d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0]) p9 = d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0];
         if (d->flags & select && d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0]) p9 = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0];

         int w = max(d->w, get_nine_patch_bitmap_min_width(p9));
         int h = max(d->h, get_nine_patch_bitmap_min_height(p9));
         draw_nine_patch_bitmap(p9, al_map_rgba_f(1.0, 1.0, 1.0, 1.0), d->x+0.5, d->y+0.5, w, h);
         al_draw_text(font, c1, d->x+d->w/2, d->y+d->h/2-al_get_font_line_height(font)/2, ALLEGRO_ALIGN_CENTRE, text);
      } else { /* Default, primitives based */

         al_draw_rounded_rectangle(d->x+1.5, d->y+1.5, d->x+d->w-0.5, d->y+d->h-0.5, rx, ry, c1, 2.0);
         al_draw_filled_rounded_rectangle(d->x+0.5, d->y+0.5, d->x+d->w-1.5, d->y+d->h-1.5, rx, ry, c2);
         al_draw_rounded_rectangle(d->x+0.5, d->y+0.5, d->x+d->w-1.5, d->y+d->h-1.5, rx, ry, c1, 1.0);
         al_draw_text(font, c1, d->x+d->w/2, d->y+d->h/2-al_get_font_line_height(font)/2, ALLEGRO_ALIGN_CENTRE, text);

         if ((d->flags & D_GOTFOCUS) && (!(d->flags & select) || !(d->flags & D_EXIT))) {
            al_draw_rounded_rectangle(d->x+1.5, d->y+1.5, d->x+d->w-2.5, d->y+d->h-2.5, rx, ry, c1, 3.0);
         }
      }

      return ret;
   }

   return ret | t3gui_button_proc(msg, d, c);
}

typedef int (getbuttonfuncptr)(T3GUI_ELEMENT * d, void *dp3);

int t3gui_push_button_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;
    assert(d);

    const char *text = d->dp;
    const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
    if (text == NULL)
    {
        text = "";
    }

    switch (msg)
    {
        case MSG_LOSTMOUSE:
        {
            if(d->flags & D_INTERACT)
            {
                d->flags &= ~D_INTERACT;
            }
            break;
        }
        case MSG_GOTMOUSE:
        {
            ALLEGRO_MOUSE_STATE mouse_state;
            al_get_mouse_state(&mouse_state);
            if(mouse_state.buttons & 1)
            {
                d->flags |= D_INTERACT;
            }
            break;
        }
        case MSG_MOUSEUP:
        {
            if(c == 1)
            {
                if(d->flags & D_INTERACT)
                {
                    d->flags &= ~D_INTERACT;
                }

                if(d->dp2)
                {
                    getbuttonfuncptr *func = d->dp2;
                    func(d, d->user_data);
                }
                if(d->flags & D_EXIT)
                {
                    d->flags ^= D_SELECTED;
                    return D_CLOSE;
                }
            }
            break;
        }
        case MSG_KEY:
        {
            if(d->flags & D_INTERACT)
            {
                d->flags &= ~D_INTERACT;
            }

            if(d->dp2)
            {
                getbuttonfuncptr *func = d->dp2;
                func(d, d->user_data);
            }
            if(d->flags & D_EXIT)
            {
                d->flags ^= D_SELECTED;
                return D_CLOSE;
            }
            break;
        }
        default:
        {
            return t3gui_button_proc(msg, d, c);
        }
    }
    return ret;
}

/* d_clear_proc:
 *  Simple dialog procedure which just clears the screen. Useful as the
 *  first object in a dialog.
 */
int t3gui_clear_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int ret = D_O_K;
   assert(d);

   if (msg == MSG_DRAW) {
      al_set_clipping_rectangle(0, 0, al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display()));
      al_clear_to_color(d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG]);
   }

   return ret;
}

/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int t3gui_bitmap_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int ret = D_O_K;

   ALLEGRO_BITMAP *b;
   assert(d);

   b = (ALLEGRO_BITMAP *)d->dp;
   if (msg==MSG_DRAW && b)
      al_draw_bitmap(b, d->x, d->y, 0);

   return ret;
}


/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int t3gui_scaled_bitmap_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int ret = D_O_K;

   ALLEGRO_BITMAP *b;
   assert(d);

   b = (ALLEGRO_BITMAP *)d->dp;
   if (msg==MSG_DRAW && b)
      al_draw_scaled_bitmap(b, 0, 0, al_get_bitmap_width(b), al_get_bitmap_height(b), d->x, d->y, d->w, d->h, 0);

   return ret;
}

/* d_text_proc:
 *  Simple dialog procedure: draws the text string which is pointed to by dp.
 */
int t3gui_text_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;

    assert(d);
    if(msg == MSG_DRAW)
    {
        const char *text = d->dp;
        int flags = d->d1;
        int x = d->x;
        ALLEGRO_COLOR fg = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
        const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
        assert(font);

        if(!text)
        {
            text = "";
        }

        if(flags == ALLEGRO_ALIGN_CENTRE)
        {
            x += d->w/2;
        }
        if(flags == ALLEGRO_ALIGN_RIGHT)
        {
            x += d->w;
        }

        render_split_text(font, fg, x, d->y, d->w, 4, text);
    }

    return ret;
}

/* d_check_proc:
 *  Who needs C++ after all? This is derived from d_button_proc,
 *  but overrides the drawing routine to provide a check box.
 */
int t3gui_check_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    ALLEGRO_COLOR c1, c2;
    int select = D_INTERACT;
    int hover = D_GOTMOUSE;
    int tx, ty;
   int ret = D_O_K;
   assert(d);
   NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0];

   if (d->flags & D_GOTFOCUS && d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0])
   {
       p9 = d->theme->state[T3GUI_ELEMENT_STATE_HOVER].bitmap[0];
   }
   if (d->flags & select && d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0])
   {
       p9 = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].bitmap[0];
   }

   if (msg==MSG_DRAW) {
      const char *text = d->dp;
      const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
      ALLEGRO_COLOR fg = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
      ALLEGRO_COLOR bg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];
      int w = max(d->w, get_nine_patch_bitmap_min_width(p9));
      int h = max(d->h, get_nine_patch_bitmap_min_height(p9));
      if(d->flags & hover)
      {
          if(d->flags & D_SELECTED)
          {
              c1 = d->theme->state[T3GUI_ELEMENT_STATE_EXTRA].color[T3GUI_THEME_COLOR_BG];
          }
          else
          {
              c1 = d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_BG];
          }
          c2 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_HOVER].color[T3GUI_THEME_COLOR_FG];
      }
      else if(d->flags & D_SELECTED)
      {
          c1 = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].color[T3GUI_THEME_COLOR_BG];
          c2 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
      }
      else
      {
          c1 = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_DISABLED].color[T3GUI_THEME_COLOR_BG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];
          c2 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
      }

      if (d->w < h) h = d->w;

      float lw = 1.0;
      if (d->flags & D_GOTFOCUS) lw = 2.0;

      /* Draw check box */
      draw_nine_patch_bitmap(p9, c1, d->x, d->y, h, h);
      tx = d->x + h + 8;
      ty = d->y;
      al_draw_text(font, c2, tx, ty, 0, text);

      return ret;
   }

   return ret | t3gui_button_proc(msg, d, 0);
}


static ALLEGRO_COLOR interpolate_colour(ALLEGRO_COLOR from, ALLEGRO_COLOR to, int factor, int max_factor)
{
   float r, g, b, a;
   float r1, g1, b1, a1;
   float r2, g2, b2, a2;
   float f;

   al_unmap_rgba_f(from, &r1, &g1, &b1, &a1);
   al_unmap_rgba_f(to,   &r2, &g2, &b2, &a2);

   f = (float)factor / max_factor;
   if (f < 0.0) f = 0.0;
   if (f > 1.0) f = 1.0;

   r = r1 + (r2 - r1) * f;
   g = g1 + (g2 - g1) * f;
   b = b1 + (b2 - b1) * f;
   a = a1 + (a2 - a1) * f;

   return al_map_rgba_f(r, g, b, f);
}



/* Similar to d_textbut_agui_proc, but it works with a coloured gui_font */
/*  dp2 can hold a gui_font structure                                    */
/*  dp3 holds the corresponding palette                              */
/*  d1 measures how far hue has been changed (0-100%)                */
/*  d2 holds the change in saturation (fixed)                        */
/* The palette is taken from dp2 or dp3 depending on wether it is selected */
/*  selected or not. */
int t3gui_text_button_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int ret = D_O_K;

   if (msg==MSG_START)
      d->d1 = 0;

   if (msg==MSG_DRAW) {
      ALLEGRO_COLOR colour, fg, bg;
      const char *text = d->dp;

      const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];

      fg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
      bg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];

      if (d->flags & D_GOTFOCUS) {
         colour = bg;
         if (d->d1<66 && d->d2) {
            d->d1+=2;
            ret |= D_REDRAWME;
            colour = interpolate_colour(fg, bg, d->d1, 64);
         }
      } else {
         colour = fg;
         if (d->d1>-2 && d->d2) {
            d->d1-=2;
            ret |= D_REDRAWME;
            colour = interpolate_colour(fg, bg, d->d1, 64);
         }
      }

      int dx, dy, w, h;
      al_get_text_dimensions(font, d->dp, &dx, &dy, &w, &h);
      al_draw_textf(font, colour, d->x+(d->w-w)/2, d->y, 0, "%s", text);

      return ret;
   }

   return ret | t3gui_button_proc(msg, d, c);
}

/* d_radio_proc:
 *  GUI procedure for radio buttons.
 *  Parameters: d1-button group number; d2-button style (0=circle,1=square);
 *  dp-text to appear as label to the right of the button.
 */
int t3gui_radio_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int x, y, h, r, ret;
   ALLEGRO_COLOR fg, bg;
   int centerx, centery;
   float lw = 1;
   assert(d);

   const char *text = d->dp;
   const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];

   ret = D_O_K;

   switch(msg) {
      case MSG_DRAW:
         fg = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
         bg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG];

         h = al_get_font_line_height(font);
         y = d->y+(d->h-(h-al_get_font_descent(font)))/2;

         if (text)
            al_draw_text(font, fg, d->x + h + al_get_text_width(font, " "), y, 0, text);

            x = d->x;
            r = min(h, d->h)/2-0.5;

            centerx = d->x+r;
            centery = d->y+d->h/2;

            if (d->flags & D_GOTFOCUS) lw = 3.;

            NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0];
            draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG], d->x, y, h, h);
            if (d->flags & D_SELECTED)
            {
                draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], d->x + 2, y + 2, h - 4, h - 4);
           }

         return D_O_K;

      case MSG_KEY:
      case MSG_CLICK:
         if (d->flags & D_SELECTED) {
            return D_O_K;
         }
         break;

      case MSG_RADIO:
         if ((c == d->d1) && (d->flags & D_SELECTED)) {
            d->flags &= ~D_SELECTED;
            d->flags |= D_DIRTY;
         }
         break;
   }

   ret = t3gui_button_proc(msg, d, 0);

   if (((msg==MSG_KEY) || (msg==MSG_MOUSEUP)) &&
         (d->flags & D_SELECTED) && (!(d->flags & D_EXIT))) {
      d->flags &= ~D_SELECTED;
      t3gui_dialog_message(d->root, MSG_RADIO, d->d1, NULL);
      d->flags |= D_SELECTED;
      ret |= D_REDRAWME;
   }

   return ret;
}

/* d_slider_proc:
 *  A slider control object. This object returns a value in d2, in the
 *  range from 0 to d1. It will display as a vertical slider if h is
 *  greater than or equal to w; otherwise, it will display as a horizontal
 *  slider. dp can contain an optional bitmap to use for the slider handle;
 *  dp2 can contain an optional callback function, which is called each
 *  time d2 changes. The callback function should have the following
 *  prototype:
 *
 *  int function(void *dp3, int d2);
 *
 *  The d_slider_proc object will return the value of the callback function.
 */
int t3gui_slider_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   ALLEGRO_BITMAP *slhan = NULL;
   NINE_PATCH_BITMAP *p9;
   int oldpos, newpos;
   int vert = true;        /* flag: is slider vertical? */
   int hh = 7;             /* handle height (width for horizontal sliders) */
   int hmar;               /* handle margin */
   int slp;                /* slider position */
   int mp;                 /* mouse position */
   int irange;
   int slx, sly, slh, slw;
   int msx, msy;
   int retval = D_O_K;
   int upkey, downkey;
   int pgupkey, pgdnkey;
   int homekey, endkey;
   int delta;
   float slratio, slmax, slpos;
   int (*proc)(void *cbpointer, int d2value);
   int oldval;
   int range = d->d1;
   int value = d->d2;
   int offset = 0;
   assert(d);

   /* check for slider direction */
   if (d->h < d->w)
      vert = false;

   /* set up the metrics for the control */
   if (vert) {
      hh = d->h * d->h / (range + d->h);

      if (hh > d->h) hh = d->h;

      p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3];
      if(hh < get_nine_patch_bitmap_min_height(p9))
      {
          hh = get_nine_patch_bitmap_min_height(p9);
      }
   } else {
      hh = d->w * d->w / (range + d->w);

      if (hh > d->w) hh = d->w;

      p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3];
      if(hh < get_nine_patch_bitmap_min_width(p9))
      {
          hh = get_nine_patch_bitmap_min_width(p9);
      }
   }

   hmar = hh/2;
   irange = (vert) ? d->h : d->w;
   slmax = irange-hh;
   slratio = slmax / (d->d1);
   slpos = slratio * d->d2;
   slp = slpos;

   switch (msg) {

      case MSG_DRAW:
      /* check for slider direction */
      if (d->h < d->w)
         vert = false;

      if (vert) {
         hh = d->h * d->h / (range + d->h);

         if (hh > d->h) hh = d->h;

         p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3];
         if(hh < get_nine_patch_bitmap_min_height(p9))
         {
             hh = get_nine_patch_bitmap_min_height(p9);
         }
         offset = (d->h - hh) * value / range;
      } else {
         hh = d->w * d->w / (range + d->w);

         if (hh > d->w) hh = d->w;

         offset = (d->w - hh) * value / range;
      }

      if (vert) {
         slx = d->x;
         sly = d->y + offset;
         slw = d->w;
         slh = hh + 1;
      } else {
         slx = d->x + offset;
         sly = d->y;
         slw = hh;
         slh = d->h;
      }

      /* draw body */
      if (d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[2])
      {
          p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[2];
          draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_EXTRA].color[T3GUI_THEME_COLOR_BG], d->x, d->y, d->w, d->h);
      }
      if (!(d->flags & D_DISABLED) && d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3]) {
         p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3];
         int w = max(slw, get_nine_patch_bitmap_min_width(p9));
         int h = max(slh, get_nine_patch_bitmap_min_height(p9));
         draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_EXTRA].color[T3GUI_THEME_COLOR_FG], slx, sly, w, h);
      }
         break;

      case MSG_WANTFOCUS:
         return D_WANTKEYBOARD;

      case MSG_LOSTFOCUS:
         if (d->flags & D_TRACKMOUSE)
            return D_WANTKEYBOARD;
         break;

      case MSG_KEY:
         if (!(d->flags & D_GOTFOCUS))
            return D_WANTKEYBOARD;
         else
            return D_O_K;

      case MSG_KEYREPEAT:
      case MSG_KEYDOWN:
         /* handle movement keys to move slider */

         if (vert) {
            upkey = ALLEGRO_KEY_DOWN;
            downkey = ALLEGRO_KEY_UP;
            pgupkey = ALLEGRO_KEY_PGDN;
            pgdnkey = ALLEGRO_KEY_PGUP;
            homekey = ALLEGRO_KEY_HOME;
            endkey = ALLEGRO_KEY_END;
         }
         else {
            upkey = ALLEGRO_KEY_RIGHT;
            downkey = ALLEGRO_KEY_LEFT;
            pgupkey = ALLEGRO_KEY_PGDN;
            pgdnkey = ALLEGRO_KEY_PGUP;
            homekey = ALLEGRO_KEY_HOME;
            endkey = ALLEGRO_KEY_END;
         }

/*         if (c == upkey)
            delta = 1;
         else if (c == downkey)
            delta = -1;
         else if (c == pgdnkey)
            delta = -d->d1 / 16;
         else if (c == pgupkey)
            delta = d->d1 / 16;
         else if (c == homekey)
            delta = -d->d2;
         else if (c == endkey)
            delta = d->d1 - d->d2;
         else
            delta = 0; */

         if (delta) {
            oldpos = slp;
            oldval = d->d2;

            while (1) {
               d->d2 = d->d2+delta;
               slpos = slratio*d->d2;
               slp = (int)slpos;
               if ((slp != oldpos) || (d->d2 <= 0) || (d->d2 >= d->d1))
                  break;
            }

            if (d->d2 < 0)
               d->d2 = 0;
            if (d->d2 > d->d1)
               d->d2 = d->d1;

            retval = D_USED_CHAR;

            if (d->d2 != oldval) {
               /* call callback function here */
               if (d->dp2) {
                  proc = d->dp2;
                  retval |= (*proc)(d->dp3, d->d2);
               }

               retval |= D_REDRAWME;
            }
         }
         break;

      case MSG_VBALL:   // MSG_WHEEL
         if (vert) {
            oldval = d->d2;
            d->d2-=c;
            if (d->d2 < 0) d->d2 = 0;
            if (d->d2 > d->d1) d->d2 = d->d1;
            if (d->d2 != oldval) {
               /* call callback function here */
               if (d->dp2) {
                  proc = d->dp2;
                  retval |= (*proc)(d->dp3, d->d2);
               }

               retval |= D_REDRAWME;
            }
         }
         break;

      case MSG_HBALL:
         if (!vert) {
            oldval = d->d2;
            d->d2-=c;
            if (d->d2 < 0) d->d2 = 0;
            if (d->d2 > d->d1) d->d2 = d->d1;
            if (d->d2 != oldval) {
               /* call callback function here */
               if (d->dp2) {
                  proc = d->dp2;
                  retval |= (*proc)(d->dp3, d->d2);
               }

               retval |= D_REDRAWME;
            }
         }
         break;

      case MSG_CLICK:
         /* track the mouse until it is released */
         break;

      case MSG_MOUSEUP:
         if(c == 1)
         {
             d->flags &= ~D_TRACKMOUSE;
         }
         break;


      case MSG_MOUSEDOWN:
         if(c == 1)
         {
             d->flags |= D_TRACKMOUSE;
         }
         break;

      case MSG_MOUSEMOVE:
         msx = d->mousex;
         msy = d->mousey;
         oldval = d->d2;
         if (vert)
            //mp = (d->y+d->h-hmar)-msy;
            mp = msy - d->y - hmar;
         else
            mp = msx - d->x - hmar;
         if (mp < 0)
            mp = 0;
         if (mp > irange-hh)
            mp = irange-hh;
         slpos = mp;
         slmax = slpos / slratio;
         newpos = slmax;
         if (newpos != oldval) {
            d->d2 = newpos;

            /* call callback function here */
            if (d->dp2 != NULL) {
               proc = d->dp2;
               retval |= (*proc)(d->dp3, d->d2);
            }
            retval |= D_REDRAWME;
         }
         break;
   }

   return retval;
}

int t3gui_scroll_proc(int msg, T3GUI_ELEMENT *d, int c)
{
/*   assert(d);
   int vert = true;
   int hh = 0;
   int slx, sly, slh, slw;
   int retval = D_O_K;
   int range = d->d1;
   int value = d->d2;
   int offset = 0;

   if (msg == MSG_DRAW) {
      if (d->h < d->w)
         vert = false;

      if (vert) {
         hh = d->h * d->h / (range + d->h);

         if (hh > d->h) hh = d->h;

         offset = (d->h - hh) * value / range;
      } else {
         hh = d->w * d->w / (range + d->h);

         if (hh > d->w) hh = d->w;

         offset = (d->w - hh) * value / range;
      }

      if (vert) {
         slx = d->x+2;
         sly = d->y+2 + offset;
         slw = d->w-3;
         slh = hh-4;
      } else {
         slx = d->x+2 + offset;
         sly = d->y+2;
         slw = hh-4;
         slh = d->h-4;
      }

      NINE_PATCH_BITMAP *p9;

      if (d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[2])
      {
          p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[2];
          draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_EXTRA].color[T3GUI_THEME_COLOR_BG], d->x, d->y, d->w, d->h);
      }
      if (d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3]) {
         p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[3];
         int w = max(slw, get_nine_patch_bitmap_min_width(p9));
         int h = max(slh, get_nine_patch_bitmap_min_height(p9));
         draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_EXTRA].color[T3GUI_THEME_COLOR_FG], slx, sly, w, h);
      }

      if (d->flags & D_GOTFOCUS)
         al_draw_rectangle(d->x+0.5, d->y+0.5, d->x+d->w-0.5, d->y+d->h-0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 4.0);

      return retval;
  } */

   return t3gui_slider_proc(msg, d, c);
}

static int draw_textcursor(T3GUI_ELEMENT *d, bool draw, int *margin, int cursor_pos)
{
    assert(d);
    const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
    const char *text = d->dp;

    int r_margin = 0;
    int x, y;
    int x0 = d->x+4;
    int y0 = d->y - d->id2;
    int w = d->w - 4 - r_margin;
    float lx = 0;
    bool cursor_drawn = false;

    const char *s = text;
    int s_count = 0;

    x = 0;
    y = 0;

    if (margin)
    {
        r_margin = *margin;
    }

    char *buf = malloc(strlen(text) + 1);
    char lbuf[16] = {0};
    while (true)
    {
        x0 = d->x+4;
        y0 = d->y - d->id2;
        w = d->w - 4 - r_margin;

        s_count = 0;

        x = 0;
        y = 0;

        /* Parse the text and determine how much space we need to draw all of it (with and without a scroll bar) */
        while (s && *s)
        {
            char *p = buf;

            if (*s && *s == '\n')
            {
                x = 0;
                lx = 0;
                y += al_get_font_line_height(font);
                s++;
                s_count++;
                if(s_count == cursor_pos && d->tick % 2 == 0 && (d->flags & D_GOTFOCUS))
                {
                    al_draw_line(x0 + x + lx + 0.5, y0 + y - 2 + 0.5, x0 + x + lx + 0.5, y0 + y + al_get_font_line_height(font) + 2 + 0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 1.0);
                    cursor_drawn = true;
                }
                continue;
            }

            /* Copy next word */
            while (*s && uisspace(*s) && *s != '\n')
            {
                *p = *s;
                p++; s++;
            }
            while (*s && !uisspace(*s) && *s != '\n')
            {
                *p = *s;
                p++; s++;
            }
            *p = '\0';

            int tw = al_get_text_width(font, buf);
            lx = 0;

            if (x+4+tw < w)
            {
                p = buf;
                goto draw;
            }

            if (x == 0)
            {  /* Word is too long - must break the word (TODO) */
            }

            /* Strip leading space */
            p = buf;
            int want_cursor_spaces = 0;
            int want_cursor_lx = -1;
            while (*p && uisspace(*p))
            {
                p++;
                s_count++;
                want_cursor_spaces++;
                lx += al_get_text_width(font, " ");
                if(s_count == cursor_pos && d->tick % 2 == 0 && (d->flags & D_GOTFOCUS))
                {
                    want_cursor_lx = lx;
                }
            }
            if(want_cursor_lx >= 0 && lx != want_cursor_lx && d->tick % 2 == 0 && (d->flags & D_GOTFOCUS))
            {
                lx = want_cursor_lx;
                al_draw_line(x0 + x + lx + 0.5, y0 + y - 2 + 0.5, x0 + x + lx + 0.5, y0 + y +  al_get_font_line_height(font) + 2 + 0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 1.0);
                cursor_drawn = true;
            }
            tw = al_get_text_width(font, p);

            /* Line-wrap */
            y += al_get_font_line_height(font);
            x = 0;
            lx = 0;

draw:
            //if (!draw) printf("(%d %d) '%s'\n", x, y, p);
            if (draw)
            {
                int i;
                for(i = 0; i < ustrlen(p); i++)
                {
                    usetat(lbuf, 0, ugetat(p, i));
                    if(s_count == cursor_pos && d->tick % 2 == 0 && (d->flags & D_GOTFOCUS))
                    {
                        al_draw_line(x0 + x + lx + 0.5, y0 + y - 2 + 0.5, x0 + x + lx + 0.5, y0 + y + al_get_font_line_height(font) + 2 + 0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 1.0);
                        cursor_drawn = true;
                    }
                    lx += al_get_text_width(font, lbuf);
                    s_count++;
                }
                if(s_count == cursor_pos && d->tick % 2 == 0 && (d->flags & D_GOTFOCUS))
                {
                    al_draw_line(x0 + x + lx + 0.5, y0 + y - 2 + 0.5, x0 + x + lx + 0.5, y0 + y + al_get_font_line_height(font) + 2 + 0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 1.0);
                    cursor_drawn = true;
                }
            }
            x += tw;
        }

        y += al_get_font_line_height(font);

        if (draw)
        {
            break;
        }

        if (r_margin)
        {
            break;
        }

        if (y < d->h)
        {
            break;
        }

        r_margin = 16;
    }

    free(buf);

    if (margin)
    {
        *margin = r_margin;
    }

    return y;
}

static int draw_textbox(T3GUI_ELEMENT *d, bool draw, int *margin)
{
    assert(d);
    const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
    const char *text = d->dp;

    int r_margin = 0;
    int x, y;

    if (margin)
    {
        r_margin = *margin;
    }

    char *buf = malloc(strlen(text) + 1);
    while (true)
    {
        int x0 = d->x+4;
        int y0 = d->y - d->id2;
        int w = d->w - 4 - r_margin;

        const char *s = text;

        x = 0;
        y = 0;

        /* Parse the text and determine how much space we need to draw all of it (with and without a scroll bar) */
        while (s && *s)
        {
            char *p = buf;

            if (*s && *s == '\n')
            {
                x = 0;
                y += al_get_font_line_height(font);
                s++;
                continue;
            }

            /* Copy next word */
            while (*s && uisspace(*s) && *s != '\n')
            {
                *p = *s;
                p++; s++;
            }
            while (*s && !uisspace(*s) && *s != '\n')
            {
                *p = *s;
                p++; s++;
            }
            *p = '\0';

            int tw = al_get_text_width(font, buf);

            if (x+4+tw < w)
            {
                p = buf;
                goto draw;
            }

            if (x == 0)
            {  /* Word is too long - must break the word (TODO) */
            }

            /* Strip leading space */
            p = buf;
            while (*p && uisspace(*p))
            {
                p++;
            }
            tw = al_get_text_width(font, p);

            /* Line-wrap */
            y += al_get_font_line_height(font);
            x = 0;

draw:
            //if (!draw) printf("(%d %d) '%s'\n", x, y, p);
            if (draw)
            {
                al_draw_text(font, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], x0+x, y0+y, 0, p);
            }
            x += tw;
        }

        y += al_get_font_line_height(font);

        if (draw)
        {
            break;
        }

        if (r_margin)
        {
            break;
        }

        if (y < d->h)
        {
            break;
        }

        r_margin = 16;
    }

    free(buf);

    if (margin)
    {
        *margin = r_margin;
    }

    return y;
}

int t3gui_textbox_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;

    assert(d);
    bool external_scroll = d->flags & D_SCROLLBAR && d[1].proc == t3gui_scroll_proc;

    T3GUI_ELEMENT dd =
    {
        .proc = t3gui_scroll_proc,
        .x = d->x+d->w - d->d3,
        .y = d->y,
        .w = d->d3,
        .h = d->h,
        .theme = d->theme,
        .flags = d->flags,
        .d1 = d->id1 - d->h,
        .d2 = d->id2,
        .mousex = d->mousex,
        .mousey = d->mousey
    };

    if (msg != MSG_START && external_scroll)
    {
        d->id1 = d[1].d1 + d->h;
        d->id2 = d[1].d2;
        d->flags |= d[1].flags & D_DIRTY;
    }

    switch (msg)
    {
        case MSG_START:
        {
            /* Query size of required text box (id1) and size of scroll bar (d3) */
            d->d3 = 0;
//            d->id1 = draw_textbox(d, false, &d->d3);
            //printf("%d\n", d->d1);
            if (external_scroll)
            {
                initialise_vertical_scroll(d, d+1, d->d3);
                d[1].flags &= ~D_HIDDEN;
                if (d->d3 <= 0) d[1].flags |= D_HIDDEN;
            }
            break;
        }

        case MSG_DRAW:
        {
            draw_textbox(d, true, &d->d3);
            if (d->d3 > 0 && !external_scroll)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }

        case MSG_HBALL:
        case MSG_VBALL:
        {
            if (external_scroll)
            {
                ret |= d[1].proc(msg, d+1, c);
            }
            else if (d->d3 > 0)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }

        default:
        {
            if (d->d3 > 0 && !external_scroll)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }
    }

    if (msg != MSG_START && !external_scroll)
    {
        d->id1 = dd.d1 + d->h;
        d->id2 = dd.d2;
        d->flags = dd.flags;
    }

    return ret;
}

int t3gui_editbox_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int last_was_space, new_pos, i, k;
    int l;
    char *s, *t;
    assert(d);
    int ret = D_O_K;

    bool external_scroll = d->flags & D_SCROLLBAR && d[1].proc == t3gui_scroll_proc;

    s = d->dp;
    l = ustrlen(s);
    if (d->d2 > l)
       d->d2 = l;

    T3GUI_ELEMENT dd =
    {
        .proc = t3gui_scroll_proc,
        .x = d->x+d->w - d->d3,
        .y = d->y,
        .w = d->d3,
        .h = d->h,
        .theme = d->theme,
        .flags = d->flags,
        .d1 = d->id1 - d->h,
        .d2 = d->id2,
        .mousex = d->mousex,
        .mousey = d->mousey
    };

    if (msg != MSG_START && external_scroll)
    {
        d->id1 = d[1].d1 + d->h;
        d->id2 = d[1].d2;
        d->flags |= d[1].flags & D_DIRTY;
    }

    switch (msg)
    {
        case MSG_START:
        {
            d->d2 = strlen(d->dp);
            break;
        }
        case MSG_TIMER:
        {
            d->tick++;
            return D_REDRAW;
        }
        case MSG_DRAW:
        {
            t3gui_box_proc(MSG_DRAW, d, 0);
            draw_textbox(d, true, &d->d3);
            draw_textcursor(d, true, &d->d3, d->d2);
            if (d->d3 > 0 && !external_scroll)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }
        case MSG_MOUSEDOWN:
        {
            d->flags |= D_DIRTY;
            d->d2 = strlen(d->dp);
            return D_WANTKEYBOARD;
        }
        case MSG_WANTFOCUS:
        case MSG_LOSTFOCUS:
        case MSG_KEY:
        {
            d->d2 = strlen(d->dp);
            return D_WANTKEYBOARD;
        }
        case MSG_GOTMOUSE:
        {
            al_set_system_mouse_cursor(d->display, ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT);
            break;
        }
        case MSG_LOSTMOUSE:
        {
            al_set_system_mouse_cursor(d->display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            break;
        }

        case MSG_KEYUP:
        {
            if (c == ALLEGRO_KEY_TAB || c == ALLEGRO_KEY_ESCAPE)
            {
                d->flags |= D_INTERNAL;
                return D_O_K;
            }

            d->flags |= D_DIRTY;
            return D_USED_KEY;
        }

        case MSG_KEYREPEAT:
        case MSG_KEYDOWN:
        {
            (void)0;
            d->flags &= ~D_INTERNAL;
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state(&state);
            int key_shifts = 0;
            if (al_key_down(&state, ALLEGRO_KEY_LCTRL) || al_key_down(&state, ALLEGRO_KEY_RCTRL))
            {
                key_shifts |= ALLEGRO_KEYMOD_CTRL;
            }

            if (c == ALLEGRO_KEY_LEFT)
            {
                if (d->d2 > 0)
                {
                    if (key_shifts & ALLEGRO_KEYMOD_CTRL)
                    {
                        last_was_space = true;
                        new_pos = 0;
                        t = s;
                        for (i = 0; i < d->d2; i++)
                        {
                            k = ugetx(&t);
                            if (uisspace(k))
                            {
                                last_was_space = true;
                            }
                            else if (last_was_space)
                            {
                                last_was_space = false;
                                new_pos = i;
                            }
                        }
                        d->d2 = new_pos;
                    }
                    else
                    {
                        d->d2--;
                    }
                }
            }
            else if (c == ALLEGRO_KEY_RIGHT)
            {
                if (d->d2 < l)
                {
                    if (key_shifts & ALLEGRO_KEYMOD_CTRL)
                    {
                        t = s + uoffset(s, d->d2);
                        for (k = ugetx(&t); uisspace(k); k = ugetx(&t))
                        {
                            d->d2++;
                        }
                        for (; k && !uisspace(k); k = ugetx(&t))
                        {
                            d->d2++;
                        }
                    }
                    else
                    {
                        d->d2++;
                    }
                }
            }
            else if (c == ALLEGRO_KEY_HOME)
            {
                d->d2 = 0;
            }
            else if (c == ALLEGRO_KEY_END)
            {
                d->d2 = l;
            }
            else if (c == ALLEGRO_KEY_DELETE)
            {
                d->flags |= D_INTERNAL;
                if (d->d2 < l)
                {
                    uremove(s, d->d2);
                }
            }
            else if (c == ALLEGRO_KEY_BACKSPACE)
            {
                d->flags |= D_INTERNAL;
                if (d->d2 > 0)
                {
                    d->d2--;
                    uremove(s, d->d2);
                }
            }
            else if (c == ALLEGRO_KEY_ENTER)
            {
                if (d->flags & D_EXIT)
                {
                    d->flags |= D_DIRTY;
                    return D_CLOSE;
                }
                else
                {
                    if(l < d->d1)
                    {
                        uinsert(s, d->d2, '\n');
                        d->d2++;
                    }
                    return D_O_K;
                }
            }
            else if (c == ALLEGRO_KEY_TAB)
            {
                d->flags |= D_INTERNAL;
                return D_O_K;
            }
            else
            {
                /* don't process regular keys here: MSG_CHAR will do that */
                break;
            }
            d->flags |= D_DIRTY;
            return D_USED_KEY;
        }

        case MSG_CHAR:
        {
            if ((c >= ' ') && (uisok(c)) && ~d->flags & D_INTERNAL)
            {
                if (l < d->d1)
                {
                    uinsert(s, d->d2, c);
                    d->d2++;

                    d->flags |= D_DIRTY;
                }
                return D_USED_CHAR;
            }
            break;
        }
    }
    return ret;
}

static void flush_render(void)
{
    bool held = al_is_bitmap_drawing_held();

    if(held)
    {
        al_hold_bitmap_drawing(false);
        al_hold_bitmap_drawing(true);
    }
}

/* typedef for the listbox callback functions */
typedef const char *(getfuncptr)(int index, int *num_elem, void *dp3);

int t3gui_list_proc(int msg, T3GUI_ELEMENT *d, int c)
{
    int ret = D_O_K;
    assert(d);
    int i;
//    const char * right_text = NULL;
    int list_width = d->w;
//    int text_width = d->w;

    getfuncptr *func = d->dp;
    const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
    int nelem = 0;
    int visible_elements;
    int y = d->y;

    assert(func);

    func(-1, &nelem, d->dp3);

    visible_elements = d->h / al_get_font_line_height(d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0]);

    T3GUI_ELEMENT dd =
    {
        .proc = t3gui_scroll_proc,
        .x = d->x+d->w - d->d3,
        .y = d->y,
        .w = d->d3,
        .h = d->h,
        .theme = d->theme,
        .flags = d->flags,
        .d1 = (nelem + 1) * al_get_font_line_height(font) - d->h,
        .d2 = d->d2 * al_get_font_line_height(font),
        .mousex = d->mousex,
        .mousey = d->mousey
    };

    switch(msg)
    {
        case MSG_START:
        {
            /* Query size of required text box (d1) and size of scroll bar (d3) */
            d->d3 = 16;
            d->id1 = -1;
            d->id2 = -1;
            //d->d1 = draw_textbox(d, false, &d->d3);
            //printf("%d\n", d->d1);
            break;
        }

        case MSG_KEYDOWN:
        case MSG_KEYREPEAT:
        {
            int last_idx = d->d2 + d->h / al_get_font_line_height(font)-1;

            if(c == ALLEGRO_KEY_DOWN)
            {
                d->d1++;
                if(d->d1 > nelem-1) d->d1 = nelem-1;
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_UP)
            {
                d->d1--;
                if (d->d1 < 0) d->d1 = 0;
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_PGDN)
            {
                d->d1 += visible_elements;
				if(d->d1 >= nelem)
				{
					d->d1 = nelem - 1;
				}
				d->d2 += visible_elements - 1;
				if(d->d2 >= nelem - visible_elements)
				{
					d->d2 = nelem - visible_elements - 1;
				}
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_PGUP)
            {
                d->d1 -= visible_elements;
				if(d->d1 < 0)
				{
					d->d1 = 0;
				}
				d->d2 -= visible_elements;
				if(d->d2 < 0)
				{
					d->d2 = 0;
				}
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_HOME)
            {
                d->d1 = 0;
                d->d2 = 0;
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_END)
            {
                d->d1 = nelem - 1;
                d->d2 = nelem - visible_elements - 1;
                ret |= D_USED_KEY;
            }
            else if(c == ALLEGRO_KEY_ENTER)
            {
                d->id1 = d->d1;
                ret |= D_USED_KEY;
            }
            if(ret & D_USED_KEY)
            {
                if (d->d1 < d->d2) d->d2--;
                if (d->d1 > last_idx) d->d2++;
                dd.d2 = d->d2 * al_get_font_line_height(font);

                ret |= D_REDRAWME;
            }
            break;
        }

        case MSG_MOUSEUP:
        {
            if(d->d3 > 0 && dd.d1 > 0 && d->mousex > dd.x)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }

        case MSG_MOUSEDOWN:
        {
            if(d->d3 > 0 && dd.d1 > 0 && d->mousex > dd.x)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            else
            {
                int idx = d->d2 + (d->mousey - d->y) / al_get_font_line_height(font);
                if(idx >= nelem) idx = nelem-1;
                if(idx < 0) idx = 0;
                if(d->d1 != idx)
                {
                    d->d1 = idx;
                    ret |= D_REDRAWME;
                }
            }
            ret |= D_WANTKEYBOARD;
            break;
        }

        case MSG_DCLICK:
        {
            if(c == 1)
            {
                d->id1 = d->d1;
            }
            break;
        }

        /* TODO: handle scrolling with arrow keys (next/previous item in the list; scroll list as needed) */

        case MSG_DRAW:
        {
            int n;
            t3gui_box_proc(MSG_DRAW, d, 0);

            if(d->d3 > 0 && dd.d1 > 0)
            {
                flush_render();
                list_width = d->w - d->d3;
            }
            for(n = d->d2; n < nelem; n++)
            {
                ALLEGRO_COLOR fg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
                if(n == d->id2)
                {
                    fg = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_EG];
                }
                al_set_clipping_rectangle(d->x, d->y, list_width, d->h);
                if(d->d1 == n && d->flags & D_GOTFOCUS)
                {
                    al_draw_filled_rectangle(d->x+2.5,y+1.5,d->x+d->w-1.5,y+al_get_font_line_height(font)+1.5, d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].color[T3GUI_THEME_COLOR_FG]);
                    fg = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].color[T3GUI_THEME_COLOR_BG];
                    if(n == d->id2)
                    {
                        fg = d->theme->state[T3GUI_ELEMENT_STATE_SELECTED].color[T3GUI_THEME_COLOR_EG];
                    }
                }
                render_split_text(font, fg, d->x + 4, y + 2, list_width - 8, 4, func(n, NULL, d->dp3));
                y += al_get_font_line_height(font);
                if(y > d->y + d->h)
                {
                    break;
                }
            }
            if(d->d3 > 0 && dd.d1 > 0)
            {
                flush_render();
                al_set_clipping_rectangle(d->x, d->y, d->w, d->h);
            }
            NINE_PATCH_BITMAP *p9 = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[1];
            if(p9)
            {
                int w = max(d->w, get_nine_patch_bitmap_min_width(p9));
                int h = max(d->h, get_nine_patch_bitmap_min_height(p9));
                draw_nine_patch_bitmap(p9, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG], d->x, d->y, w, h);
            }
            if(d->d3 > 0 && dd.d1 > 0)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }

        case MSG_WANTFOCUS:
        case MSG_LOSTFOCUS:
        case MSG_KEY:
        {
            return D_WANTKEYBOARD;
        }
        default:
        {
            if(d->d3 > 0 && dd.d1 > 0)
            {
                ret |= t3gui_scroll_proc(msg, &dd, c);
            }
            break;
        }
    }

    if(msg != MSG_START)
    {
        d->d2 = dd.d2 / al_get_font_line_height(font);
        if(d->d2 >= nelem) d->d2 = nelem-1;
        if(d->d2 < 0) d->d2 = 0;
        d->flags = dd.flags;
    }

    return ret;
}

/* d_edit_proc:
 *  An editable text object (the dp field points to the string). When it
 *  has the input focus (obtained by clicking on it with the mouse), text
 *  can be typed into this object. The d1 field specifies the maximum
 *  number of characters that it will accept, and d2 is the text cursor
 *  position within the string.
 */
int t3gui_edit_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   const ALLEGRO_FONT *font = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0];
   ALLEGRO_COLOR fg, tc;
   int last_was_space, new_pos, i, k;
   int f, l, p, w, x, b, scroll, h;
   char buf[16];
   char *s, *t;
   assert(d);

   s = d->dp;
   l = ustrlen(s);
   if (d->d2 > l)
      d->d2 = l;

   /* calculate maximal number of displayable characters */
   if (d->d2 == l)  {
      usetc(buf+usetc(buf, ' '), 0);
      x = al_get_text_width(font, buf);
   }
   else
      x = 0;

   b = 0;

   for (p=d->d2; p>=0; p--) {
      usetc(buf+usetc(buf, ugetat(s, p)), 0);
      x += al_get_text_width(font, buf);
      b++;
      if (x > d->w)
         break;
   }

   if (x <= d->w) {
      b = l;
      scroll = false;
   }
   else {
      b--;
      scroll = true;
   }

   switch (msg) {

      case MSG_START:
         d->d2 = l;
         break;

      case MSG_TIMER:
      {
          d->tick++;
          return D_REDRAW;
      }
      case MSG_DRAW:
        draw_nine_patch_bitmap(d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[0], d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG], d->x, d->y, d->w, d->h);
         h = min(d->h, al_get_font_line_height(font)+3);
         fg = (d->flags & D_DISABLED) ? d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_MG] : d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];
         x = 0;

         if (scroll) {
            p = d->d2-b+1;
            b = d->d2;
         }
         else
            p = 0;

         for (; p<=b; p++) {
            f = ugetat(s, p);
            usetc(buf+usetc(buf, (f) ? f : ' '), 0);
            w = al_get_text_width(font, buf);
            if (x+4+w > d->w - 4)
               break;
            f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
            tc = d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG];

            if (f && d->tick % 2 == 0) {
               int dx, dy, w, hh;
               al_get_text_dimensions(font, buf, &dx, &dy, &w, &hh);
               if (w == 0) al_get_text_dimensions(font, "x", &dx, &dy, &w, &hh);
               al_draw_line(d->x+x+4+dx+0.5 - 1, d->y+0.5, d->x+x+4+dx+0.5 - 1, d->y+h-0.5, d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_FG], 1.0);
            }
            al_draw_text(font, tc, d->x+x+4, d->y+1, 0, buf);
            x += w;
         }
         if(d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[1])
         {
             draw_nine_patch_bitmap(d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].bitmap[1], d->theme->state[T3GUI_ELEMENT_STATE_NORMAL].color[T3GUI_THEME_COLOR_BG], d->x, d->y, d->w, d->h);
         }
         break;

      case MSG_MOUSEDOWN:
         x = d->x;

         if (scroll) {
            p = d->d2-b+1;
            b = d->d2;
         }
         else
            p = 0;

         for (; p<b; p++) {
            usetc(buf+usetc(buf, ugetat(s, p)), 0);
            x += al_get_text_width(font, buf);
            if (x > d->mousex)
               break;
         }
         d->d2 = clamp(0, p, l);
         d->flags |= D_DIRTY;
         return D_WANTKEYBOARD;

      case MSG_WANTFOCUS:
      {
          d->d2 = l;
          return D_WANTKEYBOARD;
      }
      case MSG_LOSTFOCUS:
      case MSG_KEY:
      {
          return D_WANTKEYBOARD;
      }
     case MSG_GOTMOUSE:
     {
         al_set_system_mouse_cursor(d->display, ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT);
         break;
     }
     case MSG_LOSTMOUSE:
     {
         al_set_system_mouse_cursor(d->display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
         break;
     }

      case MSG_KEYUP:
         if (c == ALLEGRO_KEY_TAB || c == ALLEGRO_KEY_ESCAPE) {
            d->flags |= D_INTERNAL;
            return D_O_K;
         }

         d->flags |= D_DIRTY;
         return D_USED_KEY;

      case MSG_KEYREPEAT:
      case MSG_KEYDOWN:
         (void)0;
         d->flags &= ~D_INTERNAL;
         ALLEGRO_KEYBOARD_STATE state;
         al_get_keyboard_state(&state);
         int key_shifts = 0;
         if (al_key_down(&state, ALLEGRO_KEY_LCTRL) || al_key_down(&state, ALLEGRO_KEY_RCTRL))
            key_shifts |= ALLEGRO_KEYMOD_CTRL;

         if (c == ALLEGRO_KEY_LEFT) {
            if (d->d2 > 0) {
               if (key_shifts & ALLEGRO_KEYMOD_CTRL) {
                  last_was_space = true;
                  new_pos = 0;
                  t = s;
                  for (i = 0; i < d->d2; i++) {
                     k = ugetx(&t);
                     if (uisspace(k))
                        last_was_space = true;
                     else if (last_was_space) {
                        last_was_space = false;
                        new_pos = i;
                     }
                  }
                  d->d2 = new_pos;
               }
               else
                  d->d2--;
            }
         }
         else if (c == ALLEGRO_KEY_RIGHT) {
            if (d->d2 < l) {
               if (key_shifts & ALLEGRO_KEYMOD_CTRL) {
                  t = s + uoffset(s, d->d2);
                  for (k = ugetx(&t); uisspace(k); k = ugetx(&t))
                     d->d2++;
                  for (; k && !uisspace(k); k = ugetx(&t))
                     d->d2++;
               }
               else
                  d->d2++;
            }
         }
         else if (c == ALLEGRO_KEY_HOME) {
            d->d2 = 0;
         }
         else if (c == ALLEGRO_KEY_END) {
            d->d2 = l;
         }
         else if (c == ALLEGRO_KEY_DELETE) {
            d->flags |= D_INTERNAL;
            if (d->d2 < l)
               uremove(s, d->d2);
         }
         else if (c == ALLEGRO_KEY_BACKSPACE) {
            d->flags |= D_INTERNAL;
            if (d->d2 > 0) {
               d->d2--;
               uremove(s, d->d2);
            }
         }
         else if (c == ALLEGRO_KEY_ENTER) {
            if (d->flags & D_EXIT) {
               d->flags |= D_DIRTY;
               return D_CLOSE;
            }
            else
               return D_O_K;
         }
         else if (c == ALLEGRO_KEY_TAB) {
            d->flags |= D_INTERNAL;
            return D_O_K;
         }
         else {
            /* don't process regular keys here: MSG_CHAR will do that */
            break;
         }
         d->flags |= D_DIRTY;
         return D_USED_KEY;

      case MSG_CHAR:
         if ((c >= ' ') && (uisok(c)) && ~d->flags & D_INTERNAL) {
            if (l < d->d1) {
               uinsert(s, d->d2, c);
               d->d2++;

               d->flags |= D_DIRTY;
            }
            return D_USED_CHAR;
         }
         break;
   }

   return D_O_K;
}

int t3gui_edit_integer_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   if (msg == MSG_CHAR)
      if (!isdigit(c)) return D_O_K;

   if (msg == MSG_DRAW) {
      char *string = d->dp;
      if (string == NULL || strlen(string) == 0) d->dp = "0";
      int ret = t3gui_edit_proc(msg, d, c);
      d->dp = string;
      return ret;
   }

   return t3gui_edit_proc(msg, d, c);
}

/* d_keyboard_proc:
 *  Invisible object for implementing keyboard shortcuts. When its key
 *  is pressed, it calls the function pointed to by dp (if any), which gets passed
 *  the dialog object as an argument. This function should return
 *  an integer, which will be passed back to the dialog manager. The key
 *  can be specified by putting an ASCII code in the key field or by
 *  putting ALLEGRO_KEY_* codes in d1 and d2.
 */
int t3gui_keyboard_proc(int msg, T3GUI_ELEMENT *d, int c)
{
   int (*proc)(T3GUI_ELEMENT *d) = d->dp;
   int ret = D_O_K;
   assert(d);

   switch (msg) {
      case MSG_START:
         d->w = d->h = 0;
         break;

      case MSG_CHAR:
         if ((c != d->d1) && (c != d->d2))
            break;

         ret |= D_USED_CHAR;
         /* fall through */

      case MSG_KEY:
         if (proc) {
            ret |= (*proc)(d);
         }
         if (d->flags & D_EXIT)
            ret |= D_CLOSE;
         break;
   }

   return ret;
}
