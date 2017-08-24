#include "../t3f/t3f.h"
#include "../t3gui/t3gui.h"
#include "../t3gui/resource.h"
#include "../instance.h"
#include "ui.h"
#include "dialog_proc.h"
#include "../constants.h"

static void resize_dialogs(OMO_UI * uip, int mode, int width, int height)
{
    int queue_width, queue_height;
    int pane_width;
    int button_y, button_width, button_height;
    int bezel;
    int i;

    bezel = 8;
    button_height = 32;
    button_y = height - button_height - 8;

    uip->ui_queue_list_box_element->w = width;
    uip->ui_queue_list_box_element->h = height;
    if(mode == 0)
    {
        queue_width = width - 16;
        queue_height = height - button_height - 16 - 8;
        button_width = queue_width / 6;

        uip->ui_queue_list_element->w = queue_width;
        uip->ui_queue_list_element->h = queue_height;
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i]->x = 8 + button_width * i;
            uip->ui_button_element[i]->y = button_y;
            uip->ui_button_element[i]->w = button_width;
            uip->ui_button_element[i]->h = button_height;
        }
    }
    else
    {
        pane_width = width / 4;
        uip->ui_artist_list_element->x = pane_width * 0 + bezel;
        uip->ui_artist_list_element->y = 0 + bezel;
        uip->ui_artist_list_element->w = pane_width - bezel * 2 + bezel / 2;
        uip->ui_artist_list_element->h = height - bezel * 2;

        uip->ui_album_list_element->x = pane_width * 1 + bezel - bezel / 2;
        uip->ui_album_list_element->y = 0 + bezel;
        uip->ui_album_list_element->w = pane_width - bezel * 2 + bezel;
        uip->ui_album_list_element->h = height - bezel * 2;

        uip->ui_song_list_element->x = pane_width * 2 + bezel - bezel / 2;
        uip->ui_song_list_element->y = 0 + bezel;
        uip->ui_song_list_element->w = pane_width - bezel * 2 + bezel;
        uip->ui_song_list_element->h = height - bezel * 2;

        uip->ui_queue_list_element->x = pane_width * 3 + bezel - bezel / 2;
        queue_width = width - uip->ui_queue_list_element->x - bezel;
        queue_height = height - button_height - 16 - 8;
        uip->ui_queue_list_element->y = 0 + bezel;
        uip->ui_queue_list_element->w = queue_width;
        uip->ui_queue_list_element->h = height - bezel * 2 - button_height - bezel;
        button_width = queue_width / 6;
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i]->x = uip->ui_queue_list_element->x + button_width * i;
            uip->ui_button_element[i]->y = button_y;
            uip->ui_button_element[i]->w = button_width;
            uip->ui_button_element[i]->h = button_height;
        }
    }
}

static bool load_ui_data(OMO_UI * uip)
{
    uip->ui_box_theme = t3gui_load_theme("data/themes/basic/box_theme.ini");
    if(!uip->ui_box_theme)
    {
        return false;
    }
    uip->ui_button_theme = t3gui_load_theme("data/themes/basic/button_theme.ini");
    if(!uip->ui_button_theme)
    {
        return false;
    }
    uip->ui_list_box_theme = t3gui_load_theme("data/themes/basic/listbox_theme.ini");
    if(!uip->ui_list_box_theme)
    {
        return false;
    }

    return true;
}

static void free_ui_data(OMO_UI * uip)
{
    t3gui_destroy_theme(uip->ui_box_theme);
    t3gui_destroy_theme(uip->ui_button_theme);
    t3gui_destroy_theme(uip->ui_list_box_theme);
}

bool omo_create_main_dialog(OMO_UI * uip, int mode, int width, int height, void * data)
{
    int i;

    if(uip->ui_dialog)
    {
        t3gui_destroy_dialog(uip->ui_dialog);
    }
    if(mode == 0)
    {
        uip->ui_dialog = t3gui_create_dialog();
        if(!uip->ui_dialog)
        {
            return false;
        }
        uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_box_theme, t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);
        uip->ui_queue_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_list_box_theme, t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_queue_list_proc, NULL, data);
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i] = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_button_theme, t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, uip->ui_button_text[i], ui_player_button_proc, data);
        }
        resize_dialogs(uip, mode, width, height);
        return true;
    }
    else
    {
        uip->ui_dialog = t3gui_create_dialog();
        if(!uip->ui_dialog)
        {
            return false;
        }

        uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_box_theme, t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);

        uip->ui_artist_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_list_box_theme, t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_artist_list_proc, NULL, data);

        uip->ui_album_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_list_box_theme, t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_album_list_proc, NULL, data);

        uip->ui_song_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_list_box_theme, t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_song_list_proc, NULL, data);

        /* create queue list and controls */
        uip->ui_queue_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_list_box_theme, t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_queue_list_proc, NULL, data);
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i] = t3gui_dialog_add_element(uip->ui_dialog, uip->ui_button_theme, t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, uip->ui_button_text[i], ui_player_button_proc, data);
        }
        resize_dialogs(uip, mode, width, height);
        return true;
    }
    return false;
}

