#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"
#include "test.h"
#include "init.h"
#include "queue_helpers.h"
#include "library_helpers.h"

static bool omo_test_mode;
static int omo_test_state;
static const char * omo_test_path;
static unsigned long omo_test_tick;
static int omo_test_count;
static int omo_test_player_count;
static int omo_test_player_state;
static ALLEGRO_CONFIG * test_config;

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
	if(omo_test_mode == 0)
	{
		printf("Set test state: LIBRARY_LOAD\n");
		omo_test_state = OMO_TEST_STATE_LIBRARY_LOAD;
	}
	else
	{
		printf("Set test state: ALL\n");
		omo_test_state = OMO_TEST_STATE_ALL;
	}

	return true;
}

void omo_test_exit(void * data)
{
}

static bool omo_test_setup_library(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char file_database_fn[1024];
	char entry_database_fn[1024];

	strcpy(file_database_fn, t3f_get_filename(t3f_data_path, "test_files.ini"));
	strcpy(entry_database_fn, t3f_get_filename(t3f_data_path, "test_database.ini"));
	test_config = al_create_config();
	if(!test_config)
	{
		return false;
	}
	al_set_config_value(test_config, "Settings", "library_folders", "1");
	al_set_config_value(test_config, "Settings", "library_folder_0", app->test_path);
	omo_setup_library(app, file_database_fn, entry_database_fn, test_config);
	return true;
}

static bool omo_test_player_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

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
				omo_test_player_count = 0;
				omo_test_player_state = 2;
				break;
			}
			break;
		}
		case 2:
		{
			omo_test_tick++;
			if(omo_test_tick % OMO_TEST_INTERVAL == 0)
			{
				printf("Player test: NEXT_SONG\n");
				omo_play_next_song(app->player);
				omo_test_player_count++;
				if(omo_test_player_count >= 5)
				{
					omo_test_tick = 0;
					omo_test_player_count = 0;
					omo_test_player_state = 3;
				}
			}
			break;
		}
		case 3:
		{
			omo_test_tick++;
			if(omo_test_tick % OMO_TEST_INTERVAL == 0)
			{
				printf("Player test: PREVIOUS_SONG\n");
				omo_play_previous_song(app->player);
				omo_test_player_count++;
				if(omo_test_player_count >= 5)
				{
					omo_test_tick = 0;
					omo_test_player_count = 0;
					omo_test_player_state = 4;
				}
			}
			break;
		}
		case 4:
		{
			omo_test_tick++;
			if(omo_test_tick == OMO_TEST_INTERVAL)
			{
				printf("Player test: PAUSE\n");
				omo_pause_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 2)
			{
				printf("Player test: RESUME\n");
				omo_resume_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 3)
			{
				printf("Player test: PAUSE\n");
				omo_pause_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 4)
			{
				printf("Player test: PAUSE\n");
				omo_pause_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 5)
			{
				printf("Player test: RESUME\n");
				omo_resume_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 6)
			{
				printf("Player test: RESUME\n");
				omo_resume_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 7)
			{
				printf("Player test: STOP\n");
				omo_stop_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 8)
			{
				printf("Player test: PLAY\n");
				app->player->queue_pos = 0;
				omo_start_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 9)
			{
				printf("Player test: STOP\n");
				omo_stop_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 10)
			{
				printf("Player test: STOP\n");
				omo_stop_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 11)
			{
				printf("Player test: PLAY\n");
				app->player->queue_pos = 0;
				omo_start_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 12)
			{
				printf("Player test: PLAY\n");
				omo_start_player(app->player);
			}
			else if(omo_test_tick == OMO_TEST_INTERVAL * 13)
			{
				printf("Player test: STOP\n");
				omo_stop_player(app->player);
				return false;
			}
		}
	}
	return true;
}

static bool omo_test_setup_queue(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_FILE_HELPER_DATA file_helper_data;

	omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, app->library, app->player->queue, app->queue_temp_path, NULL);

	printf("Counting files.\n");
	if(!t3f_scan_files(app->test_path, omo_count_file, false, &file_helper_data))
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
		if(!t3f_scan_files(app->test_path, omo_queue_file, false, &file_helper_data))
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
	}
	return true;
}

