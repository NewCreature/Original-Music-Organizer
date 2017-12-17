#include <ctype.h>
#include "t3net/t3net.h"
#include "defines.h"
#include "constants.h"
#include "library.h"
#include "library_helpers.h"
#include "track.h"

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

/* submit user-genrated tags, ignore tags that are retrieved from the file */
bool omo_submit_tags(OMO_LIBRARY * lp, const char * id, const char * url, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path)
{
	T3NET_ARGUMENTS * arguments;
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
			if(entry >= 0)
			{
				track = omo_load_track(archive_handler_registry, codec_handler_registry, lp->entry[entry]->filename, lp->entry[entry]->sub_filename, lp->entry[entry]->track, temp_path);

				t3net_add_argument(arguments, "tagger", tagger_key);
				t3net_add_argument(arguments, "track_id", id);
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					if(omo_tag_type[i])
					{
						val = al_get_config_value(lp->entry_database, id, omo_tag_type[i]);
						if(val)
						{
							track_val = get_track_tag(track, omo_tag_type[i]);
							if(!track_val || strcmp(track_val, val))
							{
								t3net_add_argument(arguments, convert_tag_name(omo_tag_type[i]), val);
							}
						}
					}
				}
				if(track)
				{
					omo_unload_track(track);
				}
				ret = t3net_get_data(url, arguments);
			}
		}
		t3net_destroy_arguments(arguments);
	}

	return ret;
}
