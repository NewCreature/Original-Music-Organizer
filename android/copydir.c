#include <allegro5/allegro5.h>

static char base_path[4096] = {0};
static char rpath[4096] = {0};

const char * sub_path(const char * path)
{
    return &path[strlen(base_path)];
}

const char * dest_path(const char * path, const char * dest)
{
    sprintf(rpath, "%s/%s", dest, sub_path(path));
    return rpath;
}

bool copy_file(const char * path, const char * dest_path)
{
    ALLEGRO_FILE * src;
    ALLEGRO_FILE * dest;
    bool ret = false;
    int i;

    src = al_fopen(path, "rb");
    if(src)
    {
        dest = al_fopen(dest_path, "wb");
        if(dest)
        {
            for(i = 0; i < al_fsize(src); i++)
            {
                al_fputc(dest, al_fgetc(src));
            }
            al_fclose(dest);
            ret = true;
        }
        al_fclose(src);
    }
    return ret;
}

bool copy_dir(const char * path, const char * dest)
{
    ALLEGRO_FS_ENTRY * fs_entry;
    ALLEGRO_FS_ENTRY * directory_entry;
    char buf[1024] = {0};

    fs_entry = al_create_fs_entry(path);
    if(!fs_entry)
    {
        return false;
    }
    if(!al_open_directory(fs_entry))
    {
        al_destroy_fs_entry(fs_entry);
        return false;
    }

    /* get the base path if we don't have it yet */
    if(strlen(base_path) <= 0)
    {
        sprintf(base_path, "%s/", al_get_fs_entry_name(fs_entry));
    }

    /* create directory in target path */
    strcpy(buf, al_get_fs_entry_name(fs_entry));
    al_make_directory(dest_path(buf, dest));
    printf("\nCopying directory %s:\n\n", sub_path(buf));

    while(1)
    {
        directory_entry = al_read_directory(fs_entry);
        if(directory_entry)
        {
            strcpy(buf, al_get_fs_entry_name(directory_entry));
            if(al_get_fs_entry_mode(directory_entry) & ALLEGRO_FILEMODE_ISDIR)
            {
                copy_dir(al_get_fs_entry_name(directory_entry), dest);
            }
            else
            {
                printf("\t%s\n", sub_path(buf));
                copy_file(al_get_fs_entry_name(directory_entry), dest_path(buf, dest));
            }
        }
        else
        {
            break;
        }
    }
    al_destroy_fs_entry(fs_entry);
    return true;
}

int main(int argc, char * argv[])
{
    if(argc < 3)
    {
        printf("Usage: copydir <source path> <destination_path>\n");
    }
    copy_dir(argv[1], argv[2]);
    printf("\n");
    return 0;
}
