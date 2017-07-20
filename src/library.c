#include "t3f/t3f.h"

#include "library.h"
#include "md5.h"

OMO_LIBRARY * omo_create_library(const char * file_db_fn, const char * entry_db_fn, int total_files)
{
    OMO_LIBRARY * lp = NULL;

    lp = malloc(sizeof(OMO_LIBRARY));
    if(lp)
    {
        memset(lp, 0, sizeof(OMO_LIBRARY));
        lp->entry = malloc(sizeof(OMO_LIBRARY_ENTRY *) * total_files);
        if(lp->entry)
        {
            lp->entry_size = total_files;
            lp->entry_count = 0;
            lp->file_database_fn = malloc(strlen(file_db_fn) + 1);
            if(!lp->file_database_fn)
            {
                goto fail;
            }
            strcpy(lp->file_database_fn, file_db_fn);
            lp->file_database = al_load_config_file(file_db_fn);
            if(!lp->file_database)
            {
                lp->file_database = al_create_config();
                if(!lp->file_database)
                {
                    goto fail;
                }
            }
            lp->entry_database_fn = malloc(strlen(entry_db_fn) + 1);
            if(!lp->entry_database_fn)
            {
                goto fail;
            }
            strcpy(lp->entry_database_fn, entry_db_fn);
            lp->entry_database = al_load_config_file(entry_db_fn);
            if(!lp->entry_database)
            {
                lp->entry_database = al_create_config();
                if(!lp->entry_database)
                {
                    goto fail;
                }
            }
        }
    }
    return lp;

    fail:
    {
        if(lp)
        {
            if(lp->entry)
            {
                if(lp->file_database_fn)
                {
                    free(lp->file_database_fn);
                }
                if(lp->file_database)
                {
                    al_destroy_config(lp->file_database);
                }
                if(lp->entry_database_fn)
                {
                    free(lp->entry_database_fn);
                }
                if(lp->entry_database)
                {
                    al_destroy_config(lp->entry_database);
                }
                free(lp->entry);
            }
            free(lp);
        }
    }
    return NULL;
}

void omo_destroy_library(OMO_LIBRARY * lp)
{
    int i;

    for(i = 0; i < lp->entry_count; i++)
    {
        if(lp->entry[i]->filename)
        {
            free(lp->entry[i]->filename);
        }
        if(lp->entry[i]->id)
        {
            free(lp->entry[i]->id);
        }
        free(lp->entry[i]);
    }
    free(lp->file_database_fn);
    al_destroy_config(lp->file_database);
    free(lp->entry_database_fn);
    al_destroy_config(lp->entry_database);
    free(lp->entry);
    free(lp);
}

bool omo_save_library(OMO_LIBRARY * lp)
{
    bool ret = true;

    if(!al_save_config_file(lp->file_database_fn, lp->file_database))
    {
        ret = false;
    }
    if(!al_save_config_file(lp->entry_database_fn, lp->entry_database))
    {
        ret = false;
    }
    return ret;
}

bool omo_add_file_to_library(OMO_LIBRARY * lp, const char * fn, const char * subfn)
{
    const char * val;
    uint32_t h[4];
    char sum_string[128];
    char section[1024];

    if(lp->entry_count < lp->entry_size)
    {
        sprintf(section, "%s%s%s", fn, subfn ? "/" : "", subfn ? subfn : "");
        val = al_get_config_value(lp->file_database, section, "id");
        if(!val)
        {
            md5_file(fn, h);
            sprintf(sum_string, "%04x%04x%04x%04x%s", h[0], h[1], h[2], h[3], subfn ? subfn : "");
            al_set_config_value(lp->file_database, section, "id", sum_string);
            if(subfn)
            {
                al_set_config_value(lp->file_database, section, "subfn", subfn);
            }
        }
        lp->entry[lp->entry_count] = malloc(sizeof(OMO_LIBRARY_ENTRY));
        if(lp->entry[lp->entry_count])
        {
            lp->entry[lp->entry_count]->filename = malloc(strlen(fn) + 1);
            if(lp->entry[lp->entry_count]->filename)
            {
                strcpy(lp->entry[lp->entry_count]->filename, fn);
                return true;
            }
        }
    }
    return false;
}
