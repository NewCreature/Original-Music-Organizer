#ifndef T3F_MUSIC_H
#define T3F_MUSIC_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>

#define T3F_MUSIC_STATE_OFF           0
#define T3F_MUSIC_STATE_STARTING      1
#define T3F_MUSIC_STATE_PLAYING       2
#define T3F_MUSIC_STATE_PAUSED        3
#define T3F_MUSIC_STATE_TRACK_CHANGE  4
#define T3F_MUSIC_STATE_STOPPING      5

extern ALLEGRO_AUDIO_STREAM * t3f_stream;
extern int t3f_music_state;

bool t3f_play_music(const char * fn);
void t3f_stop_music(void);
void t3f_pause_music(void);
void t3f_resume_music(void);
void t3f_set_music_volume(float volume);
void t3f_set_new_music_volume(float volume);
float t3f_get_music_volume(void);
void t3f_fade_out_music(float speed);
int t3f_get_music_state(void);
void t3f_disable_music_looping(void);

#ifdef __cplusplus
	}
#endif

#endif
