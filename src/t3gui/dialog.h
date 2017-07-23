/* EGGDialog, an Allgro 4-style GUI library for Allegro 5.
 * Version 0.9
 * Copyright (C) 2013 Evert Glebbeek
 *
 * See LICENCE for more information.
 */
#ifndef A45_DIALOG_H
#define A45_DIALOG_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdint.h>
#include "nine_patch.h"
#include "element.h"
#include "player.h"

int t3gui_find_mouse_object(T3GUI_ELEMENT *d, int mouse_x, int mouse_y);
int t3gui_object_message(T3GUI_ELEMENT *dialog, int msg, int c);
int t3gui_dialog_message(T3GUI_ELEMENT *dialog, int msg, int c, int *obj);

T3GUI_ELEMENT *t3gui_find_widget_id(T3GUI_ELEMENT *dialog, uint32_t id);

void t3gui_get_dialog_bounding_box(T3GUI_ELEMENT *dialog, int *min_x, int *min_y, int *max_x, int *max_y);
void t3gui_centre_dialog(T3GUI_ELEMENT *dialog, int w, int h);
void t3gui_position_dialog(T3GUI_ELEMENT *dialog, int x, int y);

int t3gui_do_dialog_interval(T3GUI_ELEMENT *dialog, double speed_sec, int focus);
int t3gui_do_dialog(T3GUI_ELEMENT *dialog, int focus);

int t3gui_find_dialog_focus(T3GUI_ELEMENT *dialog);

/* ID/index conversions */
uint32_t t3gui_index_to_id(T3GUI_ELEMENT *dialog, int index);
int t3gui_id_to_index(T3GUI_ELEMENT *dialog, uint32_t id);

/* Allocation and testing for unique IDs.
 * Of limited use if dialogs are allocated statically because it's easier
 * to assign them by hand in that case.
 */
bool t3gui_id_is_unique(T3GUI_ELEMENT *dialog, uint32_t id);
uint32_t get_unique_id(T3GUI_ELEMENT *dialog);

/* TODO: revert to a colour palette and integer colour indices? */


/* Dialog procedures and helpers */
void initialise_vertical_scroll(const T3GUI_ELEMENT *parent, T3GUI_ELEMENT *scroll, int scroll_w);
void initialise_horizontal_scroll(const T3GUI_ELEMENT *parent, T3GUI_ELEMENT *scroll, int scroll_h);

int t3gui_window_frame_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_frame_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_box_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_button_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_rounded_button_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_push_button_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_text_button_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_clear_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_bitmap_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_scaled_bitmap_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_text_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_check_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_radio_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_slider_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_scroll_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_textbox_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_list_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_edit_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_edit_integer_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_keyboard_proc(int msg, T3GUI_ELEMENT *d, int c);
int t3gui_editbox_proc(int msg, T3GUI_ELEMENT *d, int c);

/* TODO: add at least the following: */
//int d_text_list_proc (int msg, A4_DIALOG *d, int c);
//int d_menu_proc (int msg, A4_DIALOG *d, int c);

#endif
