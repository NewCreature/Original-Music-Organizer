#include "t3f.h"
#include "music.h"

ALLEGRO_AUDIO_STREAM * t3f_stream = NULL;
ALLEGRO_MUTEX * t3f_music_mutex = NULL;
ALLEGRO_MUTEX * t3f_music_state_mutex = NULL;
ALLEGRO_THREAD * t3f_music_thread = NULL;
int t3f_music_state = T3F_MUSIC_STATE_OFF;
static float t3f_music_volume = 1.0;
static float t3f_new_music_volume = 1.0;
static float t3f_music_target_volume = 1.0;
static float t3f_music_fade_speed = 0.0;
static float t3f_music_gain = 1.0;
static bool t3f_music_looping_disabled = false;

static char t3f_music_thread_fn[4096] = {0};
static const ALLEGRO_FILE_INTERFACE * t3f_music_thread_file_interface = NULL;

ALLEGRO_DEBUG_CHANNEL("android");

static bool t3f_set_music_state(int state)
{

	/* create the mutex if necessary */
	if(!t3f_music_state_mutex)
	{
		t3f_music_state_mutex = al_create_mutex();
		if(!t3f_music_state_mutex)
		{
			return false;
		}
	}

	al_lock_mutex(t3f_music_state_mutex);
	t3f_music_state = state;
	al_unlock_mutex(t3f_music_state_mutex);
	return true;
}

static const char * t3f_get_music_extension(const char * fn)
{
	int i;

	for(i = strlen(fn); i >= 0; i--)
	{
		if(fn[i] == '.')
		{
			return &fn[i];
		}
	}
	return NULL;
}

static void * t3f_play_music_thread(ALLEGRO_THREAD * thread, void * arg)
{
	const char * ext = NULL;
	ALLEGRO_PATH * path = NULL;
	int loop_points = 0;
	float loop_start = -1;
	float loop_end = -1;
	bool loop_disabled = false;
	const char * val = NULL;
	ALLEGRO_CONFIG * config = NULL;

	ALLEGRO_DEBUG("music thread start\n");
	t3f_music_gain = 1.0;
	al_lock_mutex(t3f_music_mutex);
	if(t3f_stream)
	{
		t3f_stop_music();
	}
	ALLEGRO_DEBUG("setting file interface\n");
	al_set_new_file_interface(t3f_music_thread_file_interface);
	t3f_stream = al_load_audio_stream(t3f_music_thread_fn, 4, 4096);
	if(!t3f_stream)
	{
		al_unlock_mutex(t3f_music_mutex);
		t3f_set_music_state(T3F_MUSIC_STATE_OFF);
		return NULL;
	}

	ALLEGRO_DEBUG("configuring music\n");
	/* look for loop data */
	path = al_create_path(t3f_music_thread_fn);
	if(path)
	{
		al_set_path_extension(path, ".ini");
		config = al_load_config_file(al_path_cstr(path, '/'));
		if(config)
		{
			val = al_get_config_value(config, "loop", "disabled");
			if(val && !strcasecmp(val, "true"))
			{
				loop_disabled = true;
			}
			if(!loop_disabled)
			{
				val = al_get_config_value(config, "loop", "start");
				if(val)
				{
					loop_start = atof(val);
					loop_points++;
				}
				val = al_get_config_value(config, "loop", "end");
				if(val)
				{
					loop_end = atof(val);
					loop_points++;
				}
			}
			val = al_get_config_value(config, "settings", "gain");
			if(val)
			{
				t3f_music_gain = atof(val);
				if(t3f_music_gain < 0.0)
				{
					t3f_music_gain = 0;
				}
				if(t3f_music_gain > 10.0)
				{
					t3f_music_gain = 10.0;
				}
			}
			al_destroy_config(config);
		}
		al_destroy_path(path);
	}
	if(t3f_music_looping_disabled)
	{
		loop_disabled = true;
	}

	if(loop_disabled)
	{
		al_set_audio_stream_playmode(t3f_stream, ALLEGRO_PLAYMODE_ONCE);
	}
	else
	{
		if(loop_points != 2)
		{
			/* loop entire song unless audio is MOD music */
			ext = t3f_get_music_extension(t3f_music_thread_fn);
			if(strcmp(ext, ".xm") && strcmp(ext, ".it") && strcmp(ext, ".mod") && strcmp(ext, ".s3m"))
			{
				al_set_audio_stream_loop_secs(t3f_stream, 0.0, al_get_audio_stream_length_secs(t3f_stream));
				al_set_audio_stream_playmode(t3f_stream, ALLEGRO_PLAYMODE_LOOP);
			}
			else
			{
				al_set_audio_stream_playmode(t3f_stream, ALLEGRO_PLAYMODE_LOOP);
			}
		}
		else
		{
			al_set_audio_stream_loop_secs(t3f_stream, loop_start, loop_end);
			al_set_audio_stream_playmode(t3f_stream, ALLEGRO_PLAYMODE_LOOP);
		}
	}
	ALLEGRO_DEBUG("start the music\n");
	t3f_music_volume = t3f_new_music_volume;
	al_set_audio_stream_gain(t3f_stream, t3f_music_volume * t3f_music_gain);
	al_attach_audio_stream_to_mixer(t3f_stream, al_get_default_mixer());
	al_unlock_mutex(t3f_music_mutex);
	t3f_set_music_state(T3F_MUSIC_STATE_PLAYING);
	return NULL;
}

