#include "t3f/t3f.h"
#include "instance.h"
#include "library.h"

static OMO_LIBRARY * library = NULL;

static int sort_by_path(const void *e1, const void *e2)
{
	OMO_QUEUE_ENTRY ** entry1 = (OMO_QUEUE_ENTRY **)e1;
	OMO_QUEUE_ENTRY ** entry2 = (OMO_QUEUE_ENTRY **)e2;
	int c;
	int id1, id2;

	c = strcmp((*entry1)->file, (*entry2)->file);
	if(c != 0)
	{
		return c;
	}

	if((*entry1)->sub_file && (*entry2)->sub_file)
	{
		id1 = atoi((*entry1)->sub_file);
		id2 = atoi((*entry2)->sub_file);
		if(id1 != id2)
		{
			return id1 - id2;
		}
	}

	if((*entry1)->track && (*entry2)->track)
	{
		id1 = atoi((*entry1)->track);
		id2 = atoi((*entry2)->track);
		if(id1 != id2)
		{
			return id1 - id2;
		}
	}
	return 0;
}

static int sort_by_track(const void *e1, const void *e2)
{
	OMO_QUEUE_ENTRY ** entry1 = (OMO_QUEUE_ENTRY **)e1;
	OMO_QUEUE_ENTRY ** entry2 = (OMO_QUEUE_ENTRY **)e2;
	const char * sort_field[5] = {"Artist", "Album", "Disc", "Track", "Title"};
	int sort_type[5] = {0, 0, 1, 1, 0};
	char buf[1024];
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i1, i2;
	int i, c;

	if(!library)
	{
		return sort_by_path(e1, e2);
	}
	sprintf(buf, "%s%s%s", (*entry1)->file, (*entry1)->sub_file ? "/" : "", (*entry1)->sub_file ? (*entry1)->sub_file : "");
	id1 = omo_get_database_value(library->file_database, buf, "id");
	sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
	id2 = omo_get_database_value(library->file_database, buf, "id");

	if(id1 && id2)
	{
		for(i = 0; i < 5; i++)
		{
			val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
			val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
			if(val1 && val2)
			{
				if(sort_type[i] == 0)
				{
					c = strcmp(val1, val2);
					if(c != 0)
					{
						return c;
					}
				}
				else
				{
					i1 = atoi(val1);
					i2 = atoi(val2);
					if(i1 != i2)
					{
						return i1 - i2;
					}
				}
			}
		}
	}
	return sort_by_path(e1, e2);
}

static int sort_by_artist_and_title(const void *e1, const void *e2)
{
	OMO_QUEUE_ENTRY ** entry1 = (OMO_QUEUE_ENTRY **)e1;
	OMO_QUEUE_ENTRY ** entry2 = (OMO_QUEUE_ENTRY **)e2;
	const char * sort_field[2] = {"Artist", "Title"};
	char buf[1024];
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i, c;

	if(!library)
	{
		return sort_by_path(e1, e2);
	}
	sprintf(buf, "%s%s%s", (*entry1)->file, (*entry1)->sub_file ? "/" : "", (*entry1)->sub_file ? (*entry1)->sub_file : "");
	id1 = omo_get_database_value(library->file_database, buf, "id");
	sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
	id2 = omo_get_database_value(library->file_database, buf, "id");

	if(id1 && id2)
	{
		for(i = 0; i < 2; i++)
		{
			val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
			val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
			if(val1 && val2)
			{
				c = strcmp(val1, val2);
				if(c != 0)
				{
					return c;
				}
			}
		}

	}
	return sort_by_path(e1, e2);
}

static int sort_by_title(const void *e1, const void *e2)
{
	OMO_QUEUE_ENTRY ** entry1 = (OMO_QUEUE_ENTRY **)e1;
	OMO_QUEUE_ENTRY ** entry2 = (OMO_QUEUE_ENTRY **)e2;
	const char * sort_field[1] = {"Title"};
	char buf[1024];
	const char * val1;
	const char * val2;
	const char * id1;
	const char * id2;
	int i, c;

	if(!library)
	{
		return sort_by_path(e1, e2);
	}
	sprintf(buf, "%s%s%s", (*entry1)->file, (*entry1)->sub_file ? "/" : "", (*entry1)->sub_file ? (*entry1)->sub_file : "");
	id1 = omo_get_database_value(library->file_database, buf, "id");
	sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
	id2 = omo_get_database_value(library->file_database, buf, "id");

	if(id1 && id2)
	{
		for(i = 0; i < 1; i++)
		{
			val1 = omo_get_database_value(library->entry_database, id1, sort_field[i]);
			val2 = omo_get_database_value(library->entry_database, id2, sort_field[i]);
			if(val1 && val2)
			{
				c = strcmp(val1, val2);
				if(c != 0)
				{
					return c;
				}
			}
		}

	}
	return sort_by_path(e1, e2);
}

