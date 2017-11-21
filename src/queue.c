#include "t3f/t3f.h"
#include "t3f/file.h"
#include "instance.h"
#include "queue.h"

static const char * omo_queue_file_header = "OQ01";

OMO_QUEUE * omo_create_queue(int files)
{
	OMO_QUEUE * qp = NULL;

	qp = malloc(sizeof(OMO_QUEUE));
	if(qp)
	{
		memset(qp, 0, sizeof(OMO_QUEUE));
		qp->entry = malloc(sizeof(OMO_QUEUE_ENTRY *) * files);
		if(!qp->entry)
		{
			free(qp);
			return NULL;
		}
		else
		{
			qp->entry_size = files;
			qp->entry_count = 0;
		}
	}
	return qp;
}

static bool omo_save_queue_entry_f(ALLEGRO_FILE * fp, OMO_QUEUE_ENTRY * ep)
{
	int c = ep->tags_retrieved ? 1 : 0;
	if(!t3f_save_string_f(fp, ep->file))
	{
		return false;
	}
	if(!t3f_save_string_f(fp, ep->sub_file))
	{
		return false;
	}
	if(!t3f_save_string_f(fp, ep->track))
	{
		return false;
	}
	if(al_fputc(fp, c) == EOF)
	{
		return false;
	}
	if(al_fputc(fp, ep->skip_scan ? 1 : 0) == EOF)
	{
		return false;
	}
	if(c)
	{
		if(!t3f_save_string_f(fp, ep->tags.artist))
		{
			return false;
		}
		if(!t3f_save_string_f(fp, ep->tags.album))
		{
			return false;
		}
		if(!t3f_save_string_f(fp, ep->tags.title))
		{
			return false;
		}
		if(!t3f_save_string_f(fp, ep->tags.track))
		{
			return false;
		}
	}
	return true;
}

bool omo_save_queue(OMO_QUEUE * qp, const char * fn)
{
	ALLEGRO_FILE * fp;
	int i, l;

	fp = al_fopen(fn, "wb");
	if(fp)
	{
		l = strlen(omo_queue_file_header);
		if(al_fwrite(fp, omo_queue_file_header, l) != l)
		{
			goto fail;
		}
		if(al_fwrite32le(fp, qp->entry_count) < 4)
		{
			goto fail;
		}
		for(i = 0; i < qp->entry_count; i++)
		{
			if(!omo_save_queue_entry_f(fp, qp->entry[i]))
			{
				goto fail;
			}
		}
		al_fclose(fp);
		return true;
	}

	fail:
	{
		if(fp)
		{
			al_fclose(fp);
		}
	}
	return false;
}

static bool omo_load_queue_entry_f(ALLEGRO_FILE * fp, OMO_QUEUE * qp)
{
	char * file = NULL;
	char * sub_file = NULL;
	char * track = NULL;
	char * tag = NULL;
	int c;

	file = t3f_load_string_f(fp);
	if(!file)
	{
		goto fail;
	}
	sub_file = t3f_load_string_f(fp);
	if(!sub_file)
	{
		goto fail;
	}
	track = t3f_load_string_f(fp);
	if(!track)
	{
		goto fail;
	}
	if(!omo_add_file_to_queue(qp, file, sub_file, track, false))
	{
		goto fail;
	}
	free(track);
	free(sub_file);
	free(file);
	c = al_fgetc(fp);
	if(c == EOF)
	{
		goto fail;
	}
	qp->entry[qp->entry_count - 1]->skip_scan = c;
	c = al_fgetc(fp);
	if(c == EOF)
	{
		goto fail;
	}
	if(c)
	{
		qp->entry[qp->entry_count - 1]->tags_retrieved = true;
		tag = t3f_load_string_f(fp);
		if(!tag)
		{
			goto fail;
		}
		strcpy(qp->entry[qp->entry_count - 1]->tags.artist, tag);
		tag = t3f_load_string_f(fp);
		if(!tag)
		{
			goto fail;
		}
		strcpy(qp->entry[qp->entry_count - 1]->tags.album, tag);
		tag = t3f_load_string_f(fp);
		if(!tag)
		{
			goto fail;
		}
		strcpy(qp->entry[qp->entry_count - 1]->tags.title, tag);
		tag = t3f_load_string_f(fp);
		if(!tag)
		{
			goto fail;
		}
		strcpy(qp->entry[qp->entry_count - 1]->tags.track, tag);
	}
	return true;

	fail:
	{
		if(track)
		{
			free(track);
		}
		if(sub_file)
		{
			free(sub_file);
		}
		if(file)
		{
			free(file);
		}
	}
	return false;
}

OMO_QUEUE * omo_load_queue(const char * fn)
{
	ALLEGRO_FILE * fp = NULL;
	OMO_QUEUE * qp = NULL;
	int i, c;

	fp = al_fopen(fn, "rb");
	if(fp)
	{
		for(i = 0; i < strlen(omo_queue_file_header); i++)
		{
			c = al_fgetc(fp);
			if(c != omo_queue_file_header[i])
			{
				goto fail;
			}
		}
		c = al_fread32le(fp);
		if(al_feof(fp))
		{
			goto fail;
		}
		qp = omo_create_queue(c);
		if(!qp)
		{
			goto fail;
		}
		for(i = 0; i < c; i++)
		{
			if(!omo_load_queue_entry_f(fp, qp))
			{
				goto fail;
			}
		}
		al_fclose(fp);
		return qp;
	}

	fail:
	{
		if(qp)
		{
			omo_destroy_queue(qp);
		}
		if(fp)
		{
			al_fclose(fp);
		}
	}
	return NULL;
}

