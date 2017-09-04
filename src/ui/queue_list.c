#include "../instance.h"
#include "menu_proc.h"

void omo_queue_list_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->player->queue && app->ui->ui_queue_list_element->id1 >= 0)
    {
        if(app->player->state == OMO_PLAYER_STATE_PLAYING)
        {
            app->player->queue_pos = app->ui->ui_queue_list_element->id1 - 1;
            omo_play_next_song(app->player);
        }
        else
        {
            app->player->queue_pos = app->ui->ui_queue_list_element->id1;
            omo_start_player(app->player);
        }
        app->ui->ui_queue_list_element->id1 = -1;
    }
    sprintf(app->ui->ui_button_text[0], "|<");
    if(app->player->state == OMO_PLAYER_STATE_PLAYING)
    {
        sprintf(app->ui->ui_button_text[1], "||");
    }
    else
    {
        sprintf(app->ui->ui_button_text[1], ">");
    }
    sprintf(app->ui->ui_button_text[2], "[]");
    sprintf(app->ui->ui_button_text[3], ">|");
    sprintf(app->ui->ui_button_text[4], "^");
    sprintf(app->ui->ui_button_text[5], "+");
    switch(app->button_pressed)
    {
        case 0:
        {
            omo_play_previous_song(app->player);
            break;
        }
        case 1:
        {
            switch(app->player->state)
            {
                case OMO_PLAYER_STATE_STOPPED:
                {
                    app->player->queue_pos = app->ui->ui_queue_list_element->d1;
                    omo_start_player(app->player);
                    break;
                }
                case OMO_PLAYER_STATE_PLAYING:
                {
                    omo_pause_player(app->player);
                    break;
                }
                case OMO_PLAYER_STATE_PAUSED:
                {
                    omo_resume_player(app->player);
                    break;
                }
            }
            break;
        }
        case 2:
        {
            omo_stop_player(app->player);
            break;
        }
        case 3:
        {
            omo_play_next_song(app->player);
            break;
        }
        case 4:
        {
            if(OMO_KEY_CTRL)
            {
                omo_menu_file_play_folder(data);
                t3f_key[ALLEGRO_KEY_COMMAND] = 0;
                t3f_key[ALLEGRO_KEY_LCTRL] = 0;
                t3f_key[ALLEGRO_KEY_RCTRL] = 0;
            }
            else
            {
                omo_menu_file_play_files(data);
            }
            break;
        }
        case 5:
        {
            if(OMO_KEY_CTRL)
            {
                omo_menu_file_queue_folder(data);
                t3f_key[ALLEGRO_KEY_COMMAND] = 0;
                t3f_key[ALLEGRO_KEY_LCTRL] = 0;
                t3f_key[ALLEGRO_KEY_RCTRL] = 0;
            }
            else
            {
                omo_menu_file_queue_files(data);
            }
            break;
        }
    }
    app->button_pressed = -1;
}
