#include "instance.h"

#include "ui_menu_proc.h"

bool omo_setup_menus(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->menu[OMO_MENU_FILE] = al_create_menu();
    if(!app->menu[OMO_MENU_FILE])
    {
        return false;
    }
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Play Files", 0, NULL, omo_menu_file_play_files, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Queue Files", 0, NULL, omo_menu_file_queue_files, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Play Folder", 0, NULL, omo_menu_file_play_folder, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Queue Folder", 0, NULL, omo_menu_file_queue_folder, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Exit", 0, NULL, omo_menu_file_exit, NULL);

    app->menu[OMO_MENU_PLAYBACK] = al_create_menu();
    if(!app->menu[OMO_MENU_PLAYBACK])
    {
        return false;
    }
    t3f_add_menu_item(app->menu[OMO_MENU_PLAYBACK], "Play", 0, NULL, omo_menu_playback_play, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_PLAYBACK], "Pause", 0, NULL, omo_menu_playback_pause, NULL);

    app->menu[OMO_MENU_MAIN] = al_create_menu();
    if(!app->menu[OMO_MENU_MAIN])
    {
        return false;
    }
    t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "File", 0, app->menu[OMO_MENU_FILE], NULL, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "Playback", 0, app->menu[OMO_MENU_PLAYBACK], NULL, NULL);

    return true;
}
