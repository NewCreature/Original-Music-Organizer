#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <mpg123.h>
#include <stdio.h>
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

static void get_lines(mpg123_string *inlines, char * buffer)
{
	size_t i;
	int hadcr = 0, hadlf = 0;
	char *lines = NULL;
	char *line  = NULL;
	size_t len = 0;
    int buffer_len = 0;
    int line_len;

    strcpy(buffer, "");
	if(inlines != NULL && inlines->fill)
	{
		lines = inlines->p;
		len   = inlines->fill;
	}
	else return;

	line = lines;
	for(i=0; i<len; ++i)
	{
		if(lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0)
		{
			char save = lines[i]; /* saving, changing, restoring a byte in the data */
			if(save == '\n') ++hadlf;
			if(save == '\r') ++hadcr;
			if((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) line = "";

			if(line)
			{
				lines[i] = 0;
                line_len = strlen(line);
                if(buffer_len + line_len < 1023 - 1)
                {
                    strcat(buffer, line);
                    strcat(buffer, " ");
                    buffer_len += line_len + 1;
                }
                else
                {
                    break;
                }
				line = NULL;
				lines[i] = save;
			}
		}
		else
		{
			hadlf = hadcr = 0;
			if(line == NULL) line = lines+i;
		}
	}
    if(buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == ' ')
    {
        buffer[strlen(buffer) - 1] = 0;
    }
}

static char mp3a5_tag_buffer[1024] = {0};

static void mp3a5_get_length(MP3A5_MP3 * mp3)
{
    unsigned long length;
    int channels;
    int encoding;
    long freq;

    if(mpg123_getformat(mp3->mp3, &freq, &channels, &encoding) == MPG123_OK)
    {
        length = mpg123_length(mp3->mp3);
        mp3->length = (double)length / ((double)freq);
    }
}

static MP3A5_MP3_TAGS * mp3a5_get_tags(const char * filename, MP3A5_MP3 * mp3)
{
    mpg123_id3v1 * id3_v1 = NULL;
    mpg123_id3v2 * id3_v2 = NULL;
    int meta;
    char id[5] = {0};
    int i;

    /* read tags */
    mp3->tags = NULL;
    if(mpg123_open(mp3->mp3, filename) == MPG123_OK)
    {
        mpg123_scan(mp3->mp3);
        mpg123_seek(mp3->mp3, 0, SEEK_SET);
        mp3a5_get_length(mp3);
        mpg123_seek(mp3->mp3, 0, SEEK_SET);
        meta = mpg123_meta_check(mp3->mp3);
        if(meta & MPG123_ID3)
        {
            mpg123_id3(mp3->mp3, &id3_v1, &id3_v2);
            mp3->tags = malloc(sizeof(MP3A5_MP3_TAGS));
            if(mp3->tags)
            {
                memset(mp3->tags, 0, sizeof(MP3A5_MP3_TAGS));
                if(id3_v2)
                {
                    /* artist */
                    get_lines(id3_v2->artist, mp3a5_tag_buffer);
                    mp3->tags->artist = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->artist)
                    {
                        strcpy(mp3->tags->artist, mp3a5_tag_buffer);
                    }

                    /* album */
                    get_lines(id3_v2->album, mp3a5_tag_buffer);
                    mp3->tags->album = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->album)
                    {
                        strcpy(mp3->tags->album, mp3a5_tag_buffer);
                    }

                    /* title */
                    get_lines(id3_v2->title, mp3a5_tag_buffer);
                    mp3->tags->title = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->title)
                    {
                        strcpy(mp3->tags->title, mp3a5_tag_buffer);
                    }

                    /* genre */
                    get_lines(id3_v2->genre, mp3a5_tag_buffer);
                    mp3->tags->genre = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->genre)
                    {
                        strcpy(mp3->tags->genre, mp3a5_tag_buffer);
                    }

                    /* year */
                    get_lines(id3_v2->year, mp3a5_tag_buffer);
                    mp3->tags->year = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->year)
                    {
                        strcpy(mp3->tags->year, mp3a5_tag_buffer);
                    }

                    /* comment */
                    get_lines(id3_v2->comment, mp3a5_tag_buffer);
                    mp3->tags->comment = malloc(strlen(mp3a5_tag_buffer) + 1);
                    if(mp3->tags->comment)
                    {
                        strcpy(mp3->tags->comment, mp3a5_tag_buffer);
                    }

                    for(i = 0; i < id3_v2->texts; i++)
                    {
                        memcpy(id, id3_v2->text[i].id, 4);
                        if(!strcmp(id, "TPOS"))
                        {
                            get_lines(&id3_v2->text[i].text, mp3a5_tag_buffer);
                            mp3->tags->disc = malloc(strlen(mp3a5_tag_buffer) + 1);
                            if(mp3->tags->disc)
                            {
                                strcpy(mp3->tags->disc, mp3a5_tag_buffer);
                            }
                        }
                        else if(!strcmp(id, "TRCK"))
                        {
                            get_lines(&id3_v2->text[i].text, mp3a5_tag_buffer);
                            mp3->tags->track = malloc(strlen(mp3a5_tag_buffer) + 1);
                            if(mp3->tags->track)
                            {
                                strcpy(mp3->tags->track, mp3a5_tag_buffer);
                            }
                        }
                        else if(!strcmp(id, "TCOP"))
                        {
                            get_lines(&id3_v2->text[i].text, mp3a5_tag_buffer);
                            mp3->tags->copyright = malloc(strlen(mp3a5_tag_buffer) + 1);
                            if(mp3->tags->copyright)
                            {
                                strcpy(mp3->tags->copyright, mp3a5_tag_buffer);
                            }
                        }
                    }
                }
                else if(id3_v1)
                {
                    mp3->tags->artist = malloc(strlen(id3_v1->artist) + 1);
                    strcpy(mp3->tags->artist, id3_v1->artist);
                    mp3->tags->album = malloc(strlen(id3_v1->album) + 1);
                    strcpy(mp3->tags->album, id3_v1->album);
                    mp3->tags->title = malloc(strlen(id3_v1->title) + 1);
                    strcpy(mp3->tags->title, id3_v1->title);
                    mp3->tags->year = malloc(strlen(id3_v1->year) + 1);
                    strcpy(mp3->tags->year, id3_v1->year);
                    mp3->tags->comment = malloc(strlen(id3_v1->comment) + 1);
                    strcpy(mp3->tags->comment, id3_v1->comment);
                }
            }
        }
        mpg123_close(mp3->mp3);
    }
    return mp3->tags;
}