void omo_destroy_queue(OMO_QUEUE * qp)
{
	int i;

	if(qp->thread)
	{
		al_destroy_thread(qp->thread);
	}
	for(i = 0; i < qp->entry_count; i++)
	{
		if(qp->entry[i]->file)
		{
			free(qp->entry[i]->file);
		}
		if(qp->entry[i]->sub_file)
		{
			free(qp->entry[i]->sub_file);
		}
		if(qp->entry[i]->track)
		{
			free(qp->entry[i]->track);
		}
		free(qp->entry[i]);
	}
	free(qp->entry);
	free(qp);
}

bool omo_resize_queue(OMO_QUEUE ** qp, int files)
{
	OMO_QUEUE * new_queue;
	bool ret = true;
	int i;

	if(files > (*qp)->entry_count)
	{
		new_queue = omo_create_queue(files);
		if(new_queue)
		{
			for(i = 0; i < (*qp)->entry_count; i++)
			{
				if(!omo_copy_queue_item((*qp)->entry[i], new_queue))
				{
					ret = false;
				}
			}
			omo_destroy_queue(*qp);
			*qp = new_queue;
		}
		else
		{
			ret = false;
		}
	}
	return ret;
}

bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn, const char * subfn, const char * track, bool skip_scan)
{
	if(qp->entry_count < qp->entry_size)
	{
		qp->entry[qp->entry_count] = malloc(sizeof(OMO_QUEUE_ENTRY));
		if(qp->entry[qp->entry_count])
		{
			memset(qp->entry[qp->entry_count], 0, sizeof(OMO_QUEUE_ENTRY));
			qp->entry[qp->entry_count]->file = malloc(strlen(fn) + 1);
			if(qp->entry[qp->entry_count]->file)
			{
				strcpy(qp->entry[qp->entry_count]->file, fn);
				qp->entry[qp->entry_count]->sub_file = NULL;
				if(subfn && strlen(subfn) > 0)
				{
					qp->entry[qp->entry_count]->sub_file = malloc(strlen(subfn) + 1);
					if(qp->entry[qp->entry_count]->sub_file)
					{
						strcpy(qp->entry[qp->entry_count]->sub_file, subfn);
					}
				}
				qp->entry[qp->entry_count]->track = NULL;
				if(track && strlen(track) > 0)
				{
					qp->entry[qp->entry_count]->track = malloc(strlen(track) + 1);
					if(qp->entry[qp->entry_count]->track)
					{
						strcpy(qp->entry[qp->entry_count]->track, track);
					}
				}
				qp->entry[qp->entry_count]->skip_scan = skip_scan;
				qp->entry_count++;
				return true;
			}
		}
	}
	return false;
}

void omo_delete_queue_item(OMO_QUEUE * qp, int index)
{
	int i;

	if(index < qp->entry_count)
	{
		if(qp->thread)
		{
			al_destroy_thread(qp->thread);
			qp->thread = NULL;
		}
		if(qp->entry[index]->file)
		{
			free(qp->entry[index]->file);
		}
		if(qp->entry[index]->sub_file)
		{
			free(qp->entry[index]->sub_file);
		}
		free(qp->entry[index]);
		for(i = index; i < qp->entry_count - 1; i++)
		{
			qp->entry[i] = qp->entry[i + 1];
		}
		qp->entry_count--;
	}
}

bool omo_copy_queue_item(OMO_QUEUE_ENTRY * ep, OMO_QUEUE * qp)
{
	if(qp->entry_count < qp->entry_size)
	{
		qp->entry[qp->entry_count] = malloc(sizeof(OMO_QUEUE_ENTRY));
		if(qp->entry[qp->entry_count])
		{
			memset(qp->entry[qp->entry_count], 0, sizeof(OMO_QUEUE_ENTRY));
			if(ep->file)
			{
				qp->entry[qp->entry_count]->file = malloc(strlen(ep->file) + 1);
				if(qp->entry[qp->entry_count]->file)
				{
					strcpy(qp->entry[qp->entry_count]->file, ep->file);
				}
				else
				{
					return false;
				}
			}
			if(ep->sub_file)
			{
				qp->entry[qp->entry_count]->sub_file = malloc(strlen(ep->sub_file) + 1);
				if(qp->entry[qp->entry_count]->sub_file)
				{
					strcpy(qp->entry[qp->entry_count]->sub_file, ep->sub_file);
				}
				else
				{
					if(qp->entry[qp->entry_count]->file)
					{
						free(qp->entry[qp->entry_count]->file);
					}
					return false;
				}
			}
			if(ep->track)
			{
				qp->entry[qp->entry_count]->track = malloc(strlen(ep->track) + 1);
				if(qp->entry[qp->entry_count]->track)
				{
					strcpy(qp->entry[qp->entry_count]->track, ep->track);
				}
				else
				{
					if(qp->entry[qp->entry_count]->file)
					{
						free(qp->entry[qp->entry_count]->file);
					}
					if(qp->entry[qp->entry_count]->sub_file)
					{
						free(qp->entry[qp->entry_count]->sub_file);
					}
					return false;
				}
			}
			memcpy(&qp->entry[qp->entry_count]->tags, &ep->tags, sizeof(OMO_QUEUE_TAGS));
			qp->entry[qp->entry_count]->tags_retrieved = ep->tags_retrieved;
			qp->entry_count++;
			return true;
		}
	}
	return false;
}
