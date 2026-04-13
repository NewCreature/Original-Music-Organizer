#ifndef _T3F_LEADERBOARD_H
#define _T3F_LEADERBOARD_H

#include "t3f/t3f.h"
#include "t3net/t3net.h"
#include "leaderboard.h"

#define T3F_LEADERBOARD_FLAG_ASCEND 1

typedef struct
{

	char * name;
	unsigned long score;
	char * extra;

} T3F_LEADERBOARD_ENTRY;

typedef struct
{

	T3F_LEADERBOARD_ENTRY ** entry;
	int entry_size;
	int entries;

} T3F_LEADERBOARD_DATA;

typedef struct
{

  T3F_LEADERBOARD_DATA * data;
  T3F_OBJECT_LOADER * loader;

} T3F_LEADERBOARD;

/* setup functions */
bool t3f_initialize_leaderboards(const char * section, const char * title, const char * version, const char * user_key_url, const char * user_name_url, const char * update_url, const char * query_url);
void t3f_deinitialize_leaderboards(void);
void t3f_define_leaderboard_obfuscation(int offset, int prime_factor);
void t3f_enable_leaderboard_uploads(bool onoff);

/* low level functions */
void t3f_store_leaderboard_score(const char * section, const char * mode, const char * option, int flags, int score, const char * extra);
int t3f_retrieve_leaderboard_score(const char * section, const char * mode, const char * option);
const char * t3f_retrieve_leaderboard_extra(const char * section, const char * mode, const char * option);
const char * t3f_get_leaderboard_user_name(const char * section);
void t3f_set_leaderboard_user_name(const char * section, const char * name);
const char * t3f_get_leaderboard_user_key(const char * section);
bool t3f_submit_leaderboard_user_name(const char * section);
bool t3f_submit_leaderboard_score(const char * section, const char * mode, const char * option);

/* high level functions */
T3F_LEADERBOARD * t3f_get_leaderboard(const char * section, const char * mode, const char * option, int entries, int flags, bool threaded);
void t3f_destroy_leaderboard(T3F_LEADERBOARD * lp);
bool t3f_update_loaderboard(T3F_LEADERBOARD * lp);

#endif
