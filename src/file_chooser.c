#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "file_helpers.h"
#include "init.h"

static int omo_get_total_files(ALLEGRO_FILECHOOSER * fc, void * data)
{
    int i;

    omo_reset_file_count();
    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        omo_count_file(al_get_native_file_dialog_path(fc, i), data);
    }
    return omo_get_file_count();
}

static void add_files_to_queue(ALLEGRO_FILECHOOSER * fc, OMO_QUEUE * queue, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_QUEUE * old_queue;
    int i;

    old_queue = app->player->queue;
    app->player->queue = queue;
    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        omo_queue_file(al_get_native_file_dialog_path(fc, i), data);
    }
    app->player->queue = old_queue;
}

void omo_file_chooser_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_QUEUE * new_queue;
    OMO_QUEUE * old_queue;
    int total_files = 0;
    int old_queue_size = 0;
    const char * val;
    char buffer[64] = {0};
    int library_folders = 0;
    int i;

	if(app->file_chooser && app->file_chooser_done)
	{
		if(al_get_native_file_dialog_count(app->file_chooser))
		{
            al_set_config_value(t3f_config, "App Settings", "last_music_filename", al_get_native_file_dialog_path(app->file_chooser, 0));
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
                        omo_sort_queue(app->player->queue, app->library, 0, 0, app->player->queue->entry_count);
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
                            old_queue_size = app->player->queue->entry_count;
                            for(i = 0; i < app->player->queue->entry_count; i++)
                            {
                                omo_add_file_to_queue(new_queue, app->player->queue->entry[i]->file,    app->player->queue->entry[i]->sub_file, app->player->queue->entry[i]->track);
                            }
                            omo_destroy_queue(app->player->queue);
                        }
                        add_files_to_queue(app->file_chooser, new_queue, data);
                        app->player->queue = new_queue;
                        omo_sort_queue(app->player->queue, app->library, 0, old_queue_size, app->player->queue->entry_count - old_queue_size);
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
                    omo_reset_file_count();
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, NULL, data))
                    {
                        app->player->queue = omo_create_queue(omo_get_file_count());
                        if(app->player->queue)
                        {
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, NULL, data))
                            {
                                omo_destroy_queue(app->player->queue);
                            }
                            else
                            {
                                omo_sort_queue(app->player->queue, app->library, 0, 0, app->player->queue->entry_count);
                                app->player->queue_pos = 0;
                                app->player->state = OMO_PLAYER_STATE_PLAYING;
                            }
                        }
                    }
					break;
				}
				case 3:
				{
                    omo_reset_file_count();
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, NULL, data))
                    {
                        new_queue = omo_create_queue((app->player->queue ? app->player->queue->entry_count : 0) + omo_get_file_count());
                        if(new_queue)
                        {
                            old_queue_size = app->player->queue ? app->player->queue->entry_count : 0;
                            old_queue = app->player->queue;
                            app->player->queue = new_queue;
                            if(old_queue)
                            {
                                for(i = 0; i < old_queue->entry_count; i++)
                                {
                                    omo_add_file_to_queue(app->player->queue, old_queue->entry[i]->file, old_queue->entry[i]->sub_file, old_queue->entry[i]->track);
                                }
                            }
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, NULL, data))
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
                                omo_sort_queue(app->player->queue, app->library, 0, old_queue_size, app->player->queue->entry_count - old_queue_size);
                            }
                        }
                    }
					break;
				}
                case 4:
                {
                    al_set_config_value(t3f_config, "Settings", "last_music_folder", al_get_native_file_dialog_path(app->file_chooser, 0));
                    val = al_get_config_value(t3f_config, "Settings", "library_folders");
                    if(val)
                    {
                        library_folders = atoi(val);
                    }
                    for(i = 0; i < library_folders; i++)
                    {
                        sprintf(buffer, "library_folder_%d", i);
                        val = al_get_config_value(t3f_config, "Settings", buffer);
                        if(val)
                        {
                            if(!strcmp(val, al_get_native_file_dialog_path(app->file_chooser, 0)))
                            {
                                printf("dupe\n");
                                break;
                            }
                        }
                    }
                    if(i == library_folders)
                    {
                        sprintf(buffer, "library_folder_%d", library_folders);
                        al_set_config_value(t3f_config, "Settings", buffer, al_get_native_file_dialog_path(app->file_chooser, 0));
                        library_folders++;
                        sprintf(buffer, "%d", library_folders);
                        al_set_config_value(t3f_config, "Settings", "library_folders", buffer);
                        omo_destroy_library(app->library);
                        omo_setup_library(app, omo_library_setup_update_proc);
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
