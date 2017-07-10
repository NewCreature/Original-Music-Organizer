#include "t3f/t3f.h"
#include "DUMBA5/dumba5.h"

#include "instance.h"

bool omo_play_file(void * data, const char * fn)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_PATH * path;
    const char * extension;
    int i, j;

    path = al_create_path(fn);
    if(path)
    {
        extension = al_get_path_extension(path);
        for(i = 0; i < app->player_registry.players; i++)
        {
            for(j = 0; j < app->player_registry.player[i].types; j++)
            {
                if(!strcasecmp(extension, app->player_registry.player[i].type[j]))
                {
                    app->player_registry.player[i].play_file(fn);
                    al_destroy_path(path);
                    return true;
                }
            }
        }
        al_destroy_path(path);
    }
    return false;
}

void omo_stop_file(void * data, const char * fn)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_PATH * path;
    const char * extension;
    int i, j;

    path = al_create_path(fn);
    if(path)
    {
        extension = al_get_path_extension(path);
        for(i = 0; i < app->player_registry.players; i++)
        {
            for(j = 0; j < app->player_registry.player[i].types; j++)
            {
                if(!strcasecmp(extension, app->player_registry.player[i].type[j]))
                {
                    app->player_registry.player[i].stop();
                    al_destroy_path(path);
                    return;
                }
            }
        }
        al_destroy_path(path);
    }
}

void omo_pause_file(void * data, const char * fn, bool paused)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;
    ALLEGRO_PATH * path;
    const char * extension;
    int i, j;

    path = al_create_path(fn);
    if(path)
    {
        extension = al_get_path_extension(path);
        for(i = 0; i < app->player_registry.players; i++)
        {
            for(j = 0; j < app->player_registry.player[i].types; j++)
            {
                if(!strcasecmp(extension, app->player_registry.player[i].type[j]))
                {
                    app->player_registry.player[i].pause(paused);
                    al_destroy_path(path);
                    return;
                }
            }
        }
        al_destroy_path(path);
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
    ALLEGRO_PATH * path;

	fc = al_create_native_file_dialog(app->last_music_filename, "Select music file.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
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
    omo_stop_file(data, app->last_music_filename);
    path = al_create_path(al_get_native_file_dialog_path(fc, 0));
    if(path)
    {
        if(omo_play_file(data, al_path_cstr(path, '/')))
        {
            strcpy(app->last_music_filename, al_path_cstr(path, '/'));
            al_set_window_title(t3f_display, al_get_path_filename(path));
        }
        al_destroy_path(path);
        al_destroy_native_file_dialog(fc);
        return 1;
    }

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
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    omo_pause_file(data, app->last_music_filename, false);
    return 1;
}

int omo_menu_playback_pause(void * data)
{
    APP_INSTANCE * app = (APP_INSTANCE *)data;

    omo_pause_file(data, app->last_music_filename, true);
    return 1;
}
