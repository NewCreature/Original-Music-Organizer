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
				mp3->mutex = al_create_mutex();
				if(!mp3->mutex)
				{
					mpg123_close(mp3->mp3);
					mpg123_delete(mp3->mp3);
					free(mp3);
					return NULL;
				}
				mp3a5_get_length(mp3);
				mp3->loop_start = 0.0;
				mp3->loop_end = mp3->length;
				if(mpg123_getformat(mp3->mp3, &mp3->sample_rate, &mp3->channels, &mp3->encoding) != MPG123_OK)
				{
					mpg123_close(mp3->mp3);
					mpg123_delete(mp3->mp3);
					free(mp3);
					return NULL;
				}
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
	al_destroy_mutex(mp->mutex);
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

bool mp3a5_set_mp3_loop(MP3A5_MP3 * mp, double start, double end)
{
	mp->loop_start = start;
	mp->loop_end = end;
	mp->loop = true;

	return true;
}

static off_t seconds_to_sample(MP3A5_MP3 * mp, double seconds)
{
	return (seconds * (double)mp->sample_rate);
}

static double sample_to_seconds(MP3A5_MP3 * mp, off_t sample)
{
	return sample / (double)mp->sample_rate;
}

static bool mp3_rewind(MP3A5_MP3 * mp)
{
	bool ret;

	al_lock_mutex(mp->mutex);
	ret = mpg123_seek(mp->mp3, seconds_to_sample(mp, mp->loop_start), SEEK_SET) >= 0 ? true : false;
	al_unlock_mutex(mp->mutex);

	return ret;
}

static size_t feed_stream(MP3A5_MP3 * mp, void * buffer)
{
	int word_size = 2;
	if(mp->depth == ALLEGRO_AUDIO_DEPTH_INT8 || mp->depth == ALLEGRO_AUDIO_DEPTH_UINT8)
	{
		word_size = 1;
	}
	double current_time = sample_to_seconds(mp, mpg123_tell(mp->mp3));
	double buffer_time = ((double)mp->buffer_size / (double)word_size * (double)mp->channels) / (double)mp->sample_rate;
	int read_length = mp->buffer_size * word_size * mp->channels;
	volatile long pos = 0;
	size_t read;
	int r;

	if(mp->loop)
	{
		if(current_time + buffer_time > mp->loop_end)
		{
			read_length = (mp->loop_end - current_time) * (double)mp->sample_rate * (double)word_size * (double)mp->channels;
			if(read_length < 0)
			{
				return 0;
			}
			read_length += read_length % word_size;
		}
	}
	while(pos < (unsigned long)read_length)
	{
		al_lock_mutex(mp->mutex);
		r = mpg123_read(mp->mp3, (unsigned char *)buffer + pos, read_length - pos, &read);
		al_unlock_mutex(mp->mutex);
		if(r != MPG123_OK && r != MPG123_NEW_FORMAT)
		{
			return pos;
		}
		pos += read;

		if(read_length - pos <= 0)
		{
			/* Return the number of useful bytes written. */
			return pos;
		}
	}
	return pos;
}

static void * mp3a5_thread_proc(ALLEGRO_THREAD * tp, void * data)
{
	MP3A5_MP3 * mp3 = (MP3A5_MP3 *)data;
	ALLEGRO_EVENT_QUEUE * queue;
	unsigned char *fragment;
	int i;
	volatile unsigned long pos = 0;
	int word_size = 2;
	if(mp3->depth == ALLEGRO_AUDIO_DEPTH_INT8 || mp3->depth == ALLEGRO_AUDIO_DEPTH_UINT8)
	{
		word_size = 1;
	}

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
				pos = 0;
				if(mp3->paused)
				{
					memset(fragment, 0, word_size * mp3->buffer_size * mp3->channels);
				}
				else
				{
					while(pos < word_size * mp3->buffer_size * mp3->channels)
					{
						pos += feed_stream(mp3, fragment + pos);

						if(pos < word_size * mp3->buffer_size * mp3->channels)
						{
							/* didn't fill buffer all the way so check for loop */
							if(mp3->loop)
							{
								/* Keep rewinding until the fragment is filled. */
								while(pos < word_size * mp3->buffer_size * mp3->channels)
								{
									mp3_rewind(mp3);
									pos += feed_stream(mp3, fragment + pos);
								}
							}
							/* if no loop, then we reached the end of the audio */
							else
							{
								for(i = pos; i < word_size * mp3->buffer_size * mp3->channels; i++)
								{
									((unsigned short *)fragment)[i] = 0;
								}
								end_of_mp3 = true;
								break;
							}
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
	/* set up audio depth and channels */
	if(mp->encoding == MPG123_ENC_SIGNED_8)
	{
		mp->depth = ALLEGRO_AUDIO_DEPTH_INT8;
	}
	else if(mp->encoding == MPG123_ENC_UNSIGNED_8)
	{
		mp->depth = ALLEGRO_AUDIO_DEPTH_UINT8;
	}
	else if(mp->encoding == MPG123_ENC_SIGNED_16)
	{
		mp->depth = ALLEGRO_AUDIO_DEPTH_INT16;
	}
	else if(mp->encoding == MPG123_ENC_UNSIGNED_16)
	{
		mp->depth = ALLEGRO_AUDIO_DEPTH_UINT16;
	}
	else if(mp->encoding == MPG123_ENC_FLOAT_32)
	{
		mp->depth = ALLEGRO_AUDIO_DEPTH_FLOAT32;
	}
	else
	{
		printf("Unsupported audio depth!\n");
		return false;
	}
	if(mp->channels == 1)
	{
		mp->channel_conf = ALLEGRO_CHANNEL_CONF_1;
	}
	else if(mp->channels == 2)
	{
		mp->channel_conf = ALLEGRO_CHANNEL_CONF_2;
	}
	else
	{
		printf("Unsupported channel configuration!\n");
		return false;
	}

	mp->audio_stream = al_create_audio_stream(buffer_count, samples, mp->sample_rate, mp->depth, mp->channel_conf);
	if(mp->audio_stream)
	{
		if(mp->loop)
		{
		}
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

double mp3a5_get_position(MP3A5_MP3 * mp)
{
	double ret;

	al_lock_mutex(mp->mutex);
	ret = sample_to_seconds(mp, mpg123_tell(mp->mp3));
	al_unlock_mutex(mp->mutex);

	return ret;
}

bool mp3a5_set_position(MP3A5_MP3 * mp, double pos)
{
	bool ret;

	al_lock_mutex(mp->mutex);
	ret = mpg123_seek(mp->mp3, seconds_to_sample(mp, pos), SEEK_SET) >= 0;
	al_unlock_mutex(mp->mutex);

	return ret;
}
