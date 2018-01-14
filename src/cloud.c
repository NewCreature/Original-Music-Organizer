#include <ctype.h>
#include "t3net/t3net.h"
#include "defines.h"
#include "constants.h"
#include "library.h"
#include "library_helpers.h"
#include "track.h"
#include "instance.h"

static char convert_tag_buffer[256];

static char * convert_tag_name(const char * tag_name)
{
	int i;

	strcpy(convert_tag_buffer, tag_name);
	for(i = 0; i < strlen(convert_tag_buffer); i++)
	{
		convert_tag_buffer[i] = tolower(convert_tag_buffer[i]);
		if(convert_tag_buffer[i] == ' ')
		{
			convert_tag_buffer[i] = '_';
		}
	}

	return convert_tag_buffer;
}

static const char * get_track_tag(OMO_TRACK * tp, const char * name)
{
	if(tp)
	{
		if(tp->codec_handler->get_tag)
		{
			return tp->codec_handler->get_tag(tp->codec_data, name);
		}
	}
	return NULL;
}

/* compare strings where s2 might have trailing space not in s1 */
static int cloud_strcmp(const char * s1, const char * s2)
{
	int i, j;
	int offset = 0;

	while(offset < strlen(s2) && s2[offset] == ' ')
	{
		offset++;
	}
	if(strlen(s1) == 0 && strlen(s2) - offset == 0)
	{
		return 0;
	}
	if(strlen(s2) - offset < strlen(s1))
	{
		return 1;
	}
	for(i = 0; i < strlen(s1); i++)
	{
		if(s1[i] != s2[i + offset])
		{
			return 1;
		}
	}
	if(i == strlen(s1))
	{
		for(j = i + 1; j < strlen(s2) - offset; j++)
		{
			if(s2[j + offset] != ' ')
			{
				return 1;
			}
		}
	}
	return 0;
}

/* submit user-genrated tags, ignore tags that are retrieved from the file */
bool omo_submit_track_tags(OMO_LIBRARY * lp, const char * id, const char * url, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path)
{
	T3NET_ARGUMENTS * arguments;
	T3NET_DATA * submit_data;
	const char * submit_error;
	const char * tagger_key;
	const char * val;
	const char * track_val;
	bool ret = false;
	int entry;
	OMO_TRACK * track;
	int i;

	arguments = t3net_create_arguments();
	if(arguments)
	{
		tagger_key = al_get_config_value(t3f_config, "Settings", "Tagger ID");
		if(tagger_key)
		{
			entry = omo_get_library_entry(lp, id);
			if(entry < 0)
			{
				entry = omo_get_library_base_entry(lp, id);
			}
			if(entry >= 0)
			{
				al_stop_timer(t3f_timer);
				track = omo_load_track(archive_handler_registry, codec_handler_registry, lp->entry[entry]->filename, lp->entry[entry]->sub_filename, lp->entry[entry]->track, temp_path);
				al_start_timer(t3f_timer);

				t3net_add_argument(arguments, "tagger", tagger_key);
				t3net_add_argument(arguments, "track_id", id);
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					if(omo_tag_type[i])
					{
						val = omo_get_database_value(lp->entry_database, id, omo_tag_type[i]);
						if(val)
						{
							track_val = get_track_tag(track, omo_tag_type[i]);
							if(!track_val || cloud_strcmp(val, track_val))
							{
								t3net_add_argument(arguments, convert_tag_name(omo_tag_type[i]), val);
							}
						}
					}
				}
				val = omo_get_database_value(lp->entry_database, id, "Split Track Info");
				if(val)
				{
					t3net_add_argument(arguments, convert_tag_name("Split Track Info"), val);
				}
				val = omo_get_database_value(lp->entry_database, id, "Detected Length");
				if(val)
				{
					t3net_add_argument(arguments, convert_tag_name("Detected Length"), val);
				}
				if(track)
				{
					omo_unload_track(track);
				}
				submit_data = t3net_get_data(url, arguments);
				if(submit_data)
				{
					submit_error = t3net_get_error(submit_data);
					if(submit_error)
					{
						if(!strcmp(submit_error, "Can't delete non-existent entry.\r\n"))
						{
							omo_remove_database_key(lp->entry_database, id, "Submitted");
						}
					}
					else
					{
						ret = true;
					}
					t3net_destroy_data(submit_data);
				}
			}
		}
		t3net_destroy_arguments(arguments);
	}

	return ret;
}

