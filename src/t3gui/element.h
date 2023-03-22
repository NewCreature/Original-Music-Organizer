#ifndef T3GUI_ELEMENT_H
#define T3GUI_ELEMENT_H

#include "theme.h"
#include "nine_patch.h"

struct T3GUI_ELEMENT;
typedef AL_METHOD(int, T3GUI_DIALOG_PROC, (int msg, struct T3GUI_ELEMENT *d, int c));

typedef struct T3GUI_ELEMENT
{

  T3GUI_DIALOG_PROC proc;
  int x, y, w, h;               /* position and size of the element */
  T3GUI_THEME * theme;          /* theme to apply to element */
  int key;                      /* keyboard shortcut (ASCII code) */
  uint64_t flags;               /* flags about the object state */
  int d1, d2;                   /* any data the object might require */
  void *dp, *dp2, *dp3;         /* pointers to more object data */
  void * user_data;

  uint32_t id;                  /* Unique ID by which to find this widget */
  T3GUI_DIALOG_PROC callback;

  int d3, d4;
  int parent;
  int id1, id2, id3;
  int ed1, ed2, ed3;
  void * dp4, * dp5, * dp6, *dp7;

  /* Private/internal data */
  struct T3GUI_ELEMENT *root;
  int mousex, mousey;
  ALLEGRO_DISPLAY * display;
  int tick;

} T3GUI_ELEMENT;

#endif
