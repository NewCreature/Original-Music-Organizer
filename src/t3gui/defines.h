#ifndef T3GUI_DEFINES_H
#define T3GUI_DEFINES_H

#include <allegro5/allegro_font.h>
#include "nine_patch.h"

#define T3GUI_ELEMENT_STATES             5
#define T3GUI_ELEMENT_STATE_NORMAL       0
#define T3GUI_ELEMENT_STATE_HOVER        1
#define T3GUI_ELEMENT_STATE_SELECTED     2
#define T3GUI_ELEMENT_STATE_DISABLED     3
#define T3GUI_ELEMENT_STATE_EXTRA        4

#define T3GUI_PLAYER_NO_ESCAPE           1
#define T3GUI_PLAYER_CLEAR               2

#define T3GUI_DIALOG_VERSION          0
#define T3GUI_DIALOG_SUBVERSION       9
#define T3GUI_DIALOG_VERSION_STRING   "0.9"

#define EGGC_STOP    ALLEGRO_GET_EVENT_TYPE('E','G','G', 'S')
#define EGGC_PAUSE   ALLEGRO_GET_EVENT_TYPE('E','G','G', 'P')
#define EGGC_RESUME  ALLEGRO_GET_EVENT_TYPE('E','G','G', 'R')

//struct T3GUI_ELEMENT;

//typedef AL_METHOD(int, T3GUI_DIALOG_PROC, (int msg, struct T3GUI_ELEMENT *d, int c));

#define RGBA(r,g,b,a) { (r), (g), (b), (a) }
#define RGB(r,g,b)    RGBA(r,g,b,1.0)

/* Standard dialog colours */
#define t3gui_black       al_map_rgb_f(0.0, 0.0, 0.0)
#define t3gui_blue        al_map_rgb_f(0.0, 0.0, 0.5)
#define t3gui_green       al_map_rgb_f(0.0, 0.5, 0.0)
#define t3gui_cyan        al_map_rgb_f(0.0, 0.5, 0.5)
#define t3gui_red         al_map_rgb_f(0.5, 0.0, 0.0)
#define t3gui_purple      al_map_rgb_f(0.5, 0.0, 0.5)
#define t3gui_maroon      al_map_rgb_f(0.5, 0.5, 0.0)
#define t3gui_silver      al_map_rgb_f(0.5, 0.5, 0.5)
#define t3gui_grey        al_map_rgb_f(0.25, 0.25, 0.25)
#define t3gui_highblue    al_map_rgb_f(0.5, 0.5, 1.0)
#define t3gui_highgreen   al_map_rgb_f(0.5, 1.0, 0.5)
#define t3gui_highcyan    al_map_rgb_f(0.5, 1.0, 1.0)
#define t3gui_highred     al_map_rgb_f(1.0, 0.5, 0.5)
#define t3gui_magenta     al_map_rgb_f(1.0, 0.5, 1.0)
#define t3gui_yellow      al_map_rgb_f(1.0, 1.0, 0.5)
#define t3gui_white       al_map_rgb_f(1.0, 1.0, 1.0)
#define t3gui_trans       al_map_rgba_f(1.0, 1.0, 1.0, 0.0)

/* Special dialog ID to indicate that this item has no ID */
#define T3GUI_NO_ID       0

/* Widget alignment flags, for the frame widget */
#define T3GUI_WALIGN_HORIZONTAL 0
#define T3GUI_WALIGN_VERTICAL   1
#define T3GUI_WALIGN_LEFT       ALLEGRO_ALIGN_LEFT
#define T3GUI_WALIGN_CENTRE     ALLEGRO_ALIGN_CENTRE
#define T3GUI_WALIGN_RIGHT      ALLEGRO_ALIGN_RIGHT
#define T3GUI_WALIGN_TOP        ALLEGRO_ALIGN_LEFT
#define T3GUI_WALIGN_BOTTOM     ALLEGRO_ALIGN_RIGHT

#ifndef T3GUI_NO_FLAGS

