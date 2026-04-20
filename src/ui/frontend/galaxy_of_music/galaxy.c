#include "t3f/t3f.h"
#include "galaxy.h"
#include "library.h"

GOM_GALAXY * gom_create_galaxy(OMO_LIBRARY * library)
{
  GOM_GALAXY * galaxy = NULL;

  galaxy = malloc(sizeof(GOM_GALAXY));
  if(!galaxy)
  {
    goto fail;
  }
  memset(galaxy, 0, sizeof(GOM_GALAXY));

  return galaxy;

  fail:
  {
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
          free(galaxy->star[i]);
        }
      }
      free(galaxy->star);
    }
    free(galaxy);
  }
}
