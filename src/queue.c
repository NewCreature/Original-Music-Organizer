#include "t3f/t3f.h"
#include "instance.h"
#include "queue.h"

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

bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn, const char * subfn, const char * track)
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
                if(subfn)
                {
                    qp->entry[qp->entry_count]->sub_file = malloc(strlen(subfn) + 1);
                    if(qp->entry[qp->entry_count]->sub_file)
                    {
                        strcpy(qp->entry[qp->entry_count]->sub_file, subfn);
                    }
                }
                qp->entry[qp->entry_count]->track = NULL;
                if(track)
                {
                    qp->entry[qp->entry_count]->track = malloc(strlen(track) + 1);
                    if(qp->entry[qp->entry_count]->track)
                    {
                        strcpy(qp->entry[qp->entry_count]->track, track);
                    }
                }
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