OMO_UI * omo_create_ui(void)
{
    OMO_UI * uip;

    uip = malloc(sizeof(OMO_UI));
    if(uip)
    {
        memset(uip, 0, sizeof(OMO_UI));
        load_ui_data(uip);
    }
    return uip;
}

void omo_destroy_ui(OMO_UI * uip)
{
    free_ui_data(uip);
    t3gui_destroy_dialog(uip->ui_dialog);
    free(uip);
}

void omo_resize_ui(OMO_UI * uip, int mode, int width, int height)
{
    resize_dialogs(uip, mode, width, height);
}

bool omo_open_tags_dialog(OMO_UI * uip, void * data)
{
    int y = 8;
    int h = 8;
    int i;
    int edit_flags = D_SETFOCUS;

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);
    for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
    {
        if(omo_tag_type[i])
        {
            h += al_get_font_line_height(uip->ui_list_box_theme->state[0].font) * 3 + 4;
        }
    }
    h += 32 + 8;
    uip->tags_display = al_create_display(320, h);
    if(uip->tags_display)
    {
        al_register_event_source(t3f_queue, al_get_display_event_source(uip->tags_display));
        al_set_target_bitmap(al_get_backbuffer(uip->tags_display));
        uip->tags_box_theme = t3gui_load_theme("data/themes/basic/box_theme.ini");
        if(!uip->tags_box_theme)
        {
            goto fail;
        }
        uip->tags_list_box_theme = t3gui_load_theme("data/themes/basic/listbox_theme.ini");
        if(!uip->tags_list_box_theme)
        {
            goto fail;
        }
        uip->tags_button_theme = t3gui_load_theme("data/themes/basic/tags_button_theme.ini");
        if(!uip->tags_button_theme)
        {
            goto fail;
        }
        uip->tags_dialog = t3gui_create_dialog();
        if(uip->tags_dialog)
        {
            t3gui_dialog_add_element(uip->tags_dialog, uip->tags_box_theme, t3gui_box_proc, 0, 0, 320, h, 0, 0, 0, 0, NULL, NULL, NULL);
            for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
            {
                if(omo_tag_type[i])
                {
                    t3gui_dialog_add_element(uip->tags_dialog, uip->tags_list_box_theme, t3gui_text_proc, 8, y, 320 - 16, al_get_font_line_height(uip->tags_list_box_theme->state[0].font), 0, 0, 0, 0, (void *)omo_tag_type[i], NULL, NULL);
                    y += al_get_font_line_height(uip->tags_list_box_theme->state[0].font) + 2;
                    strcpy(uip->original_tags_text[i], uip->tags_text[i]);
                    t3gui_dialog_add_element(uip->tags_dialog, uip->tags_list_box_theme, t3gui_edit_proc, 8, y, 320 - 16, al_get_font_line_height(uip->tags_list_box_theme->state[0].font) + 4, 0, edit_flags, 256, 0, uip->tags_text[i], NULL, NULL);
                    edit_flags = 0;
                    y += al_get_font_line_height(uip->tags_list_box_theme->state[0].font) * 2 + 2;
                }
            }
            uip->tags_ok_button_element = t3gui_dialog_add_element(uip->tags_dialog, uip->tags_button_theme, t3gui_push_button_proc, 8, y, 320 / 2 - 8 - 4, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, data);
            t3gui_dialog_add_element(uip->tags_dialog, uip->tags_button_theme, t3gui_push_button_proc, 320 / 2 + 4, y, 320 / 2 - 8 - 4, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, data);
            t3gui_show_dialog(uip->tags_dialog, t3f_queue, T3GUI_PLAYER_CLEAR, data);
            return true;
        }
    }
    fail:
    {
        if(uip->tags_display)
        {
            al_destroy_display(uip->tags_display);
            uip->tags_display = NULL;
        }
        if(uip->tags_box_theme)
        {
            t3gui_destroy_theme(uip->tags_box_theme);
            uip->tags_box_theme = NULL;
        }
        if(uip->tags_list_box_theme)
        {
            t3gui_destroy_theme(uip->tags_list_box_theme);
            uip->tags_list_box_theme = NULL;
        }
        if(uip->tags_button_theme)
        {
            t3gui_destroy_theme(uip->tags_button_theme);
            uip->tags_button_theme = NULL;
        }
    }
    return false;
}

void omo_close_tags_dialog(OMO_UI * uip, void * data)
{
    t3gui_close_dialog(uip->tags_dialog);
    t3gui_destroy_dialog(uip->tags_dialog);
    t3gui_destroy_theme(uip->tags_box_theme);
    t3gui_unload_resources(uip->tags_display, true);
    al_destroy_display(uip->tags_display);
    uip->tags_display = NULL;
}
