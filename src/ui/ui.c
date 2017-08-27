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
    uip->main_theme = omo_load_theme("data/themes/basic/omo_theme.ini", 0);
    if(!uip->main_theme)
    {
        return false;
    }

    return true;
}

static void free_ui_data(OMO_UI * uip)
{
    omo_destroy_theme(uip->main_theme);
}

bool omo_create_main_dialog(OMO_UI * uip, int mode, int width, int height, void * data)
{
    int buttons[6] = {OMO_THEME_BITMAP_PREVIOUS_TRACK, OMO_THEME_BITMAP_PLAY, OMO_THEME_BITMAP_STOP, OMO_THEME_BITMAP_NEXT_TRACK, OMO_THEME_BITMAP_OPEN, OMO_THEME_BITMAP_ADD};
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
        uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);
        uip->ui_queue_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_queue_list_proc, NULL, data);
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i] = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, uip->ui_button_text[i], ui_player_button_proc, uip->main_theme->bitmap[buttons[i]]);
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

        uip->ui_queue_list_box_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, width, height, 0, 0, 0, 0, NULL, NULL, NULL);

        uip->ui_artist_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, D_SETFOCUS, 0, 0, ui_artist_list_proc, NULL, data);

        uip->ui_album_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_album_list_proc, NULL, data);

        uip->ui_song_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_song_list_proc, NULL, data);

        /* create queue list and controls */
        uip->ui_queue_list_element = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_list_proc, 8, 8, width - 16, height - 16, 0, 0, 0, 0, ui_queue_list_proc, NULL, data);
        for(i = 0; i < 6; i++)
        {
            uip->ui_button_element[i] = t3gui_dialog_add_element(uip->ui_dialog, uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, uip->ui_button_text[i], ui_player_button_proc, uip->main_theme->bitmap[buttons[i]]);
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
    int row = 0;
    int rows = 0;
    int column = 0;
    int i;
    int edit_flags = D_SETFOCUS;

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
    {
        if(omo_tag_type[i])
        {
            rows++;
        }
    }
    if(rows % 2)
    {
        rows++;
    }
    h = al_get_font_line_height(uip->main_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font) * 3 + 4;
    h *= rows / 2;
    h += 8;
//    h += 32 + 8;
    uip->tags_display = al_create_display(640, h);
    if(uip->tags_display)
    {
        al_register_event_source(t3f_queue, al_get_display_event_source(uip->tags_display));
        al_set_target_bitmap(al_get_backbuffer(uip->tags_display));
        uip->popup_theme = omo_load_theme("data/themes/basic/omo_theme.ini", 1);
        if(!uip->popup_theme)
        {
            goto fail;
        }
        uip->tags_dialog = t3gui_create_dialog();
        if(uip->tags_dialog)
        {
            t3gui_dialog_add_element(uip->tags_dialog, uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_BOX], t3gui_box_proc, 0, 0, 640, h, 0, 0, 0, 0, NULL, NULL, NULL);
            for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
            {
                if(omo_tag_type[i])
                {
                    row++;
                    if(row > rows / 2)
                    {
                        row = 0;
                        column = 1;
                        y = 8;
                    }
                    t3gui_dialog_add_element(uip->tags_dialog, uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_text_proc, 8 + 320 * column, y, 320 - 16, al_get_font_line_height(uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font), 0, 0, 0, 0, (void *)omo_tag_type[i], NULL, NULL);
                    y += al_get_font_line_height(uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font) + 2;
                    strcpy(uip->original_tags_text[i], uip->tags_text[i]);
                    t3gui_dialog_add_element(uip->tags_dialog, uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX], t3gui_edit_proc, 8 + 320 * column, y, 320 - 16, al_get_font_line_height(uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font) + 4, 0, edit_flags, 256, 0, uip->tags_text[i], NULL, NULL);
                    edit_flags = 0;
                    y += al_get_font_line_height(uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_LIST_BOX]->state[0].font) * 2 + 2;
                }
            }
            y += 12;
            uip->tags_ok_button_element = t3gui_dialog_add_element(uip->tags_dialog, uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 8 + 320 * column, y, 320 / 2 - 8 - 4, 32, '\r', 0, 0, 0, "Okay", ui_tags_button_proc, NULL);
            t3gui_dialog_add_element(uip->tags_dialog, uip->popup_theme->gui_theme[OMO_THEME_GUI_THEME_BUTTON], t3gui_push_button_proc, 320 * column + 320 / 2 + 4, y, 320 / 2 - 8 - 4, 32, 0, 0, 0, 1, "Cancel", ui_tags_button_proc, NULL);
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
        if(uip->popup_theme)
        {
            omo_destroy_theme(uip->popup_theme);
            uip->popup_theme = NULL;
        }
    }
    return false;
}

void omo_close_tags_dialog(OMO_UI * uip, void * data)
{
    t3gui_close_dialog(uip->tags_dialog);
    t3gui_destroy_dialog(uip->tags_dialog);
    omo_destroy_theme(uip->popup_theme);
    uip->popup_theme = NULL;
    t3gui_unload_resources(uip->tags_display, true);
    al_destroy_display(uip->tags_display);
    uip->tags_display = NULL;
}
