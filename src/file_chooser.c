#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "file_helpers.h"
#include "init.h"

static void file_chooser_thread_helper(void * data)
{
  APP_INSTANCE * app = (APP_INSTANCE *)data;

  if(al_show_native_file_dialog(al_get_current_display(), app->file_chooser))
  {
    app->file_chooser_done = true;
  }
  else
  {
    al_destroy_native_file_dialog(app->file_chooser);
    app->file_chooser = NULL;
  }
}

static void * file_chooser_thread_proc(ALLEGRO_THREAD * thread, void * arg)
{
    file_chooser_thread_helper(arg);
	return NULL;
}

bool omo_start_file_chooser(void * data, const char * title, const char * types, int mode, bool threaded)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    const char * last_music_filename = al_get_config_value(t3f_config, "App Settings", "last_music_filename");

    #ifdef ALLEGRO_WINDOWS
      if(mode == ALLEGRO_FILECHOOSER_FOLDER)
      {
        threaded = false;
      }
    #endif

    app->file_chooser = al_create_native_file_dialog(last_music_filename, title, types, mode);
    if(!app->file_chooser)
    {
        return false;
    }
    if(!threaded)
    {
      file_chooser_thread_helper(data);
    }
    else
    {
      app->file_chooser_thread = al_create_thread(file_chooser_thread_proc, data);
      if(!app->file_chooser_thread)
      {
        al_destroy_native_file_dialog(app->file_chooser);
        app->file_chooser = NULL;
        return false;
      }
      al_start_thread(app->file_chooser_thread);
    }
    return true;
}

static int omo_get_total_files(ALLEGRO_FILECHOOSER * fc, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_FILE_HELPER_DATA file_helper_data;
    int i;

    omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path);
    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        omo_count_file(al_get_native_file_dialog_path(fc, i), &file_helper_data);
    }
    return file_helper_data.file_count;
}

static void add_files_to_queue(ALLEGRO_FILECHOOSER * fc, OMO_QUEUE * queue, void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_FILE_HELPER_DATA file_helper_data;
    OMO_QUEUE * old_queue;
    int i;

    old_queue = app->player->queue;
    app->player->queue = queue;
    omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path);
    for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
    {
        omo_queue_file(al_get_native_file_dialog_path(fc, i), &file_helper_data);
    }
    app->player->queue = old_queue;
}

void omo_file_chooser_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
    OMO_FILE_HELPER_DATA file_helper_data;
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
                        omo_get_queue_tags(app->player->queue, app->library);
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
                        omo_get_queue_tags(app->player->queue, app->library);
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
                    omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path);
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, NULL, &file_helper_data))
                    {
                        app->player->queue = omo_create_queue(file_helper_data.file_count);
                        if(app->player->queue)
                        {
                            file_helper_data.queue = app->player->queue;
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, NULL, &file_helper_data))
                            {
                                omo_destroy_queue(app->player->queue);
                            }
                            else
                            {
                                omo_sort_queue(app->player->queue, app->library, 0, 0, app->player->queue->entry_count);
                                app->player->queue_pos = 0;
                                app->player->state = OMO_PLAYER_STATE_PLAYING;
                                omo_get_queue_tags(app->player->queue, app->library);
                            }
                        }
                    }
					break;
				}
				case 3:
				{
                    omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path);
                    if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, NULL, &file_helper_data))
                    {
                        new_queue = omo_create_queue((app->player->queue ? app->player->queue->entry_count : 0) + file_helper_data.file_count);
                        if(new_queue)
                        {
                            file_helper_data.queue = new_queue;
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
                            if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, NULL, &file_helper_data))
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
                                omo_get_queue_tags(app->player->queue, app->library);
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
                        omo_cancel_library_setup(app);
                        if(app->library)
                        {
                            omo_destroy_library(app->library);
                            app->library = NULL;
                        }
                        omo_setup_library(app);
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