static void * t3f_fade_music_thread(void * arg)
{
	ALLEGRO_TIMER * timer;
	ALLEGRO_EVENT_QUEUE * queue;
	bool done = false;
	ALLEGRO_EVENT event;
	float s = t3f_music_fade_speed / 50.0;

	timer = al_create_timer(1.0 / s);
	if(!timer)
	{
		return NULL;
	}
	queue = al_create_event_queue();
	if(!queue)
	{
		return NULL;
	}
	al_register_event_source(queue, al_get_timer_event_source(timer));
	while(!done)
	{
		al_wait_for_event(queue, &event);
		al_set_audio_stream_gain(t3f_stream, t3f_music_target_volume);
		t3f_music_target_volume -= s;
		if(t3f_music_target_volume <= 0.0)
		{
			t3f_stop_music();
			done = true;
		}
	}
	return NULL;
}

/* need to come up with a way to define loops for non-MOD audio,
 * see if there is a corresponding INI file and read loop data from that */
bool t3f_play_music(const char * fn)
{
	ALLEGRO_DEBUG("attempting to play %s\n", fn);
	if(!(t3f_flags & T3F_USE_SOUND))
	{
		return false;
	}
	if(t3f_music_thread)
	{
		al_destroy_thread(t3f_music_thread);
		t3f_music_thread = NULL;
	}
	if(!t3f_music_mutex)
	{
		t3f_music_mutex = al_create_mutex();
	}
	if(t3f_music_mutex)
	{
		strcpy(t3f_music_thread_fn, fn);
		t3f_music_thread_file_interface = al_get_new_file_interface(); // copy current file interface so we can use it in the music thread
		t3f_set_music_state(T3F_MUSIC_STATE_TRACK_CHANGE);
		t3f_music_thread = al_create_thread(t3f_play_music_thread, NULL);
		if(t3f_music_thread)
		{
			al_start_thread(t3f_music_thread);
		}
//		al_run_detached_thread(t3f_play_music_thread, NULL);
		return true;
	}
	else
	{
		t3f_set_music_state(T3F_MUSIC_STATE_OFF);
	}
	return false;
}

void t3f_stop_music(void)
{
	if(t3f_stream)
	{
		al_destroy_audio_stream(t3f_stream);
		t3f_stream = NULL;
		t3f_set_music_state(T3F_MUSIC_STATE_OFF);
	}
}

void t3f_pause_music(void)
{
	if(t3f_stream && t3f_music_mutex)
	{
		al_lock_mutex(t3f_music_mutex);
		al_set_audio_stream_playing(t3f_stream, false);
		al_unlock_mutex(t3f_music_mutex);
		t3f_set_music_state(T3F_MUSIC_STATE_PAUSED);
	}
}

void t3f_resume_music(void)
{
	if(t3f_stream && t3f_music_mutex)
	{
		al_lock_mutex(t3f_music_mutex);
		al_set_audio_stream_playing(t3f_stream, true);
		al_unlock_mutex(t3f_music_mutex);
		t3f_set_music_state(T3F_MUSIC_STATE_PLAYING);
	}
}

void t3f_set_music_volume(float volume)
{
	t3f_music_volume = volume;
	t3f_new_music_volume = volume; // set this here so music new music will start at the desired volume
	if(t3f_stream)
	{
		al_set_audio_stream_gain(t3f_stream, t3f_music_volume * t3f_music_gain);
	}
}

void t3f_set_new_music_volume(float volume)
{
	t3f_new_music_volume = volume;
}

float t3f_get_music_volume(void)
{
	return t3f_music_volume;
}

void t3f_fade_out_music(float speed)
{
	if(t3f_stream)
	{
		t3f_music_fade_speed = speed;
		al_run_detached_thread(t3f_fade_music_thread, NULL);
	}
}

int t3f_get_music_state(void)
{
	int state;

	if(t3f_music_state_mutex)
	{
		al_lock_mutex(t3f_music_state_mutex);
		state = t3f_music_state;
		al_unlock_mutex(t3f_music_state_mutex);
		return state;
	}
	return T3F_MUSIC_STATE_OFF;
}

void t3f_disable_music_looping(void)
{
	t3f_music_looping_disabled = true;
}
