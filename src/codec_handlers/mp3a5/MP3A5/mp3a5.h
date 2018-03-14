#ifndef MP3A5_H
#define MP3A5_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <mpg123.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{

	char * artist;
	char * album;
	char * disc;
	char * track;
	char * title;
	char * genre;
	char * year;
	char * copyright;
	char * comment;

} MP3A5_MP3_TAGS;

typedef struct
{

	ALLEGRO_AUDIO_STREAM * audio_stream;
	mpg123_handle * mp3;
	MP3A5_MP3_TAGS * tags;
	ALLEGRO_THREAD * thread;
	ALLEGRO_MUTEX * mutex;
	double length;
	int buffer_size;
	bool paused;
	bool done;
	double loop_start;
	double loop_end;
	bool loop;

	/* format */
	long sample_rate;
	int channels;
	int encoding;
	ALLEGRO_AUDIO_DEPTH depth;
	ALLEGRO_CHANNEL_CONF channel_conf;

} MP3A5_MP3;

bool mp3a5_init(void);
void mp3a5_exit(void);

MP3A5_MP3 * mp3a5_load_mp3(const char *filename);
void mp3a5_destroy_mp3(MP3A5_MP3 * mp);
bool mp3a5_set_mp3_loop(MP3A5_MP3 * mp, double start, double end);
bool mp3a5_play_mp3(MP3A5_MP3 * mp, size_t buffer_count, unsigned int samples);
void mp3a5_stop_mp3(MP3A5_MP3 * mp);
void mp3a5_pause_mp3(MP3A5_MP3 * mp);
void mp3a5_resume_mp3(MP3A5_MP3 * mp);
double mp3a5_get_position(MP3A5_MP3 * mp);
bool mp3a5_set_position(MP3A5_MP3 * mp, double pos);

#ifdef __cplusplus
}
#endif

#endif
