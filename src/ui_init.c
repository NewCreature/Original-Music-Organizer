#include "instance.h"

#include "ui_menu_proc.h"

static char ui_queue_text[1024] = {0};

static void get_path_filename(const char * fn, char * outfn)
{
	int i, j;
	int pos = 0;

	for(i = strlen(fn) - 1; i >= 0; i--)
	{
		if(fn[i] == '/')
		{
			for(j = i + 1; j < strlen(fn); j++)
			{
				outfn[pos] = fn[j];
				pos++;
			}
			outfn[pos] = 0;
			break;
		}
	}
}

static char * ui_queue_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char section[1024] = {0};
    char display_fn[256] = {0};
    const char * val = NULL;
    const char * val2 = NULL;

    if(index < 0)
    {
        if(list_size)
        {
            *list_size = app->queue ? app->queue->entry_count : 0;
        }
        return NULL;
    }
    if(app->queue)
    {
        sprintf(section, "%s%s%s", app->queue->entry[index]->file, app->queue->entry[index]->sub_file ? "/" : "", app->queue->entry[index]->sub_file ? app->queue->entry[index]->sub_file : "");
        if(app->library)
        {
            val = al_get_config_value(app->library->file_database, section, "id");
            if(val)
            {
                val2 = al_get_config_value(app->library->entry_database, val, "title");
            }
        }
        if(val2)
        {
            sprintf(ui_queue_text, "%3d. %s", index + 1, val2);
        }
        else
        {
            get_path_filename(app->queue->entry[index]->file, display_fn);
            sprintf(ui_queue_text, "%3d. %s%s%s", index + 1, display_fn, app->queue->entry[index]->sub_file ? "/" : "", app->queue->entry[index]->sub_file ? app->queue->entry[index]->sub_file : "");
        }
       return ui_queue_text;
   }
   return NULL;
}

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
    t3f_add_menu_item(app->menu[OMO_MENU_PLAYBACK], "Shuffle", 0, NULL, omo_menu_playback_shuffle, NULL);

    app->menu[OMO_MENU_MAIN] = al_create_menu();
    if(!app->menu[OMO_MENU_MAIN])
    {
        return false;
    }
    t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "File", 0, app->menu[OMO_MENU_FILE], NULL, NULL);
    t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "Playback", 0, app->menu[OMO_MENU_PLAYBACK], NULL, NULL);

    app->ui_dialog = t3gui_create_dialog();
    if(!app->ui_dialog)
    {
        return false;
    }
    app->ui_queue_list_box_element = t3gui_dialog_add_element(app->ui_dialog, NULL, t3gui_box_proc, 0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), 0, 0, 0, 0, NULL, NULL, NULL);
    app->ui_queue_list_element = t3gui_dialog_add_element(app->ui_dialog, NULL, t3gui_list_proc, 8, 8, al_get_display_width(t3f_display) - 16, al_get_display_height(t3f_display) - 16, 0, 0, 0, 0, ui_queue_list_proc, NULL, app);

    return true;
}
