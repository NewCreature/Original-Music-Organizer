#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <mpg123.h>
#include "mp3a5.h"

bool mp3a5_init(void)
{
    mpg123_init();
    return true;
}

void mp3a5_exit(void)
{
    mpg123_exit();
}

MP3A5_MP3 * mp3a5_load_mp3(const char *filename)
{
    MP3A5_MP3 * mp3;

    mp3 = malloc(sizeof(MP3A5_MP3));
    if(mp3)
    {
        mp3->mp3 = mpg123_new(NULL, NULL);
        if(mp3->mp3)
        {
            if(mpg123_open(mp3->mp3, filename) == MPG123_OK)
            {
                return mp3;
            }
            else
            {
                mpg123_delete(mp3->mp3);
                free(mp3);
            }
        }
        else
        {
            free(mp3);
        }
    }
    return NULL;
}

void mp3a5_destroy_mp3(MP3A5_MP3 * mp)
{
    mp3a5_stop_mp3(mp);
    mpg123_delete(mp->mp3);
    free(mp);
}

static void * mp3a5_thread_proc(ALLEGRO_THREAD * tp, void * data)
{
    MP3A5_MP3 * mp3 = (MP3A5_MP3 *)data;
    ALLEGRO_EVENT_QUEUE * queue;
	unsigned char *fragment;
    size_t done;
    int bytes_left;
    int i, r;

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_audio_stream_event_source(mp3->audio_stream));
    al_set_audio_stream_playing(mp3->audio_stream, true);
    mp3->paused = false;
    mp3->done = false;
    bool end_of_mp3 = false;

	while(!end_of_mp3)
	{
		ALLEGRO_EVENT event;

		al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
		{
			fragment = (unsigned char *)al_get_audio_stream_fragment(mp3->audio_stream);
			if(fragment)
			{
				if(mp3->paused)
				{
					memset(fragment, 0, sizeof(unsigned short) * mp3->buffer_size * 2);
				}
				else
				{
                    bytes_left = sizeof(unsigned short) * mp3->buffer_size * 2;
                    while(bytes_left > 0)
                    {
                        r = mpg123_read(mp3->mp3, fragment, sizeof(unsigned short) * mp3->buffer_size * 2, &done);
                        if(r != MPG123_OK && r != MPG123_NEW_FORMAT)
                        {
                            bytes_left -= done;
                            for(i = sizeof(unsigned short) * mp3->buffer_size * 2 - bytes_left; i < sizeof(unsigned short) * mp3->buffer_size * 2; i++)
                            {
                                fragment[i] = 0;
                            }
                            end_of_mp3 = true;
                            break;
                        }
                        else
                        {
                            bytes_left -= done;
                        }
                    }
				}
				if(!al_set_audio_stream_fragment(mp3->audio_stream, fragment))
				{
				}
			}
		}
		if(al_get_thread_should_stop(tp))
		{
			break;
		}
	}
	al_destroy_event_queue(queue);
    mp3->done = true;
    return NULL;
}

bool mp3a5_play_mp3(MP3A5_MP3 * mp, size_t buffer_count, unsigned int samples)
{
    mp->audio_stream = al_create_audio_stream(buffer_count, samples, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    if(mp->audio_stream)
    {
        mp->buffer_size = samples;
        mp->thread = al_create_thread(mp3a5_thread_proc, mp);
        if(mp->thread)
        {
            al_attach_audio_stream_to_mixer(mp->audio_stream, al_get_default_mixer());
            al_start_thread(mp->thread);
            return true;
        }
        else
        {
            al_destroy_audio_stream(mp->audio_stream);
            mp->audio_stream = NULL;
        }
    }
    return false;
}

void mp3a5_stop_mp3(MP3A5_MP3 * mp)
{
    if(mp->thread && mp->audio_stream)
    {
        al_join_thread(mp->thread, NULL);
        al_destroy_thread(mp->thread);
        mp->thread = NULL;
        al_destroy_audio_stream(mp->audio_stream);
        mp->audio_stream = NULL;
        mpg123_close(mp->mp3);
        mp->mp3 = NULL;
    }
}

void mp3a5_pause_mp3(MP3A5_MP3 * mp)
{
    mp->paused = true;
}

void mp3a5_resume_mp3(MP3A5_MP3 * mp)
{
    mp->paused = false;
}
