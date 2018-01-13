#include "instance.h"
#include "library_helpers.h"
#include "queue_helpers.h"
#include "cloud.h"

static void kill_all_threads(APP_INSTANCE * app)
{
	if(app->player->queue && app->player->queue->thread)
	{
		al_destroy_thread(app->player->queue->thread);
		app->player->queue->thread = NULL;
	}
	if(app->cloud_thread)
	{
		al_destroy_thread(app->cloud_thread);
		app->cloud_thread = NULL;
	}
	omo_cancel_library_setup(app);
}

void omo_threads_logic(APP_INSTANCE * app)
{
	char file_database_fn[1024];
	char entry_database_fn[1024];

	if(app->spawn_library_thread)
	{
		kill_all_threads(app);
		strcpy(file_database_fn, t3f_get_filename(t3f_data_path, "files.ini"));
		strcpy(entry_database_fn, t3f_get_filename(t3f_data_path, "database.ini"));
		if(app->library)
		{
			omo_destroy_library(app->library);
			app->library = NULL;
		}
		omo_setup_library(app, file_database_fn, entry_database_fn, NULL);
		app->spawn_library_thread = false;
	}
	if(app->spawn_library_lists_thread)
	{
		omo_setup_library_lists(app);
		app->spawn_library_lists_thread = false;
	}
	if(app->spawn_cloud_thread)
	{
		omo_submit_library_tags(app, "http://www.t3-i.com/omo/tag_track.php");
		app->spawn_cloud_thread = false;
	}
	if(app->spawn_queue_thread)
	{
		omo_get_queue_tags(app->player->queue, app->library, app);
		app->spawn_queue_thread = false;
	}
}
