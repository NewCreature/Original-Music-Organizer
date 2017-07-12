#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "instance.h"
#include "player_registry.h"

bool omo_play_file(void * data, const char * fn)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    printf("play: %s\n", fn);
    app->player = omo_get_player(&app->player_registry, fn);
    if(app->player)
    {
        app->player->load_file(fn);
        app->player->play();
    }

    return false;
}

void omo_stop_file(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->player)
    {
        app->player->stop();
        app->player = NULL;
    }
}

void omo_pause_file(void * data, bool paused)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    if(app->player)
    {
        app->player->pause(paused);
    }
}

static char type_buf[1024] = {0};

static const char * omo_get_type_string(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    int i, j;

    strcpy(type_buf, "");
    for(i = 0; i < app->player_registry.players; i++)
    {
        for(j = 0; j < app->player_registry.player[i].types; j++)
        {
            strcat(type_buf, "*");
            strcat(type_buf, app->player_registry.player[i].type[j]);
            strcat(type_buf, ";");
        }
    }
    type_buf[strlen(type_buf) - 1] = '\0';
    return type_buf;
}

int omo_menu_file_open(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_FILECHOOSER * fc;
    int i;

	fc = al_create_native_file_dialog(app->last_music_filename, "Select music file.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE);
	if(!fc)
	{
		goto fail;
	}
	if(!al_show_native_file_dialog(al_get_current_display(), fc))
	{
		goto fail;
	}
	if(!al_get_native_file_dialog_count(fc))
	{
		goto fail;
	}
    omo_stop_file(data);
    if(app->queue)
    {
        omo_destroy_queue(app->queue);
    }
    app->queue = omo_create_queue(al_get_native_file_dialog_count(fc));
    if(app->queue)
    {
        for(i = 0; i < al_get_native_file_dialog_count(fc); i++)
        {
            omo_add_file_to_queue(app->queue, al_get_native_file_dialog_path(fc, i));
        }
        app->queue_pos = -1;
    }
    al_destroy_native_file_dialog(fc);
    return 1;

	fail:
	{
		if(fc)
		{
			al_destroy_native_file_dialog(fc);
		}
		return 0;
	}
    return 1;
}

int omo_menu_file_exit(void * data)
{
    t3f_exit();
    return 1;
}

int omo_menu_playback_play(void * data)
{
    omo_pause_file(data, false);
    return 1;
}

int omo_menu_playback_pause(void * data)
{
    omo_pause_file(data, true);
    return 1;
}
