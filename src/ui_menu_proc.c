#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "DUMBA5/dumba5.h"

#include "instance.h"
#include "player_registry.h"
#include "queue.h"

bool omo_play_file(void * data, const char * fn)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    printf("play: %s\n", fn);
    app->player = omo_get_player(&app->player_registry, fn);
    if(app->player)
    {
        app->player->load_file(fn);
        app->player->play();
    }

    return false;
}

void omo_stop_file(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->player)
    {
        app->player->stop();
        app->player = NULL;
    }
}

void omo_pause_file(void * data, bool paused)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->player)
    {
        app->player->pause(paused);
    }
}

static char type_buf[1024] = {0};

static const char * omo_get_type_string(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i, j;

    strcpy(type_buf, "");
    for(i = 0; i < app->player_registry.players; i++)
    {
        for(j = 0; j < app->player_registry.player[i].types; j++)
        {
            strcat(type_buf, "*");
            strcat(type_buf, app->player_registry.player[i].type[j]);
            strcat(type_buf, ";");
        }
    }
    type_buf[strlen(type_buf) - 1] = '\0';
    return type_buf;
}

int omo_menu_file_play_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;
    int i;

	fc = al_create_native_file_dialog(app->last_music_filename, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE);
	if(!fc)
	{
		goto fail;
	}
	if(!al_show_native_file_dialog(al_get_current_display(), fc))
	{
		goto fail;
	}
	if(!al_get_native_file_dialog_count(fc))
	{
		goto fail;
	}
    omo_stop_file(data);
    if(app->queue)
    {
        omo_destroy_queue(app->queue);
    }
    app->queue = omo_create_queue(al_get_native_file_dialog_count(fc));
    if(app->queue)
    {
        for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
        {
            if(omo_get_player(&app->player_registry, al_get_native_file_dialog_path(fc, i)))
            {
                omo_add_file_to_queue(app->queue, al_get_native_file_dialog_path(fc, i));
            }
        }
        app->queue_pos = -1;
    }
    al_destroy_native_file_dialog(fc);
    return 1;

	fail:
	{
		if(fc)
		{
			al_destroy_native_file_dialog(fc);
		}
		return 0;
	}
    return 1;
}

int omo_menu_file_queue_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;
    OMO_QUEUE * new_queue;
    int i;

	fc = al_create_native_file_dialog(app->last_music_filename, "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE);
	if(!fc)
	{
		goto fail;
	}
	if(!al_show_native_file_dialog(al_get_current_display(), fc))
	{
		goto fail;
	}
	if(!al_get_native_file_dialog_count(fc))
	{
		goto fail;
	}
    new_queue = omo_create_queue(al_get_native_file_dialog_count(fc) + (app->queue ? app->queue->file_count : 0));
    if(!new_queue)
    {
        goto fail;
    }
    if(app->queue)
    {
        for(i = 0; i < app->queue->file_count; i++)
        {
            omo_add_file_to_queue(new_queue, app->queue->file[i]);
        }
    }
    omo_destroy_queue(app->queue);
    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        if(omo_get_player(&app->player_registry, al_get_native_file_dialog_path(fc, i)))
        {
            omo_add_file_to_queue(new_queue, al_get_native_file_dialog_path(fc, i));
        }
    }
    al_destroy_native_file_dialog(fc);
    app->queue = new_queue;
    return 1;

	fail:
	{
		if(fc)
		{
			al_destroy_native_file_dialog(fc);
		}
		return 0;
	}
    return 1;
}

static int file_count = 0;

static bool count_file(const char * fn, void * data)
{
    file_count++;
    return false;
}

static bool process_file(const char * fn, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(omo_get_player(&app->player_registry, fn))
    {
        if(omo_add_file_to_queue(app->queue, fn))
        {
            return true;
        }
    }
    return false;
}

int omo_menu_file_play_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;

    fc = al_create_native_file_dialog(app->last_music_filename, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER);
	if(!fc)
	{
		goto fail;
	}
	if(!al_show_native_file_dialog(al_get_current_display(), fc))
	{
		goto fail;
	}
	if(!al_get_native_file_dialog_count(fc))
	{
		goto fail;
	}
    omo_stop_file(data);
    if(app->queue)
    {
        omo_destroy_queue(app->queue);
    }
    file_count = 0;
    if(t3f_scan_files(al_get_native_file_dialog_path(fc, 0), count_file, false, data))
    {
        app->queue = omo_create_queue(file_count);
        if(app->queue)
        {
            if(!t3f_scan_files(al_get_native_file_dialog_path(fc, 0), process_file, false, data))
            {
                omo_destroy_queue(app->queue);
                goto fail;
            }
            app->queue_pos = -1;
        }
    }

    al_destroy_native_file_dialog(fc);
    return 1;

    fail:
    {
        if(fc)
		{
			al_destroy_native_file_dialog(fc);
		}
		return 0;
    }
    return 1;
}

int omo_menu_file_queue_folder(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;
    OMO_QUEUE * new_queue;
    OMO_QUEUE * old_queue;
    int i;

    fc = al_create_native_file_dialog(app->last_music_filename, "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER);
	if(!fc)
	{
		goto fail;
	}
	if(!al_show_native_file_dialog(al_get_current_display(), fc))
	{
		goto fail;
	}
	if(!al_get_native_file_dialog_count(fc))
	{
		goto fail;
	}
    file_count = 0;
    if(t3f_scan_files(al_get_native_file_dialog_path(fc, 0), count_file, false, data))
    {
        new_queue = omo_create_queue(app->queue->file_count + file_count);
        if(new_queue)
        {
            old_queue = app->queue;
            app->queue = new_queue;
            if(old_queue)
            {
                for(i = 0; i < old_queue->file_count; i++)
                {
                    omo_add_file_to_queue(app->queue, old_queue->file[i]);
                }
            }
            if(!t3f_scan_files(al_get_native_file_dialog_path(fc, 0), process_file, false, data))
            {
                omo_destroy_queue(app->queue);
                app->queue = old_queue;
                goto fail;
            }
            if(old_queue)
            {
                omo_destroy_queue(old_queue);
            }
        }
    }

    al_destroy_native_file_dialog(fc);
    return 1;

    fail:
    {
        if(fc)
		{
			al_destroy_native_file_dialog(fc);
		}
		return 0;
    }
    return 1;
}

int omo_menu_file_exit(void * data)
{
    t3f_exit();
    return 1;
}

int omo_menu_playback_play(void * data)
{
    omo_pause_file(data, false);
    return 1;
}

int omo_menu_playback_pause(void * data)
{
    omo_pause_file(data, true);
    return 1;
}
