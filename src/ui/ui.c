#include "../t3f/t3f.h"
#include "../t3gui/t3gui.h"
#include "../instance.h"
#include "ui.h"
#include "dialog_proc.h"

static void resize_dialogs(OMO_UI * uip, int width, int height)
{
    int queue_width, queue_height;
    int button_y, button_width, button_height;
    int i;

    button_height = 32;
    button_y = height - button_height - 8;
    queue_width = width - 16;
    queue_height = height - button_height - 16 - 8;
    button_width = queue_width / 6;

    uip->ui_queue_list_box_element->w = width;
    uip->ui_queue_list_box_element->h = height;
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

static bool setup_dialogs(OMO_UI * uip, int mode, int width, int height, void * data)
{
    int i;

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
    resize_dialogs(uip, width, height);

    return true;
}

OMO_UI * omo_create_ui(int mode, int width, int height, void * data)
{
    OMO_UI * uip;

    uip = malloc(sizeof(OMO_UI));
    if(uip)
    {
        memset(uip, 0, sizeof(OMO_UI));
        setup_dialogs(uip, mode, width, height, data);
    }
    return uip;
}

void omo_destroy_ui(OMO_UI * uip)
{
    t3gui_destroy_dialog(uip->ui_dialog);
    free(uip);
}

void omo_resize_ui(OMO_UI * uip, int width, int height)
{
    resize_dialogs(uip, width, height);
}

bool omo_open_tags_dialog(OMO_UI * uip, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char * tag_types[7] = {"Album Artist", "Artist", "Album", "Track", "Disc", "Title", "Comment"};
    int y = 8;
    int i;

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);
    uip->tags_display = al_create_display(320, 240);
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
        uip->tags_button_theme = t3gui_load_theme("data/themes/basic/button_theme.ini");
        if(!uip->tags_button_theme)
        {
            goto fail;
        }
        uip->tags_dialog = t3gui_create_dialog();
        if(uip->tags_dialog)
        {
            t3gui_dialog_add_element(uip->tags_dialog, uip->tags_box_theme, t3gui_box_proc, 0, 0, 320, 240, 0, 0, 0, 0, NULL, NULL, NULL);
            for(i = 0; i < 7; i++)
            {
                t3gui_dialog_add_element(uip->tags_dialog, uip->tags_box_theme, t3gui_text_proc, 8, y, 320 - 16, al_get_font_line_height(uip->tags_box_theme->state[0].font), 0, 0, 0, 0, tag_types[i], NULL, NULL);
                y += al_get_font_line_height(uip->tags_box_theme->state[0].font) + 2;
                t3gui_dialog_add_element(uip->tags_dialog, uip->tags_list_box_theme, t3gui_edit_proc, 8, y, 320 - 16, al_get_font_line_height(uip->tags_box_theme->state[0].font) + 4, 0, 0, 256, 0, uip->tags_text[i], NULL, NULL);
                y += al_get_font_line_height(uip->tags_box_theme->state[0].font) * 2 + 2;
            }
            t3gui_show_dialog(app->ui->tags_dialog, t3f_queue, T3GUI_PLAYER_CLEAR, app);
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
