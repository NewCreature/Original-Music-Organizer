#include "../t3f/t3f.h"
#include "../instance.h"
#include "../constants.h"

void omo_tags_dialog_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i;

    if(t3f_key[ALLEGRO_KEY_ESCAPE])
    {
        omo_close_tags_dialog(app->ui, app);
        t3f_key[ALLEGRO_KEY_ESCAPE] = 0;
    }
    if(app->button_pressed == 0)
    {
        for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
        {
            if(omo_tag_type[i] && strcmp(app->ui->tags_text[i], app->ui->original_tags_text[i]))
            {
                al_set_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i], app->ui->tags_text[i]);
            }
        }
        omo_close_tags_dialog(app->ui, app);
        if(app->ui->tags_queue_entry >= 0)
        {
            omo_get_queue_entry_tags(app->player->queue, app->ui->tags_queue_entry, app->library);
        }
        else
        {
            omo_get_queue_tags(app->player->queue, app->library);
        }
        app->button_pressed = -1;
        t3f_key[ALLEGRO_KEY_ENTER] = 0;
    }
    else if(app->button_pressed == 1)
    {
        omo_close_tags_dialog(app->ui, app);
        app->button_pressed = -1;
    }
}
