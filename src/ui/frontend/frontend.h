#ifndef _OMO_FRONTEND_H
#define _OMO_FRONTEND_H

#include "t3f/t3f.h"

typedef struct
{

  char id[128];
  void * (*init)(void * app_instance, int flags);
  void (*exit)(void * data);

  void (*logic)(void * data, int flags);
  void (*render)(void * data, int flags);

  void * app;
  void * data;

} OMO_FRONTEND;

#endif
