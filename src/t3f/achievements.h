#ifndef T3F_ACHIEVEMENTS_H
#define T3F_ACHIEVEMENTS_H

#define T3F_ACHIEVEMENTS_STATE_NONE     0 // initial state
#define T3F_ACHIEVEMENTS_STATE_UNSTORED 1 // unstored changes
#define T3F_ACHIEVEMENTS_STATE_STORING  2 // storing in progress
#define T3F_ACHIEVEMENTS_STATE_STORED   3 // storing successful
#define T3F_ACHIEVEMENTS_STATE_ERROR    4

typedef struct
{

  /* definition */
  char * steam_id;
  char * name;
  char * description;
  int steps;
  bool hidden;

  /* state */
  int step;
  int store_state; // for Steam integration

} T3F_ACHIEVEMENT_ENTRY;

typedef struct
{

  T3F_ACHIEVEMENT_ENTRY * entry;
  int entries;
  bool updated;
  int store_entry;

} T3F_ACHIEVEMENTS_LIST;

T3F_ACHIEVEMENTS_LIST * t3f_create_achievements_list(int entries);
void t3f_destroy_achievements_list(T3F_ACHIEVEMENTS_LIST * achievements_list);
bool t3f_set_achievement_details(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, const char * steam_id, const char * name, const char * description, int steps, bool hidden);
void t3f_update_achievement_progress(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, int step);
bool t3f_achievement_gotten(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry);
void t3f_save_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);
void t3f_load_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);

#endif
