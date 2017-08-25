#include "../t3f/t3f.h"
#include "../t3f/file_utils.h"

#include "../instance.h"
#include "../archive_handlers/registry.h"
#include "../codec_handlers/registry.h"
#include "../queue.h"
#include "../library.h"
#include "../init.h"
#include "../file_chooser.h"

static char type_buf[1024] = {0};

static const char * omo_get_type_string(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i, j;

    strcpy(type_buf, "");
    for(i = 0; i < app->codec_handler_registry->codec_handlers; i++)
    {
        for(j = 0; j < app->codec_handler_registry->codec_handler[i].types; j++)
        {
            strcat(type_buf, "*");
            strcat(type_buf, app->codec_handler_registry->codec_handler[i].type[j]);
            strcat(type_buf, ";");
        }
    }
    for(i = 0; i < app->archive_handler_registry->archive_handlers; i++)
    {
        for(j = 0; j < app->archive_handler_registry->archive_handler[i].types; j++)
        {
            strcat(type_buf, "*");
            strcat(type_buf, app->archive_handler_registry->archive_handler[i].type[j]);
            strcat(type_buf, ";");
        }
    }
    type_buf[strlen(type_buf) - 1] = '\0';
    return type_buf;
}

int omo_menu_file_play_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 0;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
    return 1;
}

int omo_menu_file_queue_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 1;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
    return 1;
}

int omo_menu_file_play_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 2;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
    return 1;
}

int omo_menu_file_queue_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 3;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
    return 1;
}

int omo_menu_file_add_library_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 4;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select library folder.", al_get_config_value(t3f_config, "Settings", "last_music_folder"), ALLEGRO_FILECHOOSER_FOLDER, true);
    return 1;
}

int omo_menu_file_clear_library_folders(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char buf[4];

    sprintf(buf, "%d", 0);
    al_set_config_value(t3f_config, "Settings", "library_folders", buf);
    omo_destroy_library(app->library);
    omo_setup_library(app, omo_library_setup_update_proc);
    return 0;
}

int omo_menu_file_exit(void * data)
{
    t3f_exit();
    return 1;
}

int omo_menu_playback_play(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    omo_resume_player(app->player);
    return 1;
}

int omo_menu_playback_pause(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    omo_pause_player(app->player);
    return 1;
}

int omo_menu_playback_shuffle(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_QUEUE * new_queue;
    int i, r;

    if(app->player->queue)
    {
        /* stop currently playing song */
        omo_stop_player(app->player);

        /* create new queue */
        new_queue = omo_create_queue(app->player->queue->entry_count);
        if(new_queue)
        {
            for(i = 0; i < new_queue->entry_size; i++)
            {
                r = t3f_rand(&app->rng_state) % app->player->queue->entry_count;
                omo_add_file_to_queue(new_queue, app->player->queue->entry[r]->file, app->player->queue->entry[r]->sub_file, app->player->queue->entry[r]->track);
                omo_delete_queue_item(app->player->queue, r);
            }
            omo_destroy_queue(app->player->queue);
            app->player->queue = new_queue;
            app->player->queue_pos = 0;
        }
    }
    return 1;
}
