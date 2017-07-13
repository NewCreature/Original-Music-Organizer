/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * dumba5.c - Functions to play a DUH through         / / \  \
 *            Allegro 5's audio system.              | <  /   \_
 *                                                   |  \/ /\   /
 * By Todd Cope.                                      \_  /  > /
 *                                                      | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>

#include "dumba5.h"



#define ADP_PLAYING 1

struct DUMBA5_PLAYER
{
	int flags;
	long bufsize;
	int freq;
	int channels;
	ALLEGRO_AUDIO_STREAM *stream;
	DUH * duh;
	DUH_SIGRENDERER *sigrenderer; /* If this is NULL, stream is invalid. */
	float volume;
	int silentcount;
	bool done_playing;

	ALLEGRO_THREAD * thread;
	ALLEGRO_MUTEX * mutex;
};

/* these are used internally by the simple interface */
static DUH * dumba5_duh = NULL;
static DUMBA5_PLAYER * dumba5_player = NULL;

/* DUMB_RQ_ALIASING, DUMB_RQ_LINEAR, DUMB_RQ_CUBIC
You have to call this function before you can use the rest. */
bool dumba5_init(int resampler)
{
	dumb_resampling_quality = resampler;
	atexit(&dumb_exit);
	dumb_register_stdfiles();
	return true;
}

void * dumba5_update_thread(ALLEGRO_THREAD * thread, void * arg)
{
	DUMBA5_PLAYER * dp = (DUMBA5_PLAYER *)arg;
	ALLEGRO_EVENT_QUEUE * queue;
	unsigned short *fragment;
	long n;
	long size;
	int n_channels;

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_audio_stream_event_source(dp->stream));

	while(1)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (unsigned short *)al_get_audio_stream_fragment(dp->stream);
			if(!fragment)
			{
				return NULL;
			}
			n = duh_render(dp->sigrenderer, 16, 0, dp->volume, 65536.0 / dp->freq, dp->bufsize, fragment);

			if (n == 0)
			{
				if (++dp->silentcount >= 2)
				{
					duh_end_sigrenderer(dp->sigrenderer);
					if(!al_set_audio_stream_fragment(dp->stream, fragment))
					{
					}
					al_destroy_audio_stream(dp->stream);
					dp->sigrenderer = NULL;
					return NULL;
				}
			}

			n_channels = duh_sigrenderer_get_n_channels(dp->sigrenderer);
			n *= n_channels;
			size = dp->bufsize * n_channels;
			for (; n < size; n++)
			{
				fragment[n] = 0x8000;
			}
			if(!al_set_audio_stream_fragment(dp->stream, fragment))
			{
			}
		}
		if(al_get_thread_should_stop(thread))
		{
			break;
		}
	}

	al_destroy_event_queue(queue);

	return NULL;
}

DUMBA5_PLAYER * dumba5_start_duh_x(DUH *duh, int n_channels, long pos, float volume, long bufsize, int freq)
{
	DUMBA5_PLAYER * dp;
	ALLEGRO_CHANNEL_CONF c_conf;

	/* This restriction is imposed by Allegro. */
	ASSERT(n_channels > 0);
	ASSERT(n_channels <= 2);

	if(!duh)
	{
		return NULL;
	}

	dp = (DUMBA5_PLAYER *) malloc(sizeof(DUMBA5_PLAYER));
	if(!dp)
	{
		return NULL;
	}

	dp->flags = ADP_PLAYING;
	dp->bufsize = bufsize;
	dp->freq = freq;
	dp->channels = n_channels;
	if(n_channels == 1)
	{
		c_conf = ALLEGRO_CHANNEL_CONF_1;
	}
	else
	{
		c_conf = ALLEGRO_CHANNEL_CONF_2;
	}

	dp->stream = al_create_audio_stream(4, bufsize, freq, ALLEGRO_AUDIO_DEPTH_INT16, c_conf);

	if (!dp->stream) {
		free(dp);
		return NULL;
	}
	al_attach_audio_stream_to_mixer(dp->stream, al_get_default_mixer());

	dp->sigrenderer = dumb_it_start_at_order(duh, n_channels, pos);

	if (!dp->sigrenderer) {
		al_destroy_audio_stream(dp->stream);
		free(dp);
		return NULL;
	}
	dp->thread = al_create_thread(dumba5_update_thread, dp);
	if(!dp->thread)
	{
		return NULL;
	}

	dp->volume = volume;
	dp->silentcount = 0;
	dp->done_playing = false;
	dp->duh = duh;
	al_start_thread(dp->thread);

	return dp;
}



void dumba5_stop_duh(DUMBA5_PLAYER * dp)
{
	if(dp)
	{
		al_destroy_thread(dp->thread);
		if(dp->sigrenderer)
		{
			duh_end_sigrenderer(dp->sigrenderer);
//			al_drain_stream(dp->stream);
			al_destroy_audio_stream(dp->stream);
		}
		free(dp);

		/* if we are using the internal DUH, free it automatically,
		   you are responsible for freeing your own DUHs */
		if(dumba5_duh)
		{
			unload_duh(dumba5_duh);
			dumba5_duh = NULL;
		}
	}
}



