#ifndef T3F_SOUND_H
#define T3F_SOUND_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>

#define T3F_MAX_QUEUED_SAMPLES 32

extern ALLEGRO_SAMPLE_ID t3f_sample_id;

void t3f_set_sound_volume(float volume);
float t3f_get_sound_volume(void);

bool t3f_play_sample(ALLEGRO_SAMPLE * sp, float vol, float pan, float speed);
bool t3f_queue_sample(ALLEGRO_SAMPLE * sp);
void t3f_clear_sample_queue(void);
ALLEGRO_SAMPLE * t3f_get_queue_sample(void);
void t3f_poll_sound_queue(void);
float t3f_get_sound_position(float earx, float eary, float soundx, float soundy);
float t3f_get_sound_gain(float earx, float eary, float soundx, float soundy, float scale);

#ifdef __cplusplus
	}
#endif

#endif
