#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "../frontend.h"
#include "instance.h"
#include "galaxy_of_music.h"
#include "galaxy.h"

typedef struct
{

  APP_INSTANCE * app;
  T3F_BITMAP * bitmap[GOM_MAX_BITMAPS];
  GOM_GALAXY * galaxy;

} OMO_FRONTEND_DATA;

static bool _gom_load_data(OMO_FRONTEND_DATA * frontend)
{
  frontend->bitmap[GOM_BITMAP_STAR] = t3f_load_bitmap("data/galaxy_of_music/star.png", 0, false);
  if(!frontend->bitmap[GOM_BITMAP_STAR])
  {
    goto fail;
  }

  return true;

  fail:
  {
    return false;
  }
}

static void _frontend_exit(void * data)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;
  int i;

  if(frontend_data)
  {
    for(i = 0; i < GOM_MAX_BITMAPS; i++)
    {
      if(frontend_data->bitmap[i])
      {
        t3f_destroy_bitmap(frontend_data->bitmap[i]);
      }
    }
    free(frontend_data);
  }
}

static void * _frontend_init(void * app_instance, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = NULL;

  frontend_data = malloc(sizeof(OMO_FRONTEND_DATA));
  if(!frontend_data)
  {
    goto fail;
  }
  memset(frontend_data, 0, sizeof(OMO_FRONTEND_DATA));

  frontend_data->app = app_instance;

  t3f_set_gfx_mode(1280, 720, T3F_USE_FULLSCREEN);
  if(!_gom_load_data(frontend_data))
  {
    goto fail;
  }

  return frontend_data;

  fail:
  {
    _frontend_exit(frontend_data);
    return NULL;
  }
}

static void _frontend_logic(void * data, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;

  /* generate galaxy as soon as library is available */
  if(!frontend_data->galaxy)
  {
    if(frontend_data->app->library && frontend_data->app->library_thread && !frontend_data->app->cloud_thread)
    {
      frontend_data->galaxy = gom_create_galaxy(frontend_data->app->library);
    }
  }
  if(t3f_key_pressed(ALLEGRO_KEY_ESCAPE))
  {
    t3f_exit();
    t3f_use_key_press(ALLEGRO_KEY_ESCAPE);
  }
}

static void _frontend_render(void * data, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;

  if(frontend_data->galaxy)
  {
    al_clear_to_color(t3f_color_black);
  }
  else
  {
    al_clear_to_color(t3f_color_white);
  }
}

static OMO_FRONTEND _frontend;

OMO_FRONTEND * omo_get_galaxy_of_music_frontend(void * app, int flags)
{
	memset(&_frontend, 0, sizeof(OMO_FRONTEND));
	strcpy(_frontend.id, "Galaxy of Music");
	_frontend.init = _frontend_init;
	_frontend.exit = _frontend_exit;
  _frontend.logic = _frontend_logic;
  _frontend.render = _frontend_render;
  _frontend.data = _frontend.init(app, flags);
  if(!_frontend.data)
  {
    goto fail;
  }
  _frontend.app = app;
  return &_frontend;

  fail:
  {
    return NULL;
  }
}