void dumba5_pause_duh(DUMBA5_PLAYER * dp)
{
	if (dp && dp->sigrenderer && (dp->flags & ADP_PLAYING))
	{
		al_set_audio_stream_playing(dp->stream, false);
		dp->flags &= ~ADP_PLAYING;
	}
}



void dumba5_resume_duh(DUMBA5_PLAYER * dp)
{
	if (dp && dp->sigrenderer && !(dp->flags & ADP_PLAYING))
	{
		al_set_audio_stream_playing(dp->stream, true);
//		voice_start(dp->stream->voice);
		dp->flags |= ADP_PLAYING;
	}
}

void dumba5_set_player_volume(DUMBA5_PLAYER * pp, float volume)
{
	if(pp)
	{
		pp->volume = volume;
	}
}

float dumba5_get_player_volume(DUMBA5_PLAYER * pp)
{
	return pp ? pp->volume : 0;
}

long dumba5_get_player_position(DUMBA5_PLAYER * pp)
{
	return pp ? duh_sigrenderer_get_position(pp->sigrenderer) : -1;
}

bool dumba5_player_playback_finished(DUMBA5_PLAYER * pp)
{
	if(pp)
	{
		return pp->done_playing;
	}
	return false;
}

DUMBA5_PLAYER * dumba5_encapsulate_sigrenderer(DUH_SIGRENDERER * sigrenderer, float volume, long bufsize, int freq)
{
	DUMBA5_PLAYER * dp;
	int n_channels;
	ALLEGRO_CHANNEL_CONF c_conf;

	if (!sigrenderer)
		return NULL;

	dp = (DUMBA5_PLAYER *) malloc(sizeof(*dp));
	if (!dp)
		return NULL;

	n_channels = duh_sigrenderer_get_n_channels(sigrenderer);
	if(n_channels == 1)
	{
		c_conf = ALLEGRO_CHANNEL_CONF_1;
	}
	else
	{
		c_conf = ALLEGRO_CHANNEL_CONF_2;
	}

	/* This restriction is imposed by Allegro. */
	ASSERT(n_channels > 0);
	ASSERT(n_channels <= 2);

	dp->flags = ADP_PLAYING;
	dp->bufsize = bufsize;
	dp->freq = freq;

	dp->stream = al_create_audio_stream(4, bufsize, freq, ALLEGRO_AUDIO_DEPTH_UINT16, c_conf);

	if (!dp->stream) {
		free(dp);
		return NULL;
	}

	dp->stream = al_create_audio_stream(4, bufsize, freq, ALLEGRO_AUDIO_DEPTH_UINT16, c_conf);

	if (!dp->stream) {
		free(dp);
		return NULL;
	}

	dp->volume = volume;
	dp->silentcount = 0;

	return dp;
}



DUH_SIGRENDERER * dumba5_get_module_sigrenderer(DUMBA5_PLAYER * dp)
{
	return dp ? dp->sigrenderer : NULL;
}



/* IMPORTANT: This function will return NULL if the music has ended. */
// Should this be changed? User might want to hack the underlying SIGRENDERER
// and resurrect it (e.g. change pattern number), before it gets destroyed...
DUH_SIGRENDERER * dumba5_decompose_to_sigrenderer(DUMBA5_PLAYER * pp)
{
	if(pp)
	{
		DUH_SIGRENDERER * sigrenderer = pp->sigrenderer;
		if(sigrenderer)
		{
			al_destroy_audio_stream(pp->stream);
		}
		free(pp);
		return sigrenderer;
	}
	return NULL;
}

DUH * dumba5_load_module(const char * fn)
{
	DUH * dp = NULL;
	dp = dumb_load_xm_quick(fn);
	if(!dp)
	{
		dp = dumb_load_it_quick(fn);
		if(!dp)
		{
			dp = dumb_load_mod_quick(fn);
			if(!dp)
			{
				dp = dumb_load_s3m_quick(fn);
				if(!dp)
				{
					return NULL;
				}
			}
		}
	}
	return dp;
}

static int it_loop_callback(void * data)
{
	DUMBA5_PLAYER * pp = (DUMBA5_PLAYER *)data;
	pp->done_playing = true;
	return 1;
}

/* return the player so you can use more advanced features if you want
   you can safely ignore the return value if all you want is to play a mod */
