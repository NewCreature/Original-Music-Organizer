#include "t3f/t3f.h"
#include "t3f/file.h"
#include "library.h"

static const char * omo_library_file_header = "OL01";
static const char * omo_library_artists_file_header = "OAR01";
static const char * omo_library_albums_file_header = "OAL01";
static const char * omo_library_songs_file_header = "OS01";

static bool omo_load_library_entry_f(ALLEGRO_FILE * fp, OMO_LIBRARY * lp)
{
    lp->entry[lp->entry_count] = malloc(sizeof(OMO_LIBRARY_ENTRY));
    if(!lp->entry[lp->entry_count])
    {
        return false;
    }
    lp->entry[lp->entry_count]->filename = t3f_load_string_f(fp);
    if(!lp->entry[lp->entry_count]->filename)
    {
        return false;
    }
    lp->entry[lp->entry_count]->sub_filename = t3f_load_string_f(fp);
    if(!lp->entry[lp->entry_count]->sub_filename)
    {
        return false;
    }
    lp->entry[lp->entry_count]->track = t3f_load_string_f(fp);
    if(!lp->entry[lp->entry_count]->track)
    {
        return false;
    }
    lp->entry[lp->entry_count]->id = t3f_load_string_f(fp);
    if(!lp->entry[lp->entry_count]->id)
    {
        return false;
    }
    lp->entry_count++;
    return true;
}

