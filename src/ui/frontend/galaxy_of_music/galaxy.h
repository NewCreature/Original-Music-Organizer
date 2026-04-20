#ifndef _OMO_GALAXY_OF_MUSIC_GALAXY_H
#define _OMO_GALAXY_OF_MUSIC_GALAXY_H

#include "t3f/t3f.h"
#include "library.h"

/* properties of a body within the galaxy */
typedef struct
{

  char * id;
  char * name;
  int type;
  float size;
  float x, y, z;
  ALLEGRO_COLOR color;

} GOM_BODY;

typedef struct
{

  GOM_BODY body;

} GOM_MOON;

typedef struct
{

  /* planet properties */
  GOM_BODY body;

  /* orbiting bodies */
  GOM_MOON ** moon;
  int moon_count;

} GOM_PLANET;

typedef struct
{

  /* star properties */
  GOM_BODY body;

  /* orbiting bodies */
  GOM_PLANET ** planet;
  int planet_count;

} GOM_STAR;

typedef struct
{

  GOM_STAR ** star;
  int star_count;

} GOM_GALAXY;

GOM_GALAXY * gom_create_galaxy(OMO_LIBRARY * library);
void gom_destroy_galaxy(GOM_GALAXY * galaxy);

#endif