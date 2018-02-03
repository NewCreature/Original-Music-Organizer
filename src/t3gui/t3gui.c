#include <stdio.h>
#include "dialog.h"
#include "t3gui.h"
#include "theme.h"
#include "resource.h"

static T3GUI_PLAYER ** t3gui_dialog_player;
static int t3gui_dialog_players = 0;
static int t3gui_allocated_dialog_players = 0;
static ALLEGRO_EVENT_QUEUE * t3gui_queue = NULL;
static ALLEGRO_EVENT_SOURCE t3gui_event_source;
static bool event_source_initialized = false;
static bool t3gui_initialized = false;

static void t3gui_dialog_close_proc(T3GUI_ELEMENT * ep)
{
    t3gui_close_dialog_by_element(ep);
}

static T3GUI_ELEMENT * t3gui_allocate_element(int count)
{
	T3GUI_ELEMENT * dp;

	dp = malloc(sizeof(T3GUI_ELEMENT) * count);
	if(dp)
	{
		memset(dp, 0, sizeof(T3GUI_ELEMENT) * count);
	}
	return dp;
}

static void t3gui_destroy_element(T3GUI_ELEMENT * dp)
{
	free(dp);
}

static void t3gui_expand_dialog_element(T3GUI_DIALOG * dp)
{
	T3GUI_ELEMENT * old_element;
	int e, old_e;

	old_e = dp->allocated_elements;
	e = (dp->elements / T3GUI_DIALOG_ELEMENT_CHUNK_SIZE + 1) * T3GUI_DIALOG_ELEMENT_CHUNK_SIZE;
	if(e != old_e)
	{
        printf("expand\n");
		old_element = dp->element;
		dp->element = t3gui_allocate_element(e);
		if(dp->element)
		{
			memcpy(dp->element, old_element, sizeof(T3GUI_ELEMENT) * old_e);
			dp->allocated_elements = e;
			t3gui_destroy_element(old_element);
		}
		else
		{
			dp->element = old_element;
		}
	}
}

bool t3gui_init(void)
{
    if(!t3gui_initialized)
    {
        t3gui_initialized = true;
        return true;
    }
    return false;
}

void t3gui_exit(void)
{
    if(t3gui_initialized)
    {
        t3gui_destroy_theme(t3gui_get_default_theme());
        t3gui_unload_resources(NULL, true);
        t3gui_initialized = false;
    }
}

ALLEGRO_EVENT_SOURCE * t3gui_get_event_source(void)
{
    return &t3gui_event_source;
}

T3GUI_DIALOG * t3gui_create_dialog(void)
{
	T3GUI_DIALOG * dp;

	dp = malloc(sizeof(T3GUI_DIALOG));
	if(dp)
	{
		dp->allocated_elements = 0;
		dp->element = t3gui_allocate_element(T3GUI_DIALOG_ELEMENT_CHUNK_SIZE);
		if(dp->element)
		{
			dp->allocated_elements = T3GUI_DIALOG_ELEMENT_CHUNK_SIZE;
		}
		dp->elements = 0;
	}
	return dp;
}

void t3gui_destroy_dialog(T3GUI_DIALOG * dp)
{
	t3gui_destroy_element(dp->element);
	free(dp);
}

T3GUI_ELEMENT * t3gui_dialog_add_element(T3GUI_DIALOG * dialog, T3GUI_THEME * theme, int (*proc)(int msg, T3GUI_ELEMENT * d, int c), int x, int y, int w, int h, int key, uint64_t flags, int d1, int d2, void * dp, void * dp2, void * dp3)
{
	T3GUI_ELEMENT * ret = NULL;
	T3GUI_THEME * dtp = NULL;

	t3gui_expand_dialog_element(dialog);
	if(dialog->elements < dialog->allocated_elements)
	{
		dialog->element[dialog->elements].proc = proc;
		dialog->element[dialog->elements].x = x;
		dialog->element[dialog->elements].y = y;
		dialog->element[dialog->elements].w = w;
		dialog->element[dialog->elements].h = h;
		if(theme)
		{
			dialog->element[dialog->elements].theme = theme;
		}
		else
		{
			dtp = t3gui_get_default_theme();
			if(!dtp)
			{
				return NULL;
			}
			dialog->element[dialog->elements].theme = dtp;
		}
		dialog->element[dialog->elements].key = key;
		dialog->element[dialog->elements].flags = flags;
		dialog->element[dialog->elements].d1 = d1;
		dialog->element[dialog->elements].d2 = d2;
		dialog->element[dialog->elements].dp = dp;
		dialog->element[dialog->elements].dp2 = dp2;
		dialog->element[dialog->elements].dp3 = dp3;
		ret = &dialog->element[dialog->elements];
		dialog->elements++;
	}
	return ret;
}

void t3gui_center_dialog(T3GUI_DIALOG * dp, int w, int h)
{
    t3gui_centre_dialog(dp->element, w, h);
}

static T3GUI_PLAYER ** t3gui_allocate_player(int count)
{
	T3GUI_PLAYER ** dp;

	dp = malloc(sizeof(T3GUI_PLAYER *) * T3GUI_DIALOG_PLAYER_CHUNK_SIZE);
	if(dp)
	{
		memset(dp, 0, sizeof(T3GUI_PLAYER *) * T3GUI_DIALOG_PLAYER_CHUNK_SIZE);
	}
	return dp;
}

static void t3gui_destroy_player(T3GUI_PLAYER ** dp)
{
	free(dp);
}