bool omo_load_library_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp = NULL;
    int i, c;

    fp = al_fopen(fn, "rb");
    if(fp)
    {
        for(i = 0; i < strlen(omo_library_file_header); i++)
        {
            c = al_fgetc(fp);
            if(c != omo_library_file_header[i])
            {
                goto fail;
            }
        }
        c = al_fread32le(fp);
        if(al_feof(fp))
        {
            goto fail;
        }
        if(!omo_allocate_library(lp, c))
        {
            goto fail;
        }
        for(i = 0; i < c; i++)
        {
            if(!omo_load_library_entry_f(fp, lp))
            {
                goto fail;
            }
        }
        al_fclose(fp);
        return true;
    }

    fail:
    {
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

static bool omo_save_library_entry_f(ALLEGRO_FILE * fp, OMO_LIBRARY_ENTRY * ep)
{
    if(!t3f_save_string_f(fp, ep->filename))
    {
        return false;
    }
    if(!t3f_save_string_f(fp, ep->sub_filename))
    {
        return false;
    }
    if(!t3f_save_string_f(fp, ep->track))
    {
        return false;
    }
    if(!t3f_save_string_f(fp, ep->id))
    {
        return false;
    }
    return true;
}

bool omo_save_library_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp;
    int i, l;

    fp = al_fopen(fn, "wb");
    if(!fp)
    {
        goto fail;
    }

    /* write header */
    l = strlen(omo_library_file_header);
    if(al_fwrite(fp, omo_library_file_header, l) != l)
    {
        goto fail;
    }

    /* write library entries */
    if(al_fwrite32le(fp, lp->entry_count) != 4)
    {
        goto fail;
    }
    for(i = 0; i < lp->entry_count; i++)
    {
        if(!omo_save_library_entry_f(fp, lp->entry[i]))
        {
            goto fail;
        }
    }
    al_fclose(fp);

    return true;

    fail:
    {
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_load_library_artists_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp = NULL;
    int i, c;

    fp = al_fopen(fn, "rb");
    if(fp)
    {
        for(i = 0; i < strlen(omo_library_artists_file_header); i++)
        {
            c = al_fgetc(fp);
            if(c != omo_library_artists_file_header[i])
            {
                goto fail;
            }
        }
        lp->artist_entry_size = al_fread32le(fp);
        if(al_feof(fp))
        {
            goto fail;
        }
        lp->artist_entry = malloc(sizeof(unsigned long) * lp->artist_entry_size);
        if(!lp->artist_entry)
        {
            goto fail;
        }
        for(i = 0; i < lp->artist_entry_size; i++)
        {
            lp->artist_entry[i] = t3f_load_string_f(fp);
            if(!lp->artist_entry[i])
            {
                goto fail;
            }
        }
        lp->artist_entry_count = lp->artist_entry_size;
        al_fclose(fp);
        return true;
    }

    fail:
    {
        for(i = 0; i < lp->album_entry_size; i++)
        {
            if(lp->album_entry[i])
            {
                free(lp->album_entry[i]);
            }
        }
        if(lp->album_entry)
        {
            free(lp->song_entry);
        }
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_save_library_artists_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp;
    int i, l;

    fp = al_fopen(fn, "wb");
    if(!fp)
    {
        goto fail;
    }

    /* write header */
    l = strlen(omo_library_artists_file_header);
    if(al_fwrite(fp, omo_library_artists_file_header, l) != l)
    {
        goto fail;
    }

    /* write artists list */
    if(al_fwrite32le(fp, lp->artist_entry_count) != 4)
    {
        goto fail;
    }
    for(i = 0; i < lp->artist_entry_count; i++)
    {
        if(!t3f_save_string_f(fp, lp->artist_entry[i]))
        {
            goto fail;
        }
    }
    al_fclose(fp);
    return true;

    fail:
    {
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_load_library_albums_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp = NULL;
    int i, c;

    fp = al_fopen(fn, "rb");
    if(fp)
    {
        for(i = 0; i < strlen(omo_library_albums_file_header); i++)
        {
            c = al_fgetc(fp);
            if(c != omo_library_albums_file_header[i])
            {
                goto fail;
            }
        }
        lp->album_entry_size = al_fread32le(fp);
        if(al_feof(fp))
        {
            goto fail;
        }
        lp->album_entry = malloc(sizeof(unsigned long) * lp->album_entry_size);
        if(!lp->album_entry)
        {
            goto fail;
        }
        for(i = 0; i < lp->album_entry_size; i++)
        {
            lp->album_entry[i] = t3f_load_string_f(fp);
            if(!lp->album_entry[i])
            {
                goto fail;
            }
        }
        lp->album_entry_count = lp->album_entry_size;
        al_fclose(fp);
        return true;
    }

    fail:
    {
        for(i = 0; i < lp->album_entry_size; i++)
        {
            if(lp->album_entry[i])
            {
                free(lp->album_entry[i]);
            }
        }
        if(lp->album_entry)
        {
            free(lp->song_entry);
        }
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_save_library_albums_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp;
    int i, l;

    fp = al_fopen(fn, "wb");
    if(!fp)
    {
        goto fail;
    }

    /* write header */
    l = strlen(omo_library_albums_file_header);
    if(al_fwrite(fp, omo_library_albums_file_header, l) != l)
    {
        goto fail;
    }

    /* write artists list */
    if(al_fwrite32le(fp, lp->album_entry_count) != 4)
    {
        goto fail;
    }
    for(i = 0; i < lp->album_entry_count; i++)
    {
        if(!t3f_save_string_f(fp, lp->album_entry[i]))
        {
            goto fail;
        }
    }
    al_fclose(fp);
    return true;

    fail:
    {
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_load_library_songs_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp = NULL;
    int i, c;

    fp = al_fopen(fn, "rb");
    if(fp)
    {
        for(i = 0; i < strlen(omo_library_songs_file_header); i++)
        {
            c = al_fgetc(fp);
            if(c != omo_library_songs_file_header[i])
            {
                goto fail;
            }
        }
        lp->song_entry_size = al_fread32le(fp);
        if(al_feof(fp))
        {
            goto fail;
        }
        lp->song_entry = malloc(sizeof(unsigned long) * lp->song_entry_size);
        if(!lp->song_entry)
        {
            goto fail;
        }
        for(i = 0; i < lp->song_entry_size; i++)
        {
            lp->song_entry[i] = al_fread32le(fp);
            if(al_feof(fp))
            {
                goto fail;
            }
        }
        lp->song_entry_count = lp->song_entry_size;
        al_fclose(fp);
        return true;
    }

    fail:
    {
        if(lp->song_entry)
        {
            free(lp->song_entry);
        }
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}

bool omo_save_library_songs_cache(OMO_LIBRARY * lp, const char * fn)
{
    ALLEGRO_FILE * fp;
    int i, l;

    fp = al_fopen(fn, "wb");
    if(!fp)
    {
        goto fail;
    }

    /* write header */
    l = strlen(omo_library_songs_file_header);
    if(al_fwrite(fp, omo_library_songs_file_header, l) != l)
    {
        goto fail;
    }

    /* write artists list */
    if(al_fwrite32le(fp, lp->song_entry_count) != 4)
    {
        goto fail;
    }
    for(i = 0; i < lp->song_entry_count; i++)
    {
        if(al_fwrite32le(fp, lp->song_entry[i]) != 4)
        {
            goto fail;
        }
    }
    al_fclose(fp);
    return true;

    fail:
    {
        if(fp)
        {
            al_fclose(fp);
        }
    }
    return false;
}
