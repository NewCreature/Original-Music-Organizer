#include "../instance.h"
#include "dialog_proc.h"

static int old_artist_d1 = -1;
static int old_album_d1 = -1;

static void queue_song_list(void * data, OMO_LIBRARY * lp)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_QUEUE * new_queue;
	int i;

	new_queue = omo_create_queue(lp->song_entry_count);
	if(new_queue)
	{
		for(i = 0; i < lp->song_entry_count; i++)
		{
			omo_add_file_to_queue(new_queue, lp->entry[lp->song_entry[i]]->filename, lp->entry[lp->song_entry[i]]->sub_filename, lp->entry[lp->song_entry[i]]->track);
		}
		omo_get_queue_tags(new_queue, lp, app);
		switch(app->player->state)
		{
			case OMO_PLAYER_STATE_PLAYING:
			case OMO_PLAYER_STATE_PAUSED:
			{
				omo_stop_player(app->player);
				break;
			}
		}
		if(app->player->queue)
		{
			omo_destroy_queue(app->player->queue);
		}
		app->player->queue = new_queue;
		app->player->queue_pos = 0;
		omo_start_player(app->player);
	}
}

void omo_library_pre_gui_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->library_view)
    {
        old_artist_d1 = app->ui->ui_artist_list_element->d1;
        old_album_d1 = app->ui->ui_album_list_element->d1;
    }
}

void omo_library_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    const char * val;
    const char * val2;

    if(app->ui->ui_artist_list_element->d1 != old_artist_d1)
    {
        app->ui->ui_album_list_element->d1 = 0;
        app->ui->ui_song_list_element->d1 = 0;
        val2 = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
        omo_get_library_song_list(app->library, val2, "All Albums");
    }
    else if(app->ui->ui_album_list_element->d1 != old_album_d1)
    {
        app->ui->ui_song_list_element->d1 = 0;
        val = ui_artist_list_proc(app->ui->ui_artist_list_element->d1, NULL, app);
        val2 = ui_album_list_proc(app->ui->ui_album_list_element->d1, NULL, app);
        omo_get_library_song_list(app->library, val, val2);
    }
    if(app->ui->ui_artist_list_element->id1 >= 0)
    {
        queue_song_list(app, app->library);
        app->ui->ui_artist_list_element->id1 = -1;
    }
    else if(app->ui->ui_album_list_element->id1 >= 0)
    {
        queue_song_list(app, app->library);
        app->ui->ui_album_list_element->id1 = -1;
    }
    else if(app->ui->ui_song_list_element->id1 >= 0)
    {
        switch(app->player->state)
        {
            case OMO_PLAYER_STATE_PLAYING:
            case OMO_PLAYER_STATE_PAUSED:
            {
                omo_stop_player(app->player);
                break;
            }
        }
        if(app->player->queue)
        {
            omo_destroy_queue(app->player->queue);
        }
        app->player->queue = omo_create_queue(1);
        if(app->player->queue)
        {
            omo_add_file_to_queue(app->player->queue, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->filename, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->sub_filename, app->library->entry[app->library->song_entry[app->ui->ui_song_list_element->d1]]->track);
            app->player->queue_pos = 0;
            omo_start_player(app->player);
        }
        app->ui->ui_song_list_element->id1 = -1;
    }
}
