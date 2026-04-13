#include "t3f.h"

T3F_OBJECT_LOADER * t3f_create_object_loader(void)
{
  T3F_OBJECT_LOADER * dp;

  dp = malloc(sizeof(T3F_OBJECT_LOADER));
  if(!dp)
  {
    goto fail;
  }
  memset(dp, 0, sizeof(T3F_OBJECT_LOADER));
  return dp;

  fail:
  {
    t3f_destroy_object_loader(dp);
    return NULL;
  }
}

void t3f_destroy_object_loader(T3F_OBJECT_LOADER * dp)
{
  if(dp)
  {
    t3f_stop_object_loader(dp);
    free(dp);
  }
}

void t3f_stop_object_loader(T3F_OBJECT_LOADER * dp)
{
  if(dp->loading_thread)
  {
    al_destroy_thread(dp->loading_thread);
    dp->loading_thread = NULL;
  }
  if(dp->loading_object && dp->dtor)
  {
    dp->dtor(dp->loading_object);
    dp->loading_object = NULL;
  }
  dp->ready = false;
}

static void * load_thread_proc(ALLEGRO_THREAD * tp, void * data)
{
	T3F_OBJECT_LOADER * dp = (T3F_OBJECT_LOADER *)data;

  al_set_fs_interface(dp->fs_interface);
  al_set_new_file_interface(dp->file_interface);
  dp->loading_object = dp->ctor(dp->arg);
  dp->ready = true;

	return NULL;
}

/* initiate object load, returns placeholder if loading from thread */
void * t3f_load_object(T3F_OBJECT_LOADER * dp, void * (*ctor)(void * data), void (*dtor)(void * data), void * arg, bool threaded)
{
  bool skip_thread = false;

  t3f_stop_object_loader(dp);
  dp->fs_interface = al_get_fs_interface();
  dp->file_interface = al_get_new_file_interface();
  dp->ctor = ctor;
  dp->dtor = dtor;
  dp->arg = arg;
  dp->ready = false;
  if(threaded)
  {
    /* get a placeholder we can return for immediate use */
    if(ctor)
    {
      dp->placeholder_object = ctor(NULL);
    }
		dp->loading_thread = al_create_thread(load_thread_proc, dp);
		if(dp->loading_thread)
		{
			al_start_thread(dp->loading_thread);
		}
		else
		{
			skip_thread = true;
		}
  }
  if(!threaded || skip_thread)
  {
    dp->ready = true;
    load_thread_proc(NULL, dp);
    dp->placeholder_object = dp->loading_object;
  }

  return dp->placeholder_object;
}

bool t3f_object_loading(T3F_OBJECT_LOADER * dp)
{
  if(dp->loading_thread)
  {
    return true;
  }
  return false;
}

/* if using threaded loading, call this to see if the object is ready */
bool t3f_object_ready(T3F_OBJECT_LOADER * dp)
{
  if(dp->ready)
  {
    return true;
  }
  return false;
}

/* fetch the object and cleanup */
void * t3f_fetch_object(T3F_OBJECT_LOADER * dp)
{
  void * ret;

  /* free placeholder if one was created during threaded load */
  if(dp->placeholder_object && dp->placeholder_object != dp->loading_object && dp->dtor)
  {
    dp->dtor(dp->placeholder_object);
  }

  /* grab loaded object pointer */
  ret = dp->loading_object;
  dp->loading_object = NULL;

  /* free object loader */
  t3f_stop_object_loader(dp);

  return ret;
}

