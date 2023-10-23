#include "t3f.h"

T3F_ACHIEVEMENTS_LIST * t3f_create_achievements_list(int entries)
{
  T3F_ACHIEVEMENTS_LIST * achievements_list = NULL;

  achievements_list = malloc(sizeof(T3F_ACHIEVEMENTS_LIST));
  if(!achievements_list)
  {
    goto fail;
  }
  memset(achievements_list, 0, sizeof(T3F_ACHIEVEMENTS_LIST));
  achievements_list->entry = malloc(sizeof(T3F_ACHIEVEMENT_ENTRY) * entries);
  if(!achievements_list->entry)
  {
    goto fail;
  }
  achievements_list->entries = entries;

  return achievements_list;

  fail:
  {
    if(achievements_list)
    {
      t3f_destroy_achievements_list(achievements_list);
    }
    return NULL;
  }
}

void t3f_destroy_achievements_list(T3F_ACHIEVEMENTS_LIST * achievements_list)
{
  int i;

  if(achievements_list)
  {
    if(achievements_list->entry)
    {
      for(i = 0; i < achievements_list->entries; i++)
      {
        if(achievements_list->entry[i].name)
        {
          free(achievements_list->entry[i].name);
        }
        if(achievements_list->entry[i].description)
        {
          free(achievements_list->entry[i].description);
        }
      }
      free(achievements_list->entry);
    }
    free(achievements_list);
  }
}

bool t3f_set_achievement_details(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, const char * steam_id, const char * name, const char * description, int steps, bool hidden)
{
  if(!name || !description)
  {
    return false;
  }
  if(steam_id)
  {
    achievements_list->entry[entry].steam_id = malloc(strlen(steam_id) + 1);
    if(!achievements_list->entry[entry].steam_id)
    {
      return false;
    }
    strcpy(achievements_list->entry[entry].steam_id, steam_id);
  }
  achievements_list->entry[entry].name = malloc(strlen(name) + 1);
  if(!achievements_list->entry[entry].name)
  {
    return false;
  }
  strcpy(achievements_list->entry[entry].name, name);
  achievements_list->entry[entry].description = malloc(strlen(description) + 1);
  if(!achievements_list->entry[entry].description)
  {
    return false;
  }
  strcpy(achievements_list->entry[entry].description, description);
  achievements_list->entry[entry].steps = steps;
  achievements_list->entry[entry].hidden = hidden;

  return true;
}

void t3f_update_achievement_progress(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, int step)
{
  achievements_list->entry[entry].step = step;
  achievements_list->modified = true;
}

bool t3f_achievement_gotten(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry)
{
  return achievements_list->entry[entry].step >= achievements_list->entry[entry].steps;
}

void t3f_save_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section)
{
  char buf[64];
  char buf2[64];
  int i;

  for(i = 0; i < achievements_list->entries; i++)
  {
    sprintf(buf, "entry_%d_step", i);
    sprintf(buf2, "%d", achievements_list->entry[i].step);
    al_set_config_value(config, section, buf, buf2);
  }
}

void t3f_load_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section)
{
  char buf[64];
  const char * val;
  int i;

  for(i = 0; i < achievements_list->entries; i++)
  {
    sprintf(buf, "entry_%d_step", i);
    val = al_get_config_value(config, section, buf);
    if(val)
    {
      achievements_list->entry[i].step = atoi(val);
    }
  }
}
