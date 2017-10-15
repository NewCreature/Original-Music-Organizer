#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "test.h"
#include "init.h"

static bool omo_test_mode;
static int omo_test_state;
static const char * omo_test_path;
static unsigned long omo_test_tick;
static int omo_test_count;
static int omo_test_player_state;

static void omo_test_delete_library_files(void)
{
    al_remove_filename(t3f_get_filename(t3f_data_path, "test_files.ini"));
    al_remove_filename(t3f_get_filename(t3f_data_path, "test_database.ini"));
}

bool omo_test_init(void * data, int mode, const char * path)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    omo_test_delete_library_files();
    omo_test_mode = mode;
    omo_test_path = path;
    omo_test_tick = 0;
    strcpy(app->test_path, path);
    omo_test_state = OMO_TEST_STATE_LIBRARY_LOAD;

    return true;
}

void omo_test_exit(void * data)
{
}

/* return false when finished */
bool omo_test_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_CONFIG * config;
    OMO_FILE_HELPER_DATA file_helper_data;

    switch(omo_test_state)
    {
        case OMO_TEST_STATE_LIBRARY_LOAD:
        {
            char file_database_fn[1024];
            char entry_database_fn[1024];

            printf("Preparing to load library.\n");
            strcpy(file_database_fn, t3f_get_filename(t3f_data_path, "test_files.ini"));
            strcpy(entry_database_fn, t3f_get_filename(t3f_data_path, "test_database.ini"));
            config = al_create_config();
            if(!config)
            {
                return false;
            }
            al_set_config_value(config, "Settings", "library_folders", "1");
            al_set_config_value(config, "Settings", "library_folder_0", app->test_path);
            omo_setup_library(app, file_database_fn, entry_database_fn, config);
            printf("Set test state: LIBRARY\n");
            omo_test_state = OMO_TEST_STATE_LIBRARY;
            break;
        }
        case OMO_TEST_STATE_LIBRARY:
        {
            if(app->library)
            {
                printf("Library loaded. Destroying library.\n");
                omo_destroy_library(app->library);
                app->library = NULL;
                printf("Set test state: QUEUE_LOAD\n");
                omo_test_state = OMO_TEST_STATE_QUEUE_LOAD;
            }
            break;
        }
        case OMO_TEST_STATE_QUEUE_LOAD:
        {
            omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path, NULL);

            printf("Counting files.\n");
			if(!t3f_scan_files(app->test_path, omo_count_file, false, NULL, &file_helper_data))
			{
                printf("Failed to count files.\n");
                return false;
			}
			if(file_helper_data.file_count > 0)
			{
                printf("Creating queue for %lu files.\n", file_helper_data.file_count);
				app->player->queue = omo_create_queue(file_helper_data.file_count);
                if(!app->player->queue)
                {
                    printf("Failed to create queue.\n");
                    return false;
                }
				file_helper_data.queue = app->player->queue;
                printf("Adding files to queue.\n");
				if(!t3f_scan_files(app->test_path, omo_queue_file, false, NULL, &file_helper_data))
				{
                    printf("Failed to add files to queue.\n");
				}
				if(app->player->queue->entry_count)
				{
                    printf("Getting tags for queued files.\n");
                    omo_get_queue_tags(app->player->queue, app->library, app);
				}
                else
                {
                    printf("No files in queue!\n");
                    return false;
                }
                printf("Set test state: QUEUE\n");
                omo_test_state = OMO_TEST_STATE_QUEUE;
			}
            break;
        }
        case OMO_TEST_STATE_QUEUE:
        {
            if(app->player->queue->thread_done)
            {
//                printf("Queue tags scanner thread finished, destroying queue.\n");
//                omo_destroy_queue(app->player->queue);
//                app->player->queue = NULL;
                printf("Set test state: PLAYER\n");
                omo_test_player_state = 0;
                omo_test_state = OMO_TEST_STATE_PLAYER;
            }
            break;
        }
        case OMO_TEST_STATE_PLAYER:
        {
            switch(omo_test_player_state)
            {
                case 0:
                {
                    printf("Player test: PLAY\n");
                    app->player->queue_pos = 0;
                    omo_start_player(app->player);
                    omo_test_player_state = 1;
                    break;
                }
                case 1:
                {
                    if(app->player->queue_pos > 0)
                    {
                        printf("Player test: SONG_FINISHED\n");
                        omo_test_tick = 0;
                        omo_test_count = 0;
                        omo_test_player_state = 2;
                        break;
                    }
                    break;
                }
                case 2:
                {
                    omo_test_tick++;
                    if(omo_test_tick % 300 == 0)
                    {
                        printf("Player test: NEXT_SONG\n");
                        omo_play_next_song(app->player);
                        omo_test_count++;
                        if(omo_test_count >= 5)
                        {
                            omo_test_tick = 0;
                            omo_test_count = 0;
                            omo_test_player_state = 3;
                        }
                    }
                    break;
                }
                case 3:
                {
                    omo_test_tick++;
                    if(omo_test_tick % 300 == 0)
                    {
                        printf("Player test: PREVIOUS_SONG\n");
                        omo_play_previous_song(app->player);
                        omo_test_count++;
                        if(omo_test_count >= 5)
                        {
                            omo_test_tick = 0;
                            omo_test_count = 0;
                            omo_test_player_state = 4;
                        }
                    }
                    break;
                }
                case 4:
                {
                    omo_test_tick++;
                    if(omo_test_tick == 300)
                    {
                        printf("Player test: PAUSE\n");
                        omo_pause_player(app->player);
                    }
                    else if(omo_test_tick == 600)
                    {
                        printf("Player test: RESUME\n");
                        omo_resume_player(app->player);
                    }
                    else if(omo_test_tick == 900)
                    {
                        printf("Player test: PAUSE\n");
                        omo_pause_player(app->player);
                    }
                    else if(omo_test_tick == 1200)
                    {
                        printf("Player test: PAUSE\n");
                        omo_pause_player(app->player);
                    }
                    else if(omo_test_tick == 1500)
                    {
                        printf("Player test: RESUME\n");
                        omo_resume_player(app->player);
                    }
                    else if(omo_test_tick == 1800)
                    {
                        printf("Player test: RESUME\n");
                        omo_resume_player(app->player);
                    }
                    else if(omo_test_tick == 2100)
                    {
                        printf("Player test: STOP\n");
                        omo_stop_player(app->player);
                    }
                    else if(omo_test_tick == 2400)
                    {
                        printf("Player test: PLAY\n");
                        app->player->queue_pos = 0;
                        omo_start_player(app->player);
                    }
                    else if(omo_test_tick == 2700)
                    {
                        printf("Player test: STOP\n");
                        omo_stop_player(app->player);
                    }
                    else if(omo_test_tick == 3000)
                    {
                        printf("Player test: STOP\n");
                        omo_stop_player(app->player);
                    }
                    else if(omo_test_tick == 3300)
                    {
                        printf("Player test: PLAY\n");
                        app->player->queue_pos = 0;
                        omo_start_player(app->player);
                    }
                    else if(omo_test_tick == 3600)
                    {
                        printf("Player test: PLAY\n");
                        omo_start_player(app->player);
                    }
                    else if(omo_test_tick == 3900)
                    {
                        printf("Player test: STOP\n");
                        omo_stop_player(app->player);
                        printf("Set test state: EXIT\n");
                        omo_test_state = OMO_TEST_STATE_EXIT;
                    }
                }
            }
            break;
        }
        case OMO_TEST_STATE_LIBRARY_AND_PLAYER:
        {
            break;
        }
        case OMO_TEST_STATE_QUEUE_AND_PLAYER:
        {
            break;
        }
        case OMO_TEST_STATE_ALL:
        {
            break;
        }
        case OMO_TEST_STATE_EXIT:
        {
            omo_test_delete_library_files();
            t3f_exit();
            break;
        }
    }
    return true;
}
