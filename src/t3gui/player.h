#ifndef T3GUI_PLAYER_H
#define T3GUI_PLAYER_H

/* stored information about the state of an active GUI dialog */
typedef struct T3GUI_PLAYER
{
    int obj;
    int res;
    int mouse_obj;
//    int focus_obj;
    int click_obj;
    int click_button;
    int keyboard_obj;
    int stick, x_axis, y_axis;
    int joy_x, joy_y, joy_b;
    bool redraw;
    bool draw_veto;
    bool focus_follows_mouse;
    bool pass_events;
    bool paused;
    bool threaded;
    bool escape_down;
    T3GUI_ELEMENT * dialog;
    ALLEGRO_EVENT_QUEUE * input;
    ALLEGRO_EVENT_SOURCE event_src;
    ALLEGRO_THREAD * thread;
    ALLEGRO_DISPLAY * display;
    bool user_queue;
//    void (*event_proc)(ALLEGRO_EVENT * event, T3GUI_ELEMENT * ep, void * data);
    void * user_data;
    void (*close_proc)(struct T3GUI_ELEMENT * ep);
    int flags;

    /* thread control */
    ALLEGRO_TIMER * dclick_timer;
    ALLEGRO_TIMER * joy_timer;
    ALLEGRO_TIMER * cursor_timer;
    bool running;
    bool no_close_callback;
    bool shift;
    int click_count;
} T3GUI_PLAYER;

T3GUI_PLAYER * t3gui_init_dialog(T3GUI_ELEMENT * dialog, int focus_obj, int flags, ALLEGRO_EVENT_QUEUE * qp, void * user_data, void (*close_proc)(T3GUI_ELEMENT * ep));
bool t3gui_start_dialog(T3GUI_PLAYER * player);
bool t3gui_stop_dialog(T3GUI_PLAYER * player);
bool t3gui_start_dialog_thread(T3GUI_PLAYER * player);
bool t3gui_stop_dialog_thread(T3GUI_PLAYER * player);
void t3gui_shutdown_dialog(T3GUI_PLAYER * player);

void t3gui_set_player_focus(T3GUI_PLAYER * player, int obj);

/* Tell a dialog player to listen for events from a particular source */
bool t3gui_listen_for_events(T3GUI_PLAYER *player, ALLEGRO_EVENT_SOURCE *src);

/* Get the event source associated with a dialog player */
ALLEGRO_EVENT_SOURCE * t3gui_get_player_event_source(T3GUI_PLAYER *player);

bool t3gui_pause_dialog(T3GUI_PLAYER * player);
bool t3gui_resume_dialog(T3GUI_PLAYER * player);

void t3gui_process_dialog(T3GUI_PLAYER * player);
bool t3gui_draw_dialog(T3GUI_PLAYER *player);

#endif
