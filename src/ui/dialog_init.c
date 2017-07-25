#include "../t3f/t3f.h"

#include "../instance.h"
#include "dialog_proc.h"

bool omo_setup_dialogs(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->ui_dialog = t3gui_create_dialog();
    if(!app->ui_dialog)
    {
        return false;
    }
    app->ui_queue_list_box_element = t3gui_dialog_add_element(app->ui_dialog, NULL, t3gui_box_proc, 0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), 0, 0, 0, 0, NULL, NULL, NULL);
    app->ui_queue_list_element = t3gui_dialog_add_element(app->ui_dialog, NULL, t3gui_list_proc, 8, 8, al_get_display_width(t3f_display) - 16, al_get_display_height(t3f_display) - 16, 0, 0, 0, 0, ui_queue_list_proc, NULL, app);

    return true;
}
