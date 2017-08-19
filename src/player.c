#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "player.h"
#include "queue.h"

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
    if(pp->codec_handler)
    {
        pp->codec_handler->stop();
        if(pp->codec_handler->unload_file)
        {
            pp->codec_handler->unload_file();
        }
        pp->codec_handler = NULL;

        /* delete previously extracted file if we played from archive */
        if(strlen(pp->extracted_filename) > 0)
        {
            al_remove_filename(pp->extracted_filename);
        }
    }
}

void omo_stop_player(OMO_PLAYER * pp)
{
    if(pp->state != OMO_PLAYER_STATE_STOPPED)
    {
        omo_stop_player_playback(pp);
        pp->queue_pos = pp->queue->entry_count;
	}
}

void omo_pause_player(OMO_PLAYER * pp)
{
    if(pp->codec_handler)
    {
        if(pp->codec_handler->pause())
        {
            pp->state = OMO_PLAYER_STATE_PAUSED;
        }
    }
}

void omo_resume_player(OMO_PLAYER * pp)
{
    if(pp->codec_handler)
    {
        if(pp->codec_handler->resume())
        {
            pp->state = OMO_PLAYER_STATE_PLAYING;
        }
    }
}

bool omo_play_previous_song(OMO_PLAYER * pp)
{
    if(pp->queue)
    {
        if(pp->codec_handler)
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
    if(pp->codec_handler)
    {
        omo_stop_player_playback(pp);
        pp->queue_pos++;
    }
    return true;
}

void omo_player_logic(OMO_PLAYER * pp, OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry)
{
    OMO_ARCHIVE_HANDLER * archive_handler;
	const char * subfile;
    bool next_file = false;

    if(pp->queue && pp->state == OMO_PLAYER_STATE_PLAYING)
    {
        if(pp->codec_handler)
        {
            if(pp->codec_handler->done_playing())
            {
                omo_stop_player_playback(pp);
                pp->queue_pos++;
                next_file = true;
            }
        }
        else
        {
            next_file = true;
        }
        if(next_file)
        {
            while(1)
            {
                if(pp->queue_pos < pp->queue->entry_count)
                {
                    archive_handler = omo_get_archive_handler(archive_handler_registry, pp->queue->entry[pp->queue_pos]->file);
                    if(archive_handler)
                    {
                        strcpy(pp->extracted_filename, "");
                        if(pp->queue->entry[pp->queue_pos]->sub_file)
                        {
                            pp->codec_handler = omo_get_codec_handler(codec_handler_registry, archive_handler->get_file(pp->queue->entry[pp->queue_pos]->file, atoi(pp->queue->entry[pp->queue_pos]->sub_file)));
                            if(pp->codec_handler)
                            {
                                subfile = archive_handler->extract_file(pp->queue->entry[pp->queue_pos]->file, atoi(pp->queue->entry[pp->queue_pos]->sub_file));
                                if(strlen(subfile) > 0)
                                {
                                    strcpy(pp->extracted_filename, subfile);
                                    if(pp->codec_handler->load_file(pp->extracted_filename, pp->queue->entry[pp->queue_pos]->track))
                                    {
                                        if(pp->codec_handler->play())
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        pp->codec_handler = omo_get_codec_handler(codec_handler_registry, pp->queue->entry[pp->queue_pos]->file);
                        if(pp->codec_handler)
                        {
                            if(pp->codec_handler->load_file(pp->queue->entry[pp->queue_pos]->file, pp->queue->entry[pp->queue_pos]->track))
                            {
                                if(pp->codec_handler->play())
                                {
                                    break;
                                }
                            }
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
}
