#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "file_chooser.h"
#include "file_helpers.h"
#include "library_helpers.h"
#include "queue_helpers.h"
#include "profile.h"
#include "ui/rebase_song_folder_dialog.h"

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

bool omo_start_file_chooser(void * data, const char * initial, const char * title, const char * types, int mode, bool threaded)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	#ifdef ALLEGRO_WINDOWS
	if(mode == ALLEGRO_FILECHOOSER_FOLDER)
	{
		threaded = false;
	}
	#endif

	app->file_chooser = al_create_native_file_dialog(initial, title, types, mode);
	if(!app->file_chooser)
	{
		return false;
	}
	if(!threaded)
	{
		al_stop_timer(t3f_timer);
		file_chooser_thread_helper(data);
		al_start_timer(t3f_timer);
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

	omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
	for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
	{
		omo_count_file(al_get_native_file_dialog_path(fc, i), false, &file_helper_data);
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
	omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
	for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
	{
		omo_queue_file(al_get_native_file_dialog_path(fc, i), false, &file_helper_data);
	}
	app->player->queue = old_queue;
}

void omo_file_chooser_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_FILE_HELPER_DATA file_helper_data;
	ALLEGRO_PATH * path;
	ALLEGRO_CONFIG * config = NULL;
	int total_files = 0;
	int old_queue_size = 0;
	const char * val;
	char buffer[64] = {0};
	int library_folders = 0;
	char section_buffer[1024];
	int d1[256];
	int d2[256];
	int d3[256];
	int i;

	if(app->file_chooser && app->file_chooser_done)
	{
		if(al_get_native_file_dialog_count(app->file_chooser))
		{
			al_set_config_value(t3f_config, "Settings", "last_music_filename", al_get_native_file_dialog_path(app->file_chooser, 0));
			switch(app->file_chooser_mode)
			{
				case OMO_FILE_CHOOSER_PLAY_FILES:
				{
					al_stop_timer(t3f_timer);
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
						app->spawn_queue_thread = true;
						app->player->queue_pos = 0;
						app->ui->ui_queue_list_element->d1 = 0;
						app->ui->ui_queue_list_element->d2 = 0;
						app->player->state = OMO_PLAYER_STATE_PLAYING;
					}
					al_start_timer(t3f_timer);
					break;
				}
				case OMO_FILE_CHOOSER_QUEUE_FILES:
				{
					al_stop_timer(t3f_timer);
					total_files = omo_get_total_files(app->file_chooser, data);
					if(app->player->queue)
					{
						old_queue_size = app->player->queue->entry_count;
						omo_resize_queue(&app->player->queue, total_files + app->player->queue->entry_count);
					}
					else
					{
						app->player->queue = omo_create_queue(total_files);
					}
					if(app->player->queue)
					{
						add_files_to_queue(app->file_chooser, app->player->queue, data);
						omo_sort_queue(app->player->queue, app->library, 0, old_queue_size, app->player->queue->entry_count - old_queue_size);
						app->spawn_queue_thread = true;
					}
					al_start_timer(t3f_timer);
					break;
				}
				case OMO_FILE_CHOOSER_PLAY_FOLDER:
				{
					al_stop_timer(t3f_timer);
					omo_stop_player(app->player);
					if(app->player->queue)
					{
						omo_destroy_queue(app->player->queue);
					}
					omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
					if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, &file_helper_data))
					{
						app->player->queue = omo_create_queue(file_helper_data.file_count);
						if(app->player->queue)
						{
							file_helper_data.queue = app->player->queue;
							if(!t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, &file_helper_data))
							{
								omo_destroy_queue(app->player->queue);
							}
							else
							{
								omo_sort_queue(app->player->queue, app->library, 0, 0, app->player->queue->entry_count);
								app->player->queue_pos = 0;
								app->ui->ui_queue_list_element->d1 = 0;
								app->ui->ui_queue_list_element->d2 = 0;
								app->player->state = OMO_PLAYER_STATE_PLAYING;
								app->spawn_queue_thread = true;
							}
						}
					}
					al_start_timer(t3f_timer);
					break;
				}
				case OMO_FILE_CHOOSER_QUEUE_FOLDER:
				{
					al_stop_timer(t3f_timer);
					omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
					if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_count_file, false, &file_helper_data))
					{
						if(app->player->queue)
						{
							old_queue_size = app->player->queue->entry_count;
							omo_resize_queue(&app->player->queue, file_helper_data.file_count + app->player->queue->entry_count);
						}
						else
						{
							app->player->queue = omo_create_queue(file_helper_data.file_count);
						}
						if(app->player->queue)
						{
							file_helper_data.queue = app->player->queue;
							if(t3f_scan_files(al_get_native_file_dialog_path(app->file_chooser, 0), omo_queue_file, false, &file_helper_data))
							{
								omo_sort_queue(app->player->queue, app->library, 0, old_queue_size, app->player->queue->entry_count - old_queue_size);
								app->spawn_queue_thread = true;
							}
						}
					}
					al_start_timer(t3f_timer);
					break;
				}
				case OMO_FILE_CHOOSER_ADD_LIBRARY_FOLDER:
				{
					al_stop_timer(t3f_timer);
					al_set_config_value(t3f_config, "Settings", "last_music_folder", al_get_native_file_dialog_path(app->file_chooser, 0));
					val = al_get_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), "library_folders");
					if(val)
					{
						library_folders = atoi(val);
					}
					for(i = 0; i < library_folders; i++)
					{
						sprintf(buffer, "library_folder_%d", i);
						val = al_get_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), buffer);
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
						al_set_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), buffer, al_get_native_file_dialog_path(app->file_chooser, 0));
						library_folders++;
						sprintf(buffer, "%d", library_folders);
						al_set_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), "library_folders", buffer);
						omo_clear_library_cache();
						app->spawn_library_thread = true;
					}
					al_start_timer(t3f_timer);
					break;
				}
				case OMO_FILE_CHOOSER_LOAD_THEME:
				{
					al_set_config_value(t3f_config, "Settings", "theme", al_get_native_file_dialog_path(app->file_chooser, 0));
					for(i = 0; i < app->ui->ui_dialog->elements; i++)
					{
						d1[i] = app->ui->ui_dialog->element[i].d1;
						d2[i] = app->ui->ui_dialog->element[i].d2;
						d3[i] = app->ui->ui_dialog->element[i].d3;
					}
					t3gui_close_dialog(app->ui->ui_dialog);
					omo_destroy_ui(app->ui);
					app->ui = omo_create_ui();
					if(app->ui)
					{
						app->library_view = false;
						val = al_get_config_value(t3f_config, "Settings", "last_view");
						if(val && !strcmp(val, "library"))
						{
							app->library_view = true;
						}
						if(!omo_create_main_dialog(app->ui, app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), app))
						{
							printf("Unable to create main dialog!\n");
						}
						for(i = 0; i < app->ui->ui_dialog->elements; i++)
						{
							app->ui->ui_dialog->element[i].d1 = d1[i];
							app->ui->ui_dialog->element[i].d2 = d2[i];
							app->ui->ui_dialog->element[i].d3 = d3[i];
						}
						for(i = 0; i < app->ui->ui_dialog->elements; i++)
						{
							app->ui->ui_dialog->element[i].id1 = -1;
							app->ui->ui_dialog->element[i].id2 = -1;
						}
						t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
					}
					break;
				}
				case OMO_FILE_CHOOSER_EXPORT_PLAYLIST:
				{
					path = al_create_path(al_get_native_file_dialog_path(app->file_chooser, 0));
					if(path)
					{
						al_set_path_extension(path, ".pls");
					}
					al_set_config_value(t3f_config, "Settings", "last_playlist_filename", al_path_cstr(path, '/'));
					omo_export_queue_to_playlist(app->player->queue, al_path_cstr(path, '/'));
					al_destroy_path(path);
					break;
				}
				case OMO_FILE_CHOOSER_IMPORT_FILE_DATABASE:
				{
					config = al_load_config_file(al_get_native_file_dialog_path(app->file_chooser, 0));
					if(config)
					{
						al_lock_mutex(app->library->file_database->mutex);
						al_merge_config_into(app->library->file_database->config, config);
						al_unlock_mutex(app->library->file_database->mutex);
						al_destroy_config(config);
					}
					break;
				}
				case OMO_FILE_CHOOSER_IMPORT_ENTRY_DATABASE:
				{
					config = al_load_config_file(al_get_native_file_dialog_path(app->file_chooser, 0));
					if(config)
					{
						al_lock_mutex(app->library->entry_database->mutex);
						al_merge_config_into(app->library->entry_database->config, config);
						al_lock_mutex(app->library->entry_database->mutex);
						al_destroy_config(config);
					}
					break;
				}
				case OMO_FILE_CHOOSER_REBASE_SONG_FOLDER:
				{
					al_set_config_value(t3f_config, "Settings", "base_path", al_get_native_file_dialog_path(app->file_chooser, 0));
					omo_open_rebase_song_folder_dialog(app->ui, data);
					break;
				}
			}
		}

		/* clean up */
		al_destroy_native_file_dialog(app->file_chooser);
		app->file_chooser = NULL;
		app->file_chooser_done = false;
	}
}
