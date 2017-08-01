#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"

static int omo_get_total_files(ALLEGRO_FILECHOOSER * fc, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler;
    OMO_CODEC_HANDLER * player;
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
                player = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(al_get_native_file_dialog_path(fc, i), j));
                if(player)
                {
                    total_files++;
                }
            }
        }
        else
        {
            player = omo_get_codec_handler(app->codec_handler_registry, al_get_native_file_dialog_path(fc, i));
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
    OMO_CODEC_HANDLER * player;
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
                player = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(al_get_native_file_dialog_path(fc, i), j));
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
            player = omo_get_codec_handler(app->codec_handler_registry, al_get_native_file_dialog_path(fc, i));
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

static int file_count = 0;

static bool count_file(const char * fn, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_ARCHIVE_HANDLER * archive_handler;
    OMO_CODEC_HANDLER * player;
    int i, c;

    archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
    if(archive_handler)
    {
        c = archive_handler->count_files(fn);
        for(i = 0; i < c; i++)
        {
            player = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(fn, i));
            if(player)
            {
                file_count++;
            }
        }
    }
    else
    {
        player = omo_get_codec_handler(app->codec_handler_registry, fn);
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
    OMO_CODEC_HANDLER * player;
    char buf[32] = {0};
    int i, c;

    archive_handler = omo_get_archive_handler(app->archive_handler_registry, fn);
    if(archive_handler)
    {
        c = archive_handler->count_files(fn);
        for(i = 0; i < c; i++)
        {
            player = omo_get_codec_handler(app->codec_handler_registry, archive_handler->get_file(fn, i));
            if(player)
            {
                sprintf(buf, "%d", i);
                omo_add_file_to_queue(app->player->queue, fn, buf);
            }
        }
    }
    else
    {
        player = omo_get_codec_handler(app->codec_handler_registry, fn);
        if(player)
        {
            c = player->get_track_count(fn);
            for(i = 0; i < c; i++)
            {
                sprintf(buf, "%d", i);
                omo_add_file_to_queue(app->player->queue, fn, c > 1 ? buf : NULL);
            }
        }
    }
    return false;
}

void omo_file_chooser_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_QUEUE * new_queue;
    OMO_QUEUE * old_queue;
    int total_files = 0;
    int i;

	if(app->file_chooser && app->file_chooser_done)
	{
		if(al_get_native_file_dialog_count(app->file_chooser))
		{
			switch(app->file_chooser_mode)
			{
				case 0:
				{
					omo_stop_player(app->player);
				    if(app->player->queue)
				    {
				        omo_destroy_queue(app->player->queue);
				    }
				    total_files = omo_get_total_files(app->file_chooser, data);
				    app->player->queue = omo_create_queue(total_files);
				    if(app->player->queue)
				    {
				        add_files_to_queue(app->file_chooser, app->player->queue, data);
				        app->player->queue_pos = 0;
				        app->player->state = OMO_PLAYER_STATE_PLAYING;
				    }
					break;
				}
				case 1:
				{
                    total_files = omo_get_total_files(app->file_chooser, data);
                    new_queue = omo_create_queue(total_files + (app->player->queue ? app->player->queue->entry_count : 0));
                    if(new_queue)
                    {
                        if(app->player->queue)
                        {
                            for(i = 0; i < app->player->queue->entry_count; i++)
                            {
                                omo_add_file_to_queue(new_queue, app->player->queue->entry[i]->file,    app->player->queue->entry[i]->sub_file);
                            }
                            omo_destroy_queue(app->player->queue);
                        }
                        add_files_to_queue(app->file_chooser, new_queue, data);
                        app->player->queue = new_queue;
                    }
					break;
				}
				case 2:
				{
                    omo_stop_player(app->player);
                    if(app->player->queue)
                    {
                        omo_destroy_queue(app->player->queue);
                    }
                    file_count = 0;
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), count_file, false, data))
                    {
                        app->player->queue = omo_create_queue(file_count);
                        if(app->player->queue)
                        {
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), process_file, false, data))
                            {
                                omo_destroy_queue(app->player->queue);
                            }
                            else
                            {
                                app->player->queue_pos = 0;
                                app->player->state = OMO_PLAYER_STATE_PLAYING;
                            }
                        }
                    }
					break;
				}
				case 3:
				{
                    file_count = 0;
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), count_file, false, data))
                    {
                        new_queue = omo_create_queue(app->player->queue->entry_count + file_count);
                        if(new_queue)
                        {
                            old_queue = app->player->queue;
                            app->player->queue = new_queue;
                            if(old_queue)
                            {
                                for(i = 0; i < old_queue->entry_count; i++)
                                {
                                    omo_add_file_to_queue(app->player->queue, old_queue->entry[i]->file, old_queue->entry[i]->sub_file);
                                }
                            }
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), process_file, false, data))
                            {
                                omo_destroy_queue(app->player->queue);
                                app->player->queue = old_queue;
                            }
                            else
                            {
                                if(old_queue)
                                {
                                    omo_destroy_queue(old_queue);
                                }
                            }
                        }
                    }
					break;
				}
			}
		}

		/* clean up */
		al_destroy_native_file_dialog(app->file_chooser);
		app->file_chooser = false;
		app->file_chooser_done = false;
	}
}
