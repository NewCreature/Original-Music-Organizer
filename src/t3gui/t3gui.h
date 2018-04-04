#ifndef T3GUI_H
#define T3GUI_H

#include "element.h"
#include "dialog.h"
#include "nine_patch.h"
#include "player.h"

#define T3GUI_DIALOG_ELEMENT_CHUNK_SIZE 128
#define T3GUI_DIALOG_PLAYER_CHUNK_SIZE  16

typedef struct
{

    T3GUI_ELEMENT * element;
    int elements;
    int allocated_elements;

} T3GUI_DIALOG;

bool t3gui_init(void);
void t3gui_exit(void);
ALLEGRO_EVENT_SOURCE * t3gui_get_event_source(void);

T3GUI_DIALOG * t3gui_create_dialog(void);
void t3gui_destroy_dialog(T3GUI_DIALOG * dp);
T3GUI_ELEMENT * t3gui_dialog_add_element(T3GUI_DIALOG * dialog, T3GUI_THEME * theme, int (*proc)(int msg, T3GUI_ELEMENT * d, int c), int x, int y, int w, int h, int key, uint64_t flags, int d1, int d2, void * dp, void * dp2, void * dp3);
void t3gui_center_dialog(T3GUI_DIALOG * dp, int w, int h);

bool t3gui_show_dialog(T3GUI_DIALOG * dp, ALLEGRO_EVENT_QUEUE * qp, int flags, void * user_data);
bool t3gui_show_dialog_thread(T3GUI_DIALOG * dp, ALLEGRO_EVENT_QUEUE * qp, int flags, void * user_data);
bool t3gui_close_dialog(T3GUI_DIALOG * dp);
bool t3gui_close_dialog_by_element(T3GUI_ELEMENT * ep);
void t3gui_logic(void);
void t3gui_render(void);
int t3gui_get_active_dialogs(void);

#endif
