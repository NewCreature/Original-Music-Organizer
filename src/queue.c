#include "t3f/t3f.h"
#include "queue.h"

OMO_QUEUE * omo_create_queue(int files)
{
    OMO_QUEUE * qp = NULL;

    qp = malloc(sizeof(OMO_QUEUE));
    if(qp)
    {
        qp->entry = malloc(sizeof(OMO_QUEUE_ENTRY *) * files);
        if(!qp->entry)
        {
            free(qp);
            qp = NULL;
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
        free(qp->entry[i]);
    }
    free(qp->entry);
    free(qp);
}

bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn, const char * subfn)
{
    if(qp->entry_count < qp->entry_size)
    {
        qp->entry[qp->entry_count] = malloc(sizeof(OMO_QUEUE_ENTRY));
        if(qp->entry[qp->entry_count])
        {
            qp->entry[qp->entry_count]->file = malloc(strlen(fn) + 2);
            if(qp->entry[qp->entry_count]->file)
            {
                strcpy(qp->entry[qp->entry_count]->file, fn);
                qp->entry[qp->entry_count]->sub_file = NULL;
                if(subfn)
                {
                    qp->entry[qp->entry_count]->sub_file = malloc(strlen(subfn) + 2);
                    if(qp->entry[qp->entry_count]->sub_file)
                    {
                        strcpy(qp->entry[qp->entry_count]->sub_file, subfn);
                    }
                }
                qp->entry_count++;
                return true;
            }
        }
    }
    return false;
}