bool omo_retrieve_track_tags(OMO_LIBRARY * lp, const char * id, const char * url)
{
	T3NET_ARGUMENTS * arguments;
	T3NET_DATA * track_data;
	const char * track_val;
	char buffer[256];
	int entry;
	bool ret = false;
	int i;

	arguments = t3net_create_arguments();
	if(arguments)
	{
		t3net_add_argument(arguments, "track_id", id);
		track_data = t3net_get_data(url, arguments);
		t3net_destroy_arguments(arguments);
		if(track_data)
		{
			for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
			{
				if(omo_tag_type[i])
				{
					track_val = t3net_get_data_entry_field(track_data, 0, convert_tag_name(omo_tag_type[i]));
					if(track_val)
					{
						omo_set_database_value(lp->entry_database, id, omo_tag_type[i], track_val);
					}
				}
			}
			track_val = t3net_get_data_entry_field(track_data, 0, convert_tag_name("Split Track Info"));
			if(track_val)
			{
				strcpy(buffer, track_val);
				entry = omo_get_library_entry(lp, id);
				if(entry >= 0)
				{
					omo_split_track(lp, lp->entry[entry]->filename, buffer);
				}
			}
			track_val = t3net_get_data_entry_field(track_data, 0, convert_tag_name("Detected Length"));
			if(track_val)
			{
				omo_set_database_value(lp->entry_database, id, convert_tag_name("Detected Length"), track_val);
			}
			ret = true;
		}
	}
	return ret;
}

static void * cloud_submit_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;
	char buffer[1024];
	const char * base_id;
	const char * id;
	int i;

	for(i = 0; i < app->library->entry_count; i++)
	{
		id = app->library->entry[i]->id;
		val = omo_get_database_value(app->library->entry_database, app->library->entry[i]->id, "Submitted");
		if(!val)
		{
			base_id = omo_get_library_file_base_id(app->library, app->library->entry[i]->filename, buffer);
			if(base_id)
			{
				id = base_id;
				val = omo_get_database_value(app->library->entry_database, id, "Submitted");
			}
		}
		if(val && !strcmp(val, "false"))
		{
			sprintf(app->status_bar_text, "Submitting tags: %s", id);
			if(omo_submit_track_tags(app->library, id, app->cloud_url, app->archive_handler_registry, app->codec_handler_registry, app->cloud_temp_path))
			{
				omo_remove_database_key(app->library->entry_database, id, "Submitted");
			}
		}
		if(al_get_thread_should_stop(thread))
		{
			break;
		}
	}
	sprintf(app->status_bar_text, "Library ready.");
	return NULL;
}

bool omo_submit_library_tags(APP_INSTANCE * app, const char * url)
{
	if(app->cloud_thread)
	{
		al_destroy_thread(app->cloud_thread);
		app->cloud_thread = NULL;
	}
	app->cloud_thread = al_create_thread(cloud_submit_thread_proc, app);
	if(app->cloud_thread)
	{
		strcpy(app->cloud_url, url);
		al_start_thread(app->cloud_thread);
		return true;
	}
	return false;
}

static void * cloud_retrieve_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < app->library->entry_count; i++)
	{
		sprintf(app->status_bar_text, "Retrieving tags: %s", app->library->entry[i]->id);
		omo_retrieve_track_tags(app->library, app->library->entry[i]->id, app->cloud_url);
		if(al_get_thread_should_stop(thread))
		{
			break;
		}
	}
	sprintf(app->status_bar_text, "Library ready.");
	return NULL;
}

bool omo_retrieve_library_tags(APP_INSTANCE * app, const char * url)
{
	if(app->cloud_thread)
	{
		al_destroy_thread(app->cloud_thread);
		app->cloud_thread = NULL;
	}
	app->cloud_thread = al_create_thread(cloud_retrieve_thread_proc, app);
	if(app->cloud_thread)
	{
		strcpy(app->cloud_url, url);
		al_start_thread(app->cloud_thread);
		return true;
	}
	return false;
}
