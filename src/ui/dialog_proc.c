#include "../t3f/t3f.h"

#include "../instance.h"

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

char * ui_queue_list_proc(int index, int *list_size, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char section[1024] = {0};
    char display_fn[256] = {0};
    const char * val = NULL;
    const char * val2 = NULL;

    if(index < 0)
    {
        if(list_size && app->player)
        {
            *list_size = app->player->queue ? app->player->queue->entry_count : 0;
        }
        return NULL;
    }
    if(app->player && app->player->queue)
    {
        sprintf(section, "%s%s%s", app->player->queue->entry[index]->file, app->player->queue->entry[index]->sub_file ? "/" : "", app->player->queue->entry[index]->sub_file ? app->player->queue->entry[index]->sub_file : "");
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
            get_path_filename(app->player->queue->entry[index]->file, display_fn);
            sprintf(ui_queue_text, "%3d. %s%s%s", index + 1, display_fn, app->player->queue->entry[index]->sub_file ? "/" : "", app->player->queue->entry[index]->sub_file ? app->player->queue->entry[index]->sub_file : "");
        }
       return ui_queue_text;
   }
   return NULL;
}

int ui_player_button_proc(T3GUI_ELEMENT * ep, void * dp3)
{
	APP_INSTANCE * app = (APP_INSTANCE *)dp3;

	app->button_pressed = ep->d2;
	return 0;
}
