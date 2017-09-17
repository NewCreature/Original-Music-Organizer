#include "../t3f/t3f.h"
#include "../instance.h"
#include "../file_chooser.h"
#include "../init.h"
#include "../constants.h"

static void omo_toggle_library_view(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * v_x;
	const char * v_y;
	const char * v_width;
	const char * v_height;
	int c_x, c_y, c_width, c_height, c_old_width;
	char buf[32] = {0};
	ALLEGRO_MONITOR_INFO monitor_info;

	t3gui_close_dialog(app->ui->ui_dialog);

	al_get_window_position(t3f_display, &c_x, &c_y);
	c_width = al_get_display_width(t3f_display);
	c_height = al_get_display_height(t3f_display);

	app->library_view = !app->library_view;
	if(app->library_view)
	{
		v_x = al_get_config_value(t3f_config, "Settings", "library_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "library_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "library_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "library_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "basic_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "basic_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "basic_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "basic_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "library");
	}
	else
	{
		v_x = al_get_config_value(t3f_config, "Settings", "basic_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "basic_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "basic_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "basic_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "library_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "library_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "library_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "library_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "basic");
	}
	if(v_x && v_y && v_width && v_height)
	{
		c_x = atoi(v_x);
		c_y = atoi(v_y);
		c_width = atoi(v_width);
		c_height = atoi(v_height);
	}
	else
	{
		al_get_monitor_info(0, &monitor_info);
		if(app->library_view)
		{
			c_old_width = c_width;
			c_width *= 4;
			if(c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_width = monitor_info.x2 - monitor_info.x1;
			}
			c_x -= c_width - c_old_width;
			if(c_x < 0)
			{
				c_x = 0;
			}
		}
		else
		{
			c_old_width = c_width;
			c_width /= 4;
			c_x += c_old_width - c_width;
			if(c_x + c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_x = monitor_info.x2 - c_width;
			}
		}
	}
	al_set_window_position(t3f_display, c_x, c_y);
	al_resize_display(t3f_display, c_width, c_height);
	omo_create_main_dialog(app->ui, app->library_view ? 1 : 0, c_width, c_height, app);
	t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
}

void omo_shortcut_logic(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    char fullfn[1024];
    const char * val2;
    int i, j;

    if(t3f_key[ALLEGRO_KEY_L])
    {
        omo_toggle_library_view(app);
        t3f_key[ALLEGRO_KEY_L] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_T] && app->library)
    {
		app->ui->tags_queue_entry = -1;
        if(app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
        {
            j = app->ui->ui_queue_list_element->d1;
            strcpy(fullfn, app->player->queue->entry[j]->file);
            if(app->player->queue->entry[j]->sub_file)
            {
                strcat(fullfn, "/");
                strcat(fullfn, app->player->queue->entry[j]->sub_file);
            }
            if(app->player->queue->entry[j]->track)
            {
                strcat(fullfn, ":");
                strcat(fullfn, app->player->queue->entry[j]->track);
            }
			app->ui->tags_queue_entry = app->ui->ui_queue_list_element->d1;
        }
        else if(app->ui->ui_song_list_element->flags & D_GOTFOCUS)
        {
            j = app->ui->ui_song_list_element->d1;
            strcpy(fullfn, app->library->entry[app->library->song_entry[j]]->filename);
            if(app->library->entry[app->library->song_entry[j]]->sub_filename)
            {
                strcat(fullfn, "/");
                strcat(fullfn, app->library->entry[app->library->song_entry[j]]->sub_filename);
            }
            if(app->library->entry[app->library->song_entry[j]]->track)
            {
                strcat(fullfn, ":");
                strcat(fullfn, app->library->entry[app->library->song_entry[j]]->track);
            }
        }
        app->ui->tags_entry = al_get_config_value(app->library->file_database, fullfn, "id");
        if(app->ui->tags_entry)
        {
            for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
            {
                strcpy(app->ui->tags_text[i], "");
                if(omo_tag_type[i])
                {
                    val2 = al_get_config_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
                    if(val2)
                    {
                        strcpy(app->ui->tags_text[i], val2);
                    }
                }
            }
            omo_open_tags_dialog(app->ui, app);
        }
        t3f_key[ALLEGRO_KEY_T] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_Z])
    {
        app->button_pressed = 0;
        t3f_key[ALLEGRO_KEY_Z] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_X])
    {
        if(app->player->queue_pos != app->ui->ui_queue_list_element->d1)
        {
            omo_stop_player(app->player);
            app->player->queue_pos = app->ui->ui_queue_list_element->d1;
            omo_start_player(app->player);
        }
    }
    if(t3f_key[ALLEGRO_KEY_C])
    {
        if(app->player->state == OMO_PLAYER_STATE_PLAYING)
        {
            omo_pause_player(app->player);
        }
        else if(app->player->state == OMO_PLAYER_STATE_PAUSED)
        {
            omo_resume_player(app->player);
        }
        t3f_key[ALLEGRO_KEY_C] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_V])
    {
        omo_stop_player(app->player);
        t3f_key[ALLEGRO_KEY_V] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_B])
    {
        app->button_pressed = 3;
        t3f_key[ALLEGRO_KEY_B] = 0;
    }
    if(t3f_key[ALLEGRO_KEY_DELETE])
    {
        if(app->player->queue && app->ui->ui_queue_list_element->d1 < app->player->queue->entry_count)
        {
            if(app->ui->ui_queue_list_element->d1 == app->player->queue_pos)
            {
                omo_stop_player(app->player);
            }
            omo_delete_queue_item(app->player->queue, app->ui->ui_queue_list_element->d1);
            if(app->player->queue_pos > app->ui->ui_queue_list_element->d1)
            {
                app->player->queue_pos--;
            }
            if(app->player->queue->entry_count > 0)
            {
                omo_start_player(app->player);
            }
            else
            {
                omo_destroy_queue(app->player->queue);
                app->player->queue = NULL;
            }
        }
        t3f_key[ALLEGRO_KEY_DELETE] = 0;

    }
}
