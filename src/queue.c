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
    id1 = al_get_config_value(library->file_database, buf, "id");
    sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
    id2 = al_get_config_value(library->file_database, buf, "id");

    if(id1 && id2)
    {
        for(i = 0; i < 5; i++)
        {
            val1 = al_get_config_value(library->entry_database, id1, sort_field[i]);
            val2 = al_get_config_value(library->entry_database, id2, sort_field[i]);
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
    id1 = al_get_config_value(library->file_database, buf, "id");
    sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
    id2 = al_get_config_value(library->file_database, buf, "id");

    if(id1 && id2)
    {
        for(i = 0; i < 2; i++)
        {
            val1 = al_get_config_value(library->entry_database, id1, sort_field[i]);
            val2 = al_get_config_value(library->entry_database, id2, sort_field[i]);
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
    id1 = al_get_config_value(library->file_database, buf, "id");
    sprintf(buf, "%s%s%s", (*entry2)->file, (*entry2)->sub_file ? "/" : "", (*entry2)->sub_file ? (*entry2)->sub_file : "");
    id2 = al_get_config_value(library->file_database, buf, "id");

    if(id1 && id2)
    {
        for(i = 0; i < 1; i++)
        {
            val1 = al_get_config_value(library->entry_database, id1, sort_field[i]);
            val2 = al_get_config_value(library->entry_database, id2, sort_field[i]);
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

void omo_get_queue_entry_tags(OMO_QUEUE * qp, int i, OMO_LIBRARY * lp)
{
    char section[1024];
    const char * val;
    const char * artist = NULL;
    const char * album = NULL;
    const char * title = NULL;
    const char * track = NULL;
    const char * extracted_fn = NULL;

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
        val = al_get_config_value(lp->file_database, section, "id");
        if(val)
        {
            artist = al_get_config_value(lp->entry_database, val, "Artist");
            album = al_get_config_value(lp->entry_database, val, "Album");
            title = al_get_config_value(lp->entry_database, val, "Title");
            track = al_get_config_value(lp->entry_database, val, "Track");
            if(artist)
            {
                strcpy(qp->entry[i]->tags.artist, artist);
            }
            if(album)
            {
                strcpy(qp->entry[i]->tags.album, album);
            }
            if(title)
            {
                strcpy(qp->entry[i]->tags.title, title);
            }
            if(track)
            {
                strcpy(qp->entry[i]->tags.track, track);
            }
        }
    }
}

void omo_get_queue_tags(OMO_QUEUE * qp, OMO_LIBRARY * lp)
{
    int i;

    if(qp)
    {
        for(i = 0; i < qp->entry_count; i++)
        {
            omo_get_queue_entry_tags(qp, i, lp);
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
