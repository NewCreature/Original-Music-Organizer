#include "instance.h"
#include "test.h"
#include "init.h"

static bool omo_test_mode;
static int omo_test_state;
static const char * omo_test_path;
static unsigned long omo_test_tick;

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
                printf("Exiting test suite.\n");
                omo_test_state = OMO_TEST_STATE_EXIT;
            }
            break;
        }
        case OMO_TEST_STATE_QUEUE_LOAD:
        {
            break;
        }
        case OMO_TEST_STATE_QUEUE:
        {
            break;
        }
        case OMO_TEST_STATE_PLAYER:
        {
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