DUMBA5_PLAYER * dumba5_create_player(DUH * dp, int pattern, bool loop, int bufsize, int frequency, bool stereo)
{
	DUMBA5_PLAYER * player;
	ALLEGRO_CHANNEL_CONF c_conf;
	int n_channels = 2;

	/* This restriction is imposed by Allegro. */
	ASSERT(n_channels > 0);
	ASSERT(n_channels <= 2);

	if(!dp)
	{
		return NULL;
	}

	player = (DUMBA5_PLAYER *) malloc(sizeof(DUMBA5_PLAYER));
	if(!player)
	{
		return NULL;
	}

	player->flags = 0;
	player->bufsize = bufsize;
	player->freq = frequency;
	player->channels = n_channels;
	if(n_channels == 1)
	{
		c_conf = ALLEGRO_CHANNEL_CONF_1;
	}
	else
	{
		c_conf = ALLEGRO_CHANNEL_CONF_2;
	}
	player->stream = al_create_audio_stream(4, bufsize, frequency, ALLEGRO_AUDIO_DEPTH_INT16, c_conf);

	if(!player->stream)
	{
		free(player);
		return NULL;
	}
	al_attach_audio_stream_to_mixer(player->stream, al_get_default_mixer());

	player->sigrenderer = dumb_it_start_at_order(dp, n_channels, pattern);

	if(!player->sigrenderer)
	{
		al_destroy_audio_stream(player->stream);
		free(player);
		return NULL;
	}
	if(!loop)
	{
		dumb_it_set_loop_callback(duh_get_it_sigrenderer(player->sigrenderer), it_loop_callback, player);
		dumb_it_set_xm_speed_zero_callback(duh_get_it_sigrenderer(player->sigrenderer), it_loop_callback, player);
	}

	player->mutex = al_create_mutex();
	if(!player->mutex)
	{
		return NULL;
	}
	player->thread = al_create_thread(dumba5_update_thread, player);
	if(!player->thread)
	{
		return NULL;
	}

	player->volume = 1.0;
	player->silentcount = 0;
	player->duh = dp;
	player->done_playing = false;

	return player;
}

/* Destroy the specified player. */
void dumba5_destroy_player(DUMBA5_PLAYER * pp)
{
	if(pp)
	{
		al_destroy_thread(pp->thread);
		al_destroy_mutex(pp->mutex);
		if(pp->sigrenderer)
		{
			duh_end_sigrenderer(pp->sigrenderer);
//			al_drain_stream(dp->stream);
			al_destroy_audio_stream(pp->stream);
		}
		free(pp);
	}
}

/* Start the player. */
void dumba5_start_player(DUMBA5_PLAYER * pp)
{
	pp->flags = ADP_PLAYING;
	al_start_thread(pp->thread);
}

/* advanced functions */
bool dumba5_set_player_pattern(DUMBA5_PLAYER * pp, int pattern)
{
	bool paused;
	DUH * duh;
	int chan, freq;
	float vol;

	paused = !(pp->flags & ADP_PLAYING);
	chan = pp->channels;
	freq = pp->freq;
	vol = pp->volume;
	if(paused)
	{
		dumba5_resume_player(pp);
	}
	duh = pp->duh;
	dumba5_stop_duh(pp);
	pp = dumba5_create_player(duh, chan, pattern, true, 4096, freq);
	if(pp)
	{
		if(!paused)
		{
			dumba5_start_player(pp);
		}
		return true;
	}
	printf("failed to set pattern!\n");
	return false;
}

void dumba5_stop_player(DUMBA5_PLAYER * pp)
{
	dumba5_stop_duh(pp);
}

void dumba5_pause_player(DUMBA5_PLAYER * pp)
{
	dumba5_pause_duh(pp);
}

void dumba5_resume_player(DUMBA5_PLAYER * pp)
{
	dumba5_resume_duh(pp);
}


/* easy access functions */

/* return the player so you can use more advanced features if you want
   you can safely ignore the return value if all you want is to play a mod */
bool dumba5_play_module(DUH * dp, int pattern, bool loop, int frequency, bool stereo)
{
	dumba5_player = dumba5_create_player(dp, pattern, loop, 4096, frequency, stereo);
	if(!dumba5_player)
	{
		return false;
	}
	dumba5_start_player(dumba5_player);
	return true;
}

/* return the player so you can use more advanced features if you want
   you can safely ignore the return value if all you want is to play a mod */
bool dumba5_load_and_play_module(const char * fn, int pattern, bool loop, int frequency, bool stereo)
{
	bool ret;

	dumba5_duh = dumba5_load_module(fn);
	if(!dumba5_duh)
	{
		return false;
	}
	ret = dumba5_play_module(dumba5_duh, pattern, loop, frequency, stereo);
	if(!ret)
	{
		unload_duh(dumba5_duh);
		return false;
	}
	//printf("Module loaded and should be playing!\n");
	return ret;
}

bool dumba5_set_module_pattern(int pattern)
{
	return dumba5_set_player_pattern(dumba5_player, pattern);
}

void dumba5_stop_module(void)
{
	dumba5_stop_player(dumba5_player);
	unload_duh(dumba5_duh);
}

void dumba5_pause_module(void)
{
	dumba5_pause_player(dumba5_player);
}

void dumba5_resume_module(void)
{
	dumba5_resume_player(dumba5_player);
}

void dumba5_set_module_volume(float volume)
{
	dumba5_set_player_volume(dumba5_player, volume);
}

float dumba5_get_module_volume(void)
{
	return dumba5_get_player_volume(dumba5_player);
}

long dumba5_get_module_position(void)
{
	return dumba5_get_player_position(dumba5_player);
}

bool dumba5_module_playback_finished(void)
{
	return dumba5_player_playback_finished(dumba5_player);
}
