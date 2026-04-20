#include <wctype.h>
#include <stdlib.h>
#include "t3f/t3f.h"
#include "galaxy.h"
#include "library.h"
#include "md5.h"

static int _extract_value_from_hash(const char * md5, int start, int length)
{
  char buf[256] = {0};
  int i;

  strcpy(buf, "0x");
  for(i = 0; i < length; i++)
  {
    buf[i + 2] = md5[start + i];
  }

  return strtol(buf, NULL, 16);
}

static bool _gom_add_star(OMO_LIBRARY * library, GOM_STAR ** star, int index)
{
  char buf[1024];
  int i, v;

  if(library->album_entry[index].name && strlen(library->album_entry[index].name))
  {
    star[index] = malloc(sizeof(GOM_STAR));
    if(!star[index])
    {
      goto fail;
    }
    memset(star[index], 0, sizeof(GOM_STAR));

    /* get album ID */
    strcpy(buf, library->album_entry[index].name);
    if(library->album_entry[index].disambiguation && strlen(library->album_entry[index].disambiguation))
    {
      strcat(buf, library->album_entry[index].disambiguation);
    }
    for(i = 0; i < strlen(buf); i++)
    {
      buf[i] = tolower(buf[i]);
    }
    star[index]->body.id = strdup(md5_string(buf));
    if(!star[index]->body.id)
    {
      goto fail;
    }
    star[index]->body.name = strdup(library->album_entry[index].name);
    if(!star[index]->body.name)
    {
      goto fail;
    }

    /* apply properties based on hash */
    v = _extract_value_from_hash(star[index]->body.id, 0, 4);
    star[index]->body.x = ((float)v / 65535.0) * 1280.0;
    v = _extract_value_from_hash(star[index]->body.id, 4, 4);
    star[index]->body.y = ((float)v / 65535.0) * 720.0;
    v = _extract_value_from_hash(star[index]->body.id, 8, 4);
    star[index]->body.z = ((float)v / 65535.0) * 1000.0;
    printf("star %s: (%4.2f, %4.2f)\n", star[index]->body.name, star[index]->body.x, star[index]->body.y);
  }

  return true;

  fail:
  {
    return false;
  }
}

static GOM_STAR ** _gom_generate_stars(OMO_LIBRARY * library)
{
  GOM_STAR ** star = NULL;
  int i;

  star = malloc(sizeof(GOM_STAR *) * library->album_entry_count);
  if(!star)
  {
    goto fail;
  }
  memset(star, 0, sizeof(GOM_STAR *) * library->album_entry_count);
  for(i = 0; i < library->album_entry_count; i++)
  {
    if(!_gom_add_star(library, star, i))
    {
      goto fail;
    }
  }

  return star;

  fail:
  {
    return NULL;
  }
}

GOM_GALAXY * gom_create_galaxy(OMO_LIBRARY * library)
{
  GOM_GALAXY * galaxy = NULL;
  int i;

  galaxy = malloc(sizeof(GOM_GALAXY));
  if(!galaxy)
  {
    goto fail;
  }
  memset(galaxy, 0, sizeof(GOM_GALAXY));

  galaxy->star = _gom_generate_stars(library);
  if(!galaxy->star)
  {
    goto fail;
  }
  galaxy->star_count = library->album_entry_count;

  return galaxy;

  fail:
  {
  printf("galaxy 4\n");
    gom_destroy_galaxy(galaxy);
    return NULL;
  }
}

void gom_destroy_galaxy(GOM_GALAXY * galaxy)
{
  int i, j, k;

  if(galaxy)
  {
    if(galaxy->star)
    {
      for(i = 0; i < galaxy->star_count; i++)
      {
        if(galaxy->star[i])
        {
          if(galaxy->star[i]->planet)
          {
            for(j = 0; j < galaxy->star[i]->planet_count; j++)
            {
              if(galaxy->star[i]->planet[j])
              {
                if(galaxy->star[i]->planet[j]->moon)
                {
                  for(k = 0; k < galaxy->star[i]->planet[j]->moon_count; k++)
                  {
                    if(galaxy->star[i]->planet[j]->moon[k])
                    {
                      free(galaxy->star[i]->planet[j]->moon[k]);
                    }
                  }
                  free(galaxy->star[i]->planet[j]->moon);
                }
                free(galaxy->star[i]->planet[j]);
              }
            }
            free(galaxy->star[i]->planet);
          }
          if(galaxy->star[i]->body.id)
          {
            free(galaxy->star[i]->body.id);
          }
          if(galaxy->star[i]->body.name)
          {
            free(galaxy->star[i]->body.name);
          }
          free(galaxy->star[i]);
        }
      }
      free(galaxy->star);
    }
    free(galaxy);
  }
}