MP3A5_MP3 * mp3a5_load_mp3(const char *filename)
{
    MP3A5_MP3 * mp3;

    mp3 = malloc(sizeof(MP3A5_MP3));
    if(mp3)
    {
        memset(mp3, 0, sizeof(MP3A5_MP3));
        mp3->mp3 = mpg123_new(NULL, NULL);
        if(mp3->mp3)
        {
            mp3->tags = mp3a5_get_tags(filename, mp3);

            /* open file in preparation for streaming */
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
    if(mp->tags)
    {
        if(mp->tags->artist)
        {
            free(mp->tags->artist);
        }
        if(mp->tags->album)
        {
            free(mp->tags->album);
        }
        if(mp->tags->disc)
        {
            free(mp->tags->disc);
        }
        if(mp->tags->track)
        {
            free(mp->tags->track);
        }
        if(mp->tags->title)
        {
            free(mp->tags->title);
        }
        if(mp->tags->genre)
        {
            free(mp->tags->genre);
        }
        if(mp->tags->year)
        {
            free(mp->tags->year);
        }
        if(mp->tags->copyright)
        {
            free(mp->tags->copyright);
        }
        if(mp->tags->comment)
        {
            free(mp->tags->comment);
        }
        free(mp->tags);
    }
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
            mp->paused = false;
            mp->done = false;
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
