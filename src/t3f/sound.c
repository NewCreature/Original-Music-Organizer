#include <math.h>
#include <stdio.h>
#include "t3f.h"
#include "sound.h"

static float t3f_sound_volume = 1.0;
static ALLEGRO_SAMPLE * t3f_sample_queue[T3F_MAX_QUEUED_SAMPLES] = {NULL};
static int t3f_queued_samples = 0;
static ALLEGRO_SAMPLE_INSTANCE * t3f_queue_sample_instance = NULL;
ALLEGRO_SAMPLE_ID t3f_sample_id;

void t3f_set_sound_volume(float volume)
{
	t3f_sound_volume = volume;
}

float t3f_get_sound_volume(void)
{
	return t3f_sound_volume;
}

bool t3f_play_sample(ALLEGRO_SAMPLE * sp, float vol, float pan, float speed)
{
	if(t3f_flags & T3F_USE_SOUND)
	{
		return al_play_sample(sp, t3f_sound_volume * vol, pan, speed, ALLEGRO_PLAYMODE_ONCE, &t3f_sample_id);
	}
	return false;
}

bool t3f_queue_sample(ALLEGRO_SAMPLE * sp)
{
	if((t3f_flags & T3F_USE_SOUND) && t3f_queued_samples < T3F_MAX_QUEUED_SAMPLES && sp)
	{
		t3f_sample_queue[t3f_queued_samples] = sp;
		t3f_queued_samples++;
		return true;
	}
	return false;
}

void t3f_clear_sample_queue(void)
{
	int i;

	if(t3f_flags & T3F_USE_SOUND)
	{
		for(i = 0; i < T3F_MAX_QUEUED_SAMPLES; i++)
		{
			t3f_sample_queue[i] = NULL;
		}
		t3f_queued_samples = 0;
		if(al_get_sample_instance_playing(t3f_queue_sample_instance))
		{
			al_stop_sample_instance(t3f_queue_sample_instance);
			al_destroy_sample_instance(t3f_queue_sample_instance);
			t3f_queue_sample_instance = NULL;
		}
	}
}

ALLEGRO_SAMPLE * t3f_get_queue_sample(void)
{
	return NULL;
}

static void t3f_play_queued_sample(void)
{
	int i;

	if(t3f_sample_queue[0])
	{
		t3f_queue_sample_instance = al_create_sample_instance(t3f_sample_queue[0]);
		al_set_sample_instance_gain(t3f_queue_sample_instance, t3f_sound_volume);
		al_set_sample_instance_speed(t3f_queue_sample_instance, 1.0);
		al_set_sample_instance_pan(t3f_queue_sample_instance, 0.0);
		al_set_sample_instance_playmode(t3f_queue_sample_instance, ALLEGRO_PLAYMODE_ONCE);
		al_attach_sample_instance_to_mixer(t3f_queue_sample_instance, al_get_default_mixer());
		al_play_sample_instance(t3f_queue_sample_instance);
		for(i = 0; i < t3f_queued_samples - 1; i++)
		{
			t3f_sample_queue[i] = t3f_sample_queue[i + 1];
		}
		t3f_sample_queue[i] = NULL;
		t3f_queued_samples--;
	}
}

void t3f_poll_sound_queue(void)
{
	if(t3f_flags & T3F_USE_SOUND)
	{
		/* a queued sample is playing */
		if(t3f_queue_sample_instance)
		{
			if(!al_get_sample_instance_playing(t3f_queue_sample_instance))
			{
				al_destroy_sample_instance(t3f_queue_sample_instance);
				t3f_queue_sample_instance = NULL;
			}
		}
		if(!t3f_queue_sample_instance)
		{
			t3f_play_queued_sample();
		}
	}
}

float t3f_get_sound_position(float earx, float eary, float soundx, float soundy)
{
	return -cos(atan2(eary - soundy, earx - soundx));
}

float t3f_get_sound_gain(float earx, float eary, float soundx, float soundy, float scale)
{
	float distance;

	distance = hypot(earx - soundx, eary - soundy);

	/* sound is out of hearing range */
	if(distance > scale)
	{
		return 0.0;
	}
	return t3f_sound_volume * (1.0 - distance / scale);
}