static void t3gui_expand_player(void)
{
	T3GUI_PLAYER ** old_player;
	int e, old_e;
	int i;

	old_e = t3gui_allocated_dialog_players;
	e = (t3gui_dialog_players / T3GUI_DIALOG_PLAYER_CHUNK_SIZE + 1) * T3GUI_DIALOG_PLAYER_CHUNK_SIZE;
	if(e != old_e)
	{
		old_player = t3gui_dialog_player;
		t3gui_dialog_player = t3gui_allocate_player(e);
		if(t3gui_dialog_player)
		{
			t3gui_allocated_dialog_players = e;
			for(i = 0; i < old_e; i++)
			{
				memcpy(t3gui_dialog_player[i], old_player[i], sizeof(T3GUI_PLAYER));
			}
			t3gui_destroy_player(old_player);
		}
		else
		{
			t3gui_dialog_player = old_player;
		}
	}
}

bool t3gui_show_dialog_init(T3GUI_DIALOG * dp, ALLEGRO_EVENT_QUEUE * qp, int flags, void * user_data)
{
    int i;
    int focus = -1;

    /* ensure we have registered our event source to the user queue */
    if(qp)
    {
        if(!event_source_initialized)
        {
            al_init_user_event_source(&t3gui_event_source);
            event_source_initialized = true;
        }
        al_register_event_source(qp, &t3gui_event_source);
    }

    /* make sure we have enough space allocated for the new player */
    if(t3gui_allocated_dialog_players == 0)
    {
        t3gui_dialog_player = t3gui_allocate_player(T3GUI_DIALOG_PLAYER_CHUNK_SIZE);
        if(t3gui_dialog_player)
        {
            t3gui_allocated_dialog_players = T3GUI_DIALOG_PLAYER_CHUNK_SIZE;
        }
        else
        {
            return false;
        }
    }
    t3gui_expand_player();

    /* initialize the player */
    for(i = 0; i < dp->elements; i++)
    {
        dp->element[i].user_data = user_data;
    }
    for(i = 0; i < dp->elements; i++)
    {
        if(dp->element[i].flags & D_SETFOCUS)
        {
            focus = i;
            break;
        }
    }
    t3gui_dialog_player[t3gui_dialog_players] = t3gui_init_dialog(dp->element, focus, flags, qp, user_data, t3gui_dialog_close_proc);
    if(t3gui_dialog_player[t3gui_dialog_players])
    {
        /* pause previous player */
        if(t3gui_dialog_players > 0)
        {
            t3gui_pause_dialog(t3gui_dialog_player[t3gui_dialog_players - 1]);
        }
        t3gui_listen_for_events(t3gui_dialog_player[t3gui_dialog_players], al_get_keyboard_event_source());
        t3gui_listen_for_events(t3gui_dialog_player[t3gui_dialog_players], al_get_mouse_event_source());
        t3gui_listen_for_events(t3gui_dialog_player[t3gui_dialog_players], al_get_display_event_source(al_get_current_display()));
    }
    return true;
}

bool t3gui_show_dialog(T3GUI_DIALOG * dp, ALLEGRO_EVENT_QUEUE * qp, int flags, void * user_data)
{
    if(t3gui_show_dialog_init(dp, qp, flags, user_data))
    {
        t3gui_start_dialog(t3gui_dialog_player[t3gui_dialog_players]);
        t3gui_dialog_players++;
        return true;
    }
    return false;
}

bool t3gui_show_dialog_thread(T3GUI_DIALOG * dp, ALLEGRO_EVENT_QUEUE * qp, int flags, void * user_data)
{
    if(t3gui_show_dialog_init(dp, qp, flags, user_data))
    {
        t3gui_start_dialog_thread(t3gui_dialog_player[t3gui_dialog_players]);
        t3gui_dialog_players++;
        return true;
    }
    return false;
}

bool t3gui_close_dialog_by_element(T3GUI_ELEMENT * ep)
{
    int i, j;

    for(i = 0; i < t3gui_dialog_players; i++)
    {
        if(ep == t3gui_dialog_player[i]->dialog)
        {
            if(t3gui_dialog_player[i]->threaded)
            {
                t3gui_stop_dialog_thread(t3gui_dialog_player[i]);
            }
            else
            {
                t3gui_stop_dialog(t3gui_dialog_player[i]);
            }
            if(t3gui_queue && t3gui_dialog_players == 1)
            {
                al_unregister_event_source(t3gui_queue, t3gui_get_player_event_source(t3gui_dialog_player[i]));
            }
            t3gui_shutdown_dialog(t3gui_dialog_player[i]);
            for(j = i; j < t3gui_dialog_players - 1; j++)
            {
                t3gui_dialog_player[j] = t3gui_dialog_player[j + 1];
            }
            t3gui_dialog_players--;
            if(t3gui_dialog_players <= 0)
            {
                t3gui_destroy_player(t3gui_dialog_player);
                t3gui_allocated_dialog_players = 0;
            }
            else
            {
                t3gui_resume_dialog(t3gui_dialog_player[t3gui_dialog_players - 1]);
            }
            return true;
        }
    }
    return false;
}

bool t3gui_close_dialog(T3GUI_DIALOG * dp)
{
    return t3gui_close_dialog_by_element(dp->element);
}

void t3gui_logic(void)
{
    if(t3gui_dialog_players)
    {
        t3gui_process_dialog(t3gui_dialog_player[t3gui_dialog_players - 1]);
    }
}

void t3gui_render(void)
{
    int i;

    for(i = 0; i < t3gui_dialog_players; i++)
    {
        t3gui_draw_dialog(t3gui_dialog_player[i]);
    }
}

int t3gui_get_active_dialogs(void)
{
    return t3gui_dialog_players;
}
