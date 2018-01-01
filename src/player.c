#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "player.h"
#include "queue.h"
#include "library_helpers.h"

OMO_PLAYER * omo_create_player(void)
{
	OMO_PLAYER * pp;

	pp = malloc(sizeof(OMO_PLAYER));
	if(pp)
	{
		memset(pp, 0, sizeof(OMO_PLAYER));
	}
	return pp;
}
void omo_destroy_player(OMO_PLAYER * pp)
{
	if(pp->queue)
	{
		omo_destroy_queue(pp->queue);
	}
	free(pp);
}

bool omo_start_player(OMO_PLAYER * pp)
{
	pp->state = OMO_PLAYER_STATE_PLAYING;
	return true;
}

static void omo_stop_player_playback(OMO_PLAYER * pp)
{
	if(pp->track)
	{
		pp->track->codec_handler->stop(pp->track->codec_data);
		omo_unload_track(pp->track);
		pp->track = NULL;
	}
}

void omo_stop_player(OMO_PLAYER * pp)
{
	if(pp->state != OMO_PLAYER_STATE_STOPPED)
	{
		omo_stop_player_playback(pp);
		pp->state = OMO_PLAYER_STATE_STOPPED;
	}
}

void omo_pause_player(OMO_PLAYER * pp)
{
	if(pp->track)
	{
		if(pp->track->codec_handler->pause(pp->track->codec_data))
		{
			pp->state = OMO_PLAYER_STATE_PAUSED;
		}
	}
}

void omo_resume_player(OMO_PLAYER * pp)
{
	if(pp->track)
	{
		if(pp->track->codec_handler->resume(pp->track->codec_data))
		{
			pp->state = OMO_PLAYER_STATE_PLAYING;
		}
	}
}

bool omo_play_previous_song(OMO_PLAYER * pp)
{
	if(pp->queue)
	{
		if(pp->track)
		{
			omo_stop_player_playback(pp);
		}
		pp->queue_pos--;
		if(pp->queue_pos < 0)
		{
			pp->queue_pos = 0;
		}
	}
	return true;
}

bool omo_play_next_song(OMO_PLAYER * pp)
{
	if(pp->track)
	{
		omo_stop_player_playback(pp);
		pp->queue_pos++;
	}
	return true;
}

static bool omo_player_play_file(OMO_PLAYER * pp, double loop_start, double loop_end, double fade_time, int loop_count, double force_length)
{
	const char * val;

	if(pp->track->codec_handler->set_loop)
	{
		if(loop_end > loop_start)
		{
			pp->track->codec_handler->set_loop(pp->track->codec_data, loop_start, loop_end, fade_time, loop_count);
		}
	}
	if(pp->track->codec_handler->set_length)
	{
		if(force_length > 0.0)
		{
			pp->track->codec_handler->set_length(pp->track->codec_data, force_length);
		}
	}
	if(pp->track->codec_handler->set_volume)
	{
		val = al_get_config_value(t3f_config, "Settings", "volume");
		if(val)
		{
			pp->track->codec_handler->set_volume(pp->track->codec_data, atof(val));
		}
	}
	al_stop_timer(t3f_timer);
	if(pp->track->codec_handler->play(pp->track->codec_data))
	{
		pp->state = OMO_PLAYER_STATE_PLAYING;
		al_start_timer(t3f_timer);
		return true;
	}
	else
	{
		omo_unload_track(pp->track);
		pp->track = NULL;
		al_start_timer(t3f_timer);
	}
	return false;
}

void omo_player_logic(OMO_PLAYER * pp, OMO_LIBRARY * lp, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, ALLEGRO_PATH * temp_path)
{
	bool next_file = false;
	const char * val;
	const char * id;
	double loop_start = 0.0;
	double loop_end = 0.0;
	double fade_time = 0.0;
	double current_time = -1.0;
	double force_length = 0.0;
	char buf[256];

	if(pp->queue && pp->state == OMO_PLAYER_STATE_PLAYING)
	{
		if(pp->track)
		{
			if(pp->track->codec_handler->get_position)
			{
				current_time = pp->track->codec_handler->get_position(pp->track->codec_data);
			}
			if(pp->track->codec_handler->done_playing(pp->track->codec_data))
			{
				if(pp->track->codec_handler->get_length)
				{
					if(current_time < pp->track->codec_handler->get_length(pp->track->codec_data) - 5.0)
					{
						id = omo_get_library_file_id(lp, pp->queue->entry[pp->queue_pos]->file, pp->queue->entry[pp->queue_pos]->sub_file, pp->queue->entry[pp->queue_pos]->track);
						if(id)
						{
							sprintf(buf, "%f", current_time);
							omo_set_database_value(lp->entry_database, id, "Detected Length", buf);
							omo_set_database_value(lp->entry_database, id, "Submitted", "false");
						}
					}
				}
				al_stop_timer(t3f_timer);
				omo_stop_player_playback(pp);
				pp->state = OMO_PLAYER_STATE_FINISHED;
				pp->queue_pos++;
				next_file = true;
				al_start_timer(t3f_timer);
			}
		}
		else
		{
			next_file = true;
		}
	}
	if(next_file)
	{
		while(1)
		{
			if(pp->queue_pos < pp->queue->entry_count)
			{
				al_stop_timer(t3f_timer);
				pp->track = omo_load_track(archive_handler_registry, codec_handler_registry, pp->queue->entry[pp->queue_pos]->file, pp->queue->entry[pp->queue_pos]->sub_file, pp->queue->entry[pp->queue_pos]->track, temp_path);
				al_start_timer(t3f_timer);
				if(pp->track)
				{
					if(lp)
					{
						id = omo_get_library_file_id(lp, pp->queue->entry[pp->queue_pos]->file, pp->queue->entry[pp->queue_pos]->sub_file, pp->queue->entry[pp->queue_pos]->track);
						if(id)
						{
							val = omo_get_database_value(lp->entry_database, id, "Detected Length");
							if(val)
							{
								force_length = atof(val);
							}
							val = omo_get_database_value(lp->entry_database, id, "Loop Start");
							if(val)
							{
								loop_start = atof(val);
							}
							val = omo_get_database_value(lp->entry_database, id, "Loop End");
							if(val)
							{
								loop_end = atof(val);
							}
							val = omo_get_database_value(lp->entry_database, id, "Fade Time");
							if(val)
							{
								fade_time = atof(val);
							}
						}
					}
					if(omo_player_play_file(pp, loop_start, loop_end, fade_time, 1, force_length))
					{
						break;
					}
				}
			}
			else
			{
				pp->state = OMO_PLAYER_STATE_STOPPED;
				break;
			}
			pp->queue_pos++;
		}
	}
}
