#include "../t3f/t3f.h"

#include "../instance.h"
#include "dialog_proc.h"

void omo_resize_dialogs(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int queue_width, queue_height;
    int button_y, button_width, button_height;
    int i;

    button_height = 32;
    button_y = al_get_display_height(t3f_display) - button_height - 8;
    queue_width = al_get_display_width(t3f_display) - 16;
    queue_height = al_get_display_height(t3f_display) - button_height - 16 - 8;
    button_width = queue_width / 6;

    app->ui_queue_list_box_element->w = al_get_display_width(t3f_display);
    app->ui_queue_list_box_element->h = al_get_display_height(t3f_display);
    app->ui_queue_list_element->w = queue_width;
    app->ui_queue_list_element->h = queue_height;
    for(i = 0; i < 6; i++)
    {
        app->ui_button_element[i]->x = 8 + button_width * i;
        app->ui_button_element[i]->y = button_y;
        app->ui_button_element[i]->w = button_width;
        app->ui_button_element[i]->h = button_height;
    }
}

bool omo_setup_dialogs(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i;

    app->ui_box_theme = t3gui_load_theme("data/themes/basic/box_theme.ini");
    if(!app->ui_box_theme)
    {
        return false;
    }
    app->ui_button_theme = t3gui_load_theme("data/themes/basic/button_theme.ini");
    if(!app->ui_button_theme)
    {
        return false;
    }
    app->ui_list_box_theme = t3gui_load_theme("data/themes/basic/listbox_theme.ini");
    if(!app->ui_list_box_theme)
    {
        return false;
    }
    app->ui_dialog = t3gui_create_dialog();
    if(!app->ui_dialog)
    {
        return false;
    }
    app->ui_queue_list_box_element = t3gui_dialog_add_element(app->ui_dialog, app->ui_box_theme, t3gui_box_proc, 0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), 0, 0, 0, 0, NULL, NULL, NULL);
    app->ui_queue_list_element = t3gui_dialog_add_element(app->ui_dialog, app->ui_list_box_theme, t3gui_list_proc, 8, 8, al_get_display_width(t3f_display) - 16, al_get_display_height(t3f_display) - 16, 0, D_SETFOCUS, 0, 0, ui_queue_list_proc, NULL, app);
    for(i = 0; i < 6; i++)
    {
        app->ui_button_element[i] = t3gui_dialog_add_element(app->ui_dialog, app->ui_button_theme, t3gui_push_button_proc, 8, 8, 16, 16, 0, 0, 0, i, app->ui_button_text[i], ui_player_button_proc, app);
    }
    omo_resize_dialogs(data);

    return true;
}