bool omo_get_queue_entry_tags(OMO_QUEUE * qp, int i, OMO_LIBRARY * lp)
{
	char section[1024];
	const char * val;
	const char * artist = NULL;
	const char * album = NULL;
	const char * title = NULL;
	const char * track = NULL;
	const char * length = NULL;
	const char * loop_start = NULL;
	const char * loop_end = NULL;
	const char * fade_time = NULL;
	const char * loop_length = NULL;
	double d_loop_start = 0.0;
	double d_loop_end = 0.0;
	double d_fade_time = 0.0;
	double d_loop_length = 0.0;
	bool ret = false;

	qp->entry[i]->tags_retrieved = false;
	if(lp)
	{
		strcpy(section, qp->entry[i]->file);
		if(qp->entry[i]->sub_file)
		{
			strcat(section, "/");
			strcat(section, qp->entry[i]->sub_file);
		}
		if(qp->entry[i]->track)
		{
			strcat(section, ":");
			strcat(section, qp->entry[i]->track);
		}
		val = omo_get_database_value(lp->file_database, section, "id");
		if(val)
		{
			artist = omo_get_database_value(lp->entry_database, val, "Artist");
			album = omo_get_database_value(lp->entry_database, val, "Album");
			title = omo_get_database_value(lp->entry_database, val, "Title");
			track = omo_get_database_value(lp->entry_database, val, "Track");
			length = omo_get_database_value(lp->entry_database, val, "Detected Length");
			if(!length)
			{
				loop_start = omo_get_database_value(lp->entry_database, val, "Loop Start");
				loop_end = omo_get_database_value(lp->entry_database, val, "Loop End");
				fade_time = omo_get_database_value(lp->entry_database, val, "Fade Time");
				loop_length = omo_get_database_value(lp->entry_database, val, "Length");
				if(!loop_start || !loop_end)
				{
					length = omo_get_database_value(lp->entry_database, val, "Length");
				}
			}
			if(artist)
			{
				strcpy(qp->entry[i]->tags.artist, artist);
				ret = true;
			}
			if(album)
			{
				strcpy(qp->entry[i]->tags.album, album);
				ret = true;
			}
			if(title)
			{
				strcpy(qp->entry[i]->tags.title, title);
				ret = true;
			}
			if(track)
			{
				strcpy(qp->entry[i]->tags.track, track);
				ret = true;
			}
			/* use 'Detected Length' or 'Length' database values */
			if(length)
			{
				qp->entry[i]->tags.length = atof(length);
			}
			/* use 'Loop Start', 'Loop End', 'Fade Time', and 'Length' database values */
			else
			{
				if(loop_start && loop_end)
				{
					d_loop_start = atof(loop_start);
					d_loop_end = atof(loop_end);
					if(fade_time)
					{
						d_fade_time = atof(fade_time);
					}
					if(loop_length)
					{
						d_loop_length = atof(loop_length);
					}

					/* loop end is < 0.0 means we use the track's original length */
					if(d_loop_end < 0)
					{
						if(loop_length)
						{
							d_loop_end = d_loop_length;
						}
						else
						{
							ret = false;
						}
					}
					else
					{
						qp->entry[i]->tags.length = d_loop_start + (d_loop_end - d_loop_start) * 2.0;
						if(fade_time)
						{
							qp->entry[i]->tags.length += d_fade_time;
						}
					}
				}
				else
				{
					ret = false;
				}
			}
		}
	}
	if(qp->entry[i]->skip_scan)
	{
		ret = true;
	}
	if(ret)
	{
		qp->entry[i]->tags_retrieved = true;
	}
	return ret;
}

