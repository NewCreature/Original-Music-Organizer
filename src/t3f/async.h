#ifndef T3F_ASYNC_H
#define T3F_ASYNC_H

#include <allegro5/allegro5.h>

typedef struct
{

  ALLEGRO_THREAD * loading_thread;
  const ALLEGRO_FS_INTERFACE * fs_interface;
  const ALLEGRO_FILE_INTERFACE * file_interface;
  void * (*ctor)(void * data);
  void (*dtor)(void * data);
  void * arg;
  void * loading_object;
  void * placeholder_object;
  bool ready;

} T3F_OBJECT_LOADER;

T3F_OBJECT_LOADER * t3f_create_object_loader(void);
void t3f_destroy_object_loader(T3F_OBJECT_LOADER * dp);

void t3f_stop_object_loader(T3F_OBJECT_LOADER * dp);

void * t3f_load_object(T3F_OBJECT_LOADER * dp, void * (*ctor)(void * data), void (*dtor)(void * data), void * arg, bool threaded);
bool t3f_object_loading(T3F_OBJECT_LOADER * dp);
bool t3f_object_ready(T3F_OBJECT_LOADER * dp);
void * t3f_fetch_object(T3F_OBJECT_LOADER * dp);

#endif