#ifndef T3F_ACHIEVEMENTS_H
#define T3F_ACHIEVEMENTS_H

typedef struct
{

  char * steam_id;
  char * name;
  char * description;
  int steps;
  int step;
  bool hidden;

} T3F_ACHIEVEMENT_ENTRY;

typedef struct
{

  T3F_ACHIEVEMENT_ENTRY * entry;
  int entries;
  bool modified; // note that something in the list has been modified

} T3F_ACHIEVEMENTS_LIST;

T3F_ACHIEVEMENTS_LIST * t3f_create_achievements_list(int entries);
void t3f_destroy_achievements_list(T3F_ACHIEVEMENTS_LIST * achievements_list);
bool t3f_set_achievement_details(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, const char * steam_id, const char * name, const char * description, int steps, bool hidden);
void t3f_update_achievement_progress(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, int step);
bool t3f_achievement_gotten(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry);
void t3f_save_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);
void t3f_load_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);

#endif