/* bits for the flags field */
#define D_EXIT          0x0001   /* object makes the dialog exit */
#define D_SELECTED      0x0002   /* object is selected */
#define D_GOTFOCUS      0x0004   /* object has the input focus */
#define D_GOTMOUSE      0x0008   /* mouse is on top of object */
#define D_HIDDEN        0x0010   /* object is not visible */
#define D_DISABLED      0x0020   /* object is visible but inactive */
#define D_DIRTY         0x0040   /* object needs to be redrawn */
#define D_INTERNAL      0x0080   /* reserved for internal use */
#define D_TRACKMOUSE    0x0100   /* object is tracking the mouse */
#define D_VSCROLLBAR    0x0200   /* object uses the scrollbar in the next widget entry (vertical) */
#define D_HSCROLLBAR    0x0400   /* object uses the scrollbar in the second next widget entry (horizontal) */
#define D_SCROLLBAR     D_VSCROLLBAR
//#define D_GOTKEYBOARD   0x0800   /* from here on is free for your own use */
#define D_INTERACT      0x1000
#define D_SETFOCUS      0x2000

/* Event types */
#define T3GUI_EVENT_CLOSE       ALLEGRO_GET_EVENT_TYPE('e','g','g', 'c')
#define T3GUI_EVENT_CANCEL      ALLEGRO_GET_EVENT_TYPE('e','g','g', 'x')
#define T3GUI_EVENT_REDRAW      ALLEGRO_GET_EVENT_TYPE('e','g','g', 'd')
#define T3GUI_EVENT_LOSTFOCUS   ALLEGRO_GET_EVENT_TYPE('e','g','g', 'l')

/* return values for the dialog procedures */
#define D_O_K           0        /* normal exit status */
#define D_CLOSE         1        /* request to close the dialog */
#define D_REDRAW        2        /* request to redraw the dialog */
#define D_REDRAWME      4        /* request to redraw this object */
//#define D_WANTFOCUS     8        /* this object wants the input focus */
#define D_USED_CHAR     16       /* object has used the keypress */
#define D_USED_KEY      D_USED_CHAR
#define D_REDRAW_ALL    32       /* request to redraw all active dialogs */
#define D_DONTWANTMOUSE 64       /* this object does not want mouse focus */
#define D_CALLBACK      128      /* for callback functions: don't call super function */
#define D_WANTKEYBOARD  256

#define D_REDRAW_ANY (D_REDRAW | D_REDRAWME | D_REDRAW_ALL)

/* messages for the dialog procedures */
#define MSG_START       1        /* start the dialog, initialise */
#define MSG_END         2        /* dialog is finished - cleanup */
#define MSG_DRAW        3        /* draw the object */
#define MSG_CLICK       4        /* mouse click on the object */
#define MSG_DCLICK      5        /* double click on the object */
#define MSG_KEY         6        /* keyboard shortcut */
#define MSG_CHAR        7        /* unicode keyboard input */
#define MSG_KEYDOWN     8        /* key was pressed */
#define MSG_KEYUP       9        /* key was released */
#define MSG_KEYREPEAT   10       /* key is held down */
#define MSG_WANTFOCUS   11       /* does object want the input focus? */
#define MSG_GOTFOCUS    12       /* got the input focus */
#define MSG_LOSTFOCUS   13       /* lost the input focus */
#define MSG_GOTMOUSE    14       /* mouse on top of object */
#define MSG_LOSTMOUSE   15       /* mouse moved away from object */
#define MSG_IDLE        16       /* update any background stuff */
#define MSG_RADIO       17       /* clear radio buttons */
#define MSG_WHEEL       18       /* mouse wheel moved */
#define MSG_VBALL       MSG_WHEEL/* mouse ball moved (vertical) */
#define MSG_HBALL       19       /* mouse ball moved (horizontal) */
#define MSG_MOUSEDOWN   20       /* mouse button pressed */
#define MSG_MOUSEUP     21       /* mouse button released */
#define MSG_WANTMOUSE   22       /* does object want the mouse? */
#define MSG_MOUSEMOVE   23       /* mouse moved and object is tracking the mouse */
#define MSG_TIMER       24       /* timer tick */
#define MSG_USER        25       /* from here on are free... */

#endif

#endif
