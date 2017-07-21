#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "DUMBA5/dumba5.h"

#include "instance.h"
#include "archive_handler_registry.h"
#include "player_registry.h"
#include "queue.h"

bool omo_play_file(void * data, const char * fn, const char * subfn)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    printf("play: %s\n", fn);
    app->player = omo_get_player(app->player_registry, fn);
    if(app->player)
    {
        app->player->load_file(fn, subfn);
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
    for(i = 0; i < app->player_registry->players; i++)
    {
        for(j = 0; j < app->player_registry->player[i].types; j++)
        {
            strcat(type_buf, "*");
            strcat(type_buf, app->player_registry->player[i].type[j]);
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

static int omo_get_total_files(ALLEGRO_FILECHOOSER * fc, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler;
    OMO_PLAYER * player;
    int total_files = 0;
    int i, j, c;

    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        archive_handler = omo_get_archive_handler(app->archive_handler_registry, al_get_native_file_dialog_path(fc, i));
        if(archive_handler)
        {
            c = archive_handler->count_files(al_get_native_file_dialog_path(fc, i));
            for(j = 0; j < c; j++)
            {
                player = omo_get_player(app->player_registry, archive_handler->get_file(al_get_native_file_dialog_path(fc, i), j));
                if(player)
                {
                    total_files++;
                }
            }
        }
        else
        {
            player = omo_get_player(app->player_registry, al_get_native_file_dialog_path(fc, i));
            if(player)
            {
                total_files += player->get_track_count(al_get_native_file_dialog_path(fc, i));
            }
        }
    }
    return total_files;
}

static void add_files_to_queue(ALLEGRO_FILECHOOSER * fc, OMO_QUEUE * queue, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler = NULL;
    OMO_PLAYER * player;
    char buf[256];
    int i, j, c;

    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        /* add all archived files if we have an archive handler for this file type */
        archive_handler = omo_get_archive_handler(app->archive_handler_registry, al_get_native_file_dialog_path(fc, i));
        if(archive_handler)
        {
            c = archive_handler->count_files(al_get_native_file_dialog_path(fc, i));
            for(j = 0; j < c; j++)
            {
                player = omo_get_player(app->player_registry, archive_handler->get_file(al_get_native_file_dialog_path(fc, i), j));
                if(player)
                {
                    sprintf(buf, "%d", j);
                    omo_add_file_to_queue(queue, al_get_native_file_dialog_path(fc, i),  buf);
                }
            }
        }

        /* otherwise, add single file */
        else
        {
            player = omo_get_player(app->player_registry, al_get_native_file_dialog_path(fc, i));
            if(player)
            {
                c = player->get_track_count(al_get_native_file_dialog_path(fc, i));
                for(j = 0; j < c; j++)
                {
                    sprintf(buf, "%d", j);
                    omo_add_file_to_queue(queue, al_get_native_file_dialog_path(fc, i), c > 1 ? buf : NULL);
                }
            }
        }
    }
}

int omo_menu_file_play_files(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;
    int total_files = 0;

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
    total_files = omo_get_total_files(fc, data);
    app->queue = omo_create_queue(total_files);
    if(app->queue)
    {
        add_files_to_queue(fc, app->queue, data);
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
    int total_files = 0;
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
    total_files = omo_get_total_files(fc, data);
    new_queue = omo_create_queue(total_files + (app->queue ? app->queue->entry_count : 0));
    if(!new_queue)
    {
        goto fail;
    }
    if(app->queue)
    {
        for(i = 0; i < app->queue->entry_count; i++)
        {
            omo_add_file_to_queue(new_queue, app->queue->entry[i]->file, app->queue->entry[i]->sub_file);
        }
    }
    omo_destroy_queue(app->queue);
    add_files_to_queue(fc, new_queue, data);
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
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler;
    OMO_PLAYER * player;
    int i, c;

    archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
    if(archive_handler)
    {
        c = archive_handler->count_files(fn);
        for(i = 0; i < c; i++)
        {
            player = omo_get_player(app->player_registry, archive_handler->get_file(fn, i));
            if(player)
            {
                file_count++;
            }
        }
    }
    else
    {
        player = omo_get_player(app->player_registry, fn);
        if(player)
        {
            c = player->get_track_count(fn);
            for(i = 0; i < c; i++)
            {
                file_count++;
            }
        }
    }
    return false;
}

static bool process_file(const char * fn, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler;
    OMO_PLAYER * player;
    char buf[32] = {0};
    int i, c;

    archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
    if(archive_handler)
    {
        c = archive_handler->count_files(fn);
        for(i = 0; i < c; i++)
        {
            player = omo_get_player(app->player_registry, archive_handler->get_file(fn, i));
            if(player)
            {
                sprintf(buf, "%d", i);
                omo_add_file_to_queue(app->queue, fn, buf);
            }
        }
    }
    else
    {
        player = omo_get_player(app->player_registry, fn);
        if(player)
        {
            c = player->get_track_count(fn);
            for(i = 0; i < c; i++)
            {
                sprintf(buf, "%d", i);
                omo_add_file_to_queue(app->queue, fn, c > 1 ? buf : NULL);
            }
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
        new_queue = omo_create_queue(app->queue->entry_count + file_count);
        if(new_queue)
        {
            old_queue = app->queue;
            app->queue = new_queue;
            if(old_queue)
            {
                for(i = 0; i < old_queue->entry_count; i++)
                {
                    omo_add_file_to_queue(app->queue, old_queue->entry[i]->file, old_queue->entry[i]->sub_file);
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

int omo_menu_playback_shuffle(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_QUEUE * new_queue;
    int i, r;

    if(app->queue)
    {
        /* stop currently playing song */
        if(app->player)
    	{
    		app->player->stop();
    		app->player = NULL;
        }

        /* create new queue */
        new_queue = omo_create_queue(app->queue->entry_count);
        if(new_queue)
        {
            for(i = 0; i < new_queue->entry_size; i++)
            {
                r = t3f_rand(&app->rng_state) % app->queue->entry_count;
                omo_add_file_to_queue(new_queue, app->queue->entry[r]->file, app->queue->entry[r]->sub_file);
                omo_delete_queue_item(app->queue, r);
            }
            omo_destroy_queue(app->queue);
            app->queue = new_queue;
            app->queue_pos = -1;
        }
    }
    return 1;
}
