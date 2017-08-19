#include "t3f.h"

bool t3f_scan_files(const char * path, bool (*process_file)(const char * fn, void * data), bool subdir, void (*update_proc)(const char * fn, void * data), void * data)
{
	ALLEGRO_FS_ENTRY * dir;
	ALLEGRO_FS_ENTRY * fp;
	char cname[1024] = {0};

	/* ignore ./ and ../ path entries */
	strcpy(cname, path);
    if(strlen(cname) > 0)
    {
        if(cname[strlen(cname) - 1] == '/')
        {
            if(subdir)
            {
                if(cname[strlen(cname) - 2] == '.')
                {
                    return false;
                }
            }
            cname[strlen(cname) - 1] = 0;
        }
    }

//	printf("!Looking in %s\n", cname);
	dir = al_create_fs_entry(cname);
	if(!dir)
	{
		return false;
	}
	if(!al_open_directory(dir))
	{
		return false;
	}
//	printf("Looking in %s\n", cname);
	while(1)
	{
		fp = al_read_directory(dir);
		if(!fp)
		{
			break;
		}
//		name = al_path_to_string(al_get_entry_name(fp), '/');
		if(al_get_fs_entry_mode(fp) & ALLEGRO_FILEMODE_ISDIR)
		{
			t3f_scan_files(al_get_fs_entry_name(fp), process_file, true, update_proc, data);
		}
		else
		{
            if(process_file(al_get_fs_entry_name(fp), data))
			{
				if(update_proc)
				{
					update_proc(al_get_fs_entry_name(fp), data);
				}
			}
		}
		al_destroy_fs_entry(fp);
	}
	al_destroy_fs_entry(dir);
    return true;
}