static void * get_queue_tags_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_ARCHIVE_HANDLER * archive_handler;
	void * archive_handler_data;
	OMO_CODEC_HANDLER * codec_handler;
	void * codec_handler_data;
	char fn_buffer[1024];
	const char * extracted_fn;
	const char * target_fn;
	const char * tag;
	int i;

	for(i = 0; i < app->player->queue->entry_count && !al_get_thread_should_stop(thread); i++)
	{
		if(!app->player->queue->entry[i]->tags_retrieved)
		{
			extracted_fn = NULL;
			target_fn = NULL;
			archive_handler = omo_get_archive_handler(app->archive_handler_registry, app->player->queue->entry[i]->file);
			if(archive_handler && app->player->queue->entry[i]->sub_file)
			{
				archive_handler_data = archive_handler->open_archive(app->player->queue->entry[i]->file, app->queue_tags_temp_path);
				if(archive_handler_data)
				{
					extracted_fn = archive_handler->extract_file(archive_handler_data, atoi(app->player->queue->entry[i]->sub_file), fn_buffer);
					target_fn = extracted_fn;
					archive_handler->close_archive(archive_handler_data);
				}
			}
			else
			{
				target_fn = app->player->queue->entry[i]->file;
			}
			codec_handler = omo_get_codec_handler(app->codec_handler_registry, target_fn, NULL);
			if(codec_handler && codec_handler->get_tag)
			{
				codec_handler_data = codec_handler->load_file(target_fn, app->player->queue->entry[i]->track);
				if(codec_handler_data)
				{
					if(!strlen(app->player->queue->entry[i]->tags.artist))
					{
						tag = codec_handler->get_tag(codec_handler_data, "Artist");
						if(tag)
						{
							strcpy(app->player->queue->entry[i]->tags.artist, tag);
						}
					}
					if(!strlen(app->player->queue->entry[i]->tags.album))
					{
						tag = codec_handler->get_tag(codec_handler_data, "Album");
						if(tag)
						{
							strcpy(app->player->queue->entry[i]->tags.album, tag);
						}
					}
					if(!strlen(app->player->queue->entry[i]->tags.title))
					{
						tag = codec_handler->get_tag(codec_handler_data, "Title");
						if(tag)
						{
							strcpy(app->player->queue->entry[i]->tags.title, tag);
						}
					}
					if(!strlen(app->player->queue->entry[i]->tags.track))
					{
						tag = codec_handler->get_tag(codec_handler_data, "Track");
						if(tag)
						{
							strcpy(app->player->queue->entry[i]->tags.track, tag);
						}
					}
					if(codec_handler->get_length)
					{
						app->player->queue->entry[i]->tags.length = codec_handler->get_length(codec_handler_data);
					}
					codec_handler->unload_file(codec_handler_data);
					app->player->queue->entry[i]->tags_retrieved = true;
				}
			}
			if(extracted_fn)
			{
				al_remove_filename(extracted_fn);
			}
		}
		app->player->queue->length = 0.0;
		for(i = 0; i < app->player->queue->entry_count; i++)
		{
			app->player->queue->length += app->player->queue->entry[i]->tags.length;
		}
	}
	app->player->queue->thread_done = true;
	return NULL;
}

void omo_get_queue_tags(OMO_QUEUE * qp, OMO_LIBRARY * lp, void * data)
{
	int i;
	bool rescan = false;

	if(qp)
	{
		if(qp->thread)
		{
			al_join_thread(qp->thread, NULL);
			al_destroy_thread(qp->thread);
			qp->thread = NULL;
		}
		for(i = 0; i < qp->entry_count; i++)
		{
			if(!omo_get_queue_entry_tags(qp, i, lp))
			{
				rescan = true;
			}
		}
		if(rescan)
		{
			qp->thread = al_create_thread(get_queue_tags_thread_proc, data);
			if(qp->thread)
			{
				qp->thread_done = false;
				al_start_thread(qp->thread);
			}
		}
		else
		{
			qp->length = 0.0;
			for(i = 0; i < qp->entry_count; i++)
			{
				qp->length += qp->entry[i]->tags.length;
			}
		}
	}
}

void omo_sort_queue(OMO_QUEUE * qp, OMO_LIBRARY * lp, int mode, int start_index, int count)
{
	library = lp;
	switch(mode)
	{
		case 0:
		{
			qsort(&qp->entry[start_index], count, sizeof(OMO_QUEUE_ENTRY *), sort_by_track);
			break;
		}
		case 1:
		{
			qsort(&qp->entry[start_index], count, sizeof(OMO_QUEUE_ENTRY *), sort_by_artist_and_title);
			break;
		}
		case 2:
		{
			qsort(&qp->entry[start_index], count, sizeof(OMO_QUEUE_ENTRY *), sort_by_title);
			break;
		}
	}
}
