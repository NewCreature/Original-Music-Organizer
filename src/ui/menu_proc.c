#include "../t3f/t3f.h"
#include "../t3f/file_utils.h"

#include "../instance.h"
#include "../archive_handlers/registry.h"
#include "../codec_handlers/registry.h"
#include "../queue.h"

static void * file_chooser_thread_proc(ALLEGRO_THREAD * thread, void * arg)
{
    APP_INSTANCE * app = (APP_INSTANCE *)arg;

    if(al_show_native_file_dialog(al_get_current_display(), app->file_chooser))
	{
        app->file_chooser_done = true;
	}
    else
    {
        al_destroy_native_file_dialog(app->file_chooser);
        app->file_chooser = NULL;
    }
	return NULL;
}

static bool omo_start_file_chooser(void * data, const char * title, const char * types, int mode)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    const char * last_music_filename = al_get_config_value(t3f_config, "App Settings", last_music_filename);

    app->file_chooser = al_create_native_file_dialog(last_music_filename, title, types, mode);
    if(!app->file_chooser)
    {
        return false;
    }
    app->file_chooser_thread = al_create_thread(file_chooser_thread_proc, data);
    if(!app->file_chooser_thread)
    {
        al_destroy_native_file_dialog(app->file_chooser);
        app->file_chooser = NULL;
        return false;
    }
    al_start_thread(app->file_chooser_thread);
    return true;
}

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
    omo_start_file_chooser(data, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE);
    return 1;
}

int omo_menu_file_queue_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 1;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE);
    return 1;
}

int omo_menu_file_play_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 2;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER);
    return 1;
}

int omo_menu_file_queue_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    app->file_chooser_mode = 2;
    app->file_chooser_done = false;
    omo_start_file_chooser(data, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER);
    return 1;
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
                omo_add_file_to_queue(new_queue, app->player->queue->entry[r]->file, app->player->queue->entry[r]->sub_file);
                omo_delete_queue_item(app->player->queue, r);
            }
            omo_destroy_queue(app->player->queue);
            app->player->queue = new_queue;
            app->player->queue_pos = 0;
        }
    }
    return 1;
}