/* return false when finished */
bool omo_test_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(omo_test_state)
	{
		case OMO_TEST_STATE_LIBRARY_LOAD:
		{
			printf("Preparing to load library.\n");
			if(!omo_test_setup_library(app))
			{
				return false;
			}
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
				al_destroy_config(test_config);
				omo_test_tick = 0;
				printf("Set test state: QUEUE\n");
				omo_test_state = OMO_TEST_STATE_QUEUE;
			}
			break;
		}
		case OMO_TEST_STATE_QUEUE:
		{
			if(omo_test_tick == 0)
			{
				if(!omo_test_setup_queue(app))
				{
					return false;
				}
				omo_test_tick++;
			}
			else
			{
				if(app->player->queue->thread_done)
				{
					printf("Set test state: PLAYER\n");
					omo_test_player_state = 0;
					omo_test_player_count = 0;
					omo_test_tick = 0;
					omo_test_state = OMO_TEST_STATE_PLAYER;
				}
			}
			break;
		}
		case OMO_TEST_STATE_PLAYER:
		{
			if(!omo_test_player_logic(app))
			{
				omo_test_count = 0;
				printf("Set test state: LIBRARY_AND_PLAYER\n");
				omo_test_state = OMO_TEST_STATE_LIBRARY_AND_PLAYER;
			}
			break;
		}
		case OMO_TEST_STATE_LIBRARY_AND_PLAYER:
		{
			if(omo_test_count == 0)
			{
				omo_test_count++;
				printf("Setting up library.\n");
				if(!omo_test_setup_library(app))
				{
					return false;
				}
				omo_test_player_state = 0;
				omo_test_player_count = 0;
				omo_test_tick = 0;
			}
			else
			{
				if(app->library)
				{
					if(!app->player->queue)
					{
						if(!omo_test_setup_queue(app))
						{
							return false;
						}
					}
					if(!omo_test_player_logic(app))
					{
						omo_destroy_queue(app->player->queue);
						app->player->queue = NULL;
						omo_destroy_library(app->library);
						app->library = NULL;
						omo_test_count = 0;
						printf("Set test state: QUEUE_AND_PLAYER\n");
						omo_test_state = OMO_TEST_STATE_QUEUE_AND_PLAYER;
					}
				}
			}
			break;
		}
		case OMO_TEST_STATE_QUEUE_AND_PLAYER:
		{
			if(omo_test_count == 0)
			{
				printf("Setting up queue.\n");
				if(!omo_test_setup_queue(app))
				{
					return false;
				}
				omo_test_player_state = 0;
				omo_test_player_count = 0;
				omo_test_tick = 0;
				omo_test_count++;
			}
			else
			{
				if(!omo_test_player_logic(app))
				{
					omo_destroy_queue(app->player->queue);
					app->player->queue = NULL;
					omo_test_count = 0;
					printf("Set test state: ALL\n");
					omo_test_state = OMO_TEST_STATE_ALL;
				}
			}
			break;
		}
		case OMO_TEST_STATE_ALL:
		{
			if(omo_test_count == 0)
			{
				printf("Setting up library.\n");
				omo_test_delete_library_files();
				if(!omo_test_setup_library(app))
				{
					return false;
				}
				printf("Setting up queue.\n");
				if(!omo_test_setup_queue(app))
				{
					return false;
				}
				omo_test_player_state = 0;
				omo_test_player_count = 0;
				omo_test_tick = 0;
				omo_test_count++;
			}
			else
			{
				if(!omo_test_player_logic(app))
				{
					printf("Set test state: EXIT\n");
					omo_test_state = OMO_TEST_STATE_EXIT;
				}
			}
			break;
		}
		case OMO_TEST_STATE_EXIT:
		{
			printf("Stopping library scan.\n");
			omo_cancel_library_setup(app);
			if(app->library)
			{
				printf("Destroying library.\n");
				omo_destroy_library(app->library);
				app->library = NULL;
			}
			printf("Destroying queue.\n");
			omo_destroy_queue(app->player->queue);
			app->player->queue = NULL;
			printf("Testing complete. Exiting.\n");
			omo_test_delete_library_files();
			t3f_exit();
			break;
		}
	}
	return true;
}
