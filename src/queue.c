#include "t3f/t3f.h"
#include "queue.h"

OMO_QUEUE * omo_create_queue(int files)
{
    OMO_QUEUE * qp = NULL;

    qp = malloc(sizeof(OMO_QUEUE));
    if(qp)
    {
        qp->file = malloc(sizeof(char *) * files);
        if(!qp->file)
        {
            free(qp);
            qp = NULL;
        }
        else
        {
            qp->file_size = files;
            qp->file_count = 0;
        }
    }
    return qp;
}

void omo_destroy_queue(OMO_QUEUE * qp)
{
    int i;

    for(i = 0; i < qp->file_count; i++)
    {
        free(qp->file[i]);
    }
    free(qp->file);
    free(qp);
}

bool omo_add_file_to_queue(OMO_QUEUE * qp, const char * fn)
{
    if(qp->file_count < qp->file_size)
    {
        qp->file[qp->file_count] = malloc(strlen(fn) + 2);
        if(qp->file[qp->file_count])
        {
            strcpy(qp->file[qp->file_count], fn);
            qp->file_count++;
            return true;
        }
    }
    return false;
}
