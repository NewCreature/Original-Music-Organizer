#include "t3f.h"

bool t3f_scan_files(const char * path, bool (*process_file)(const char * fn, bool isfolder, void * data), bool subdir, void * data)
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
			if(process_file(al_get_fs_entry_name(fp), true, data))
			{
			}
			t3f_scan_files(al_get_fs_entry_name(fp), process_file, true, data);
		}
		else
		{
            if(process_file(al_get_fs_entry_name(fp), false, data))
			{
			}
		}
		al_destroy_fs_entry(fp);
	}
	al_destroy_fs_entry(dir);
    return true;
}

typedef struct
{

	char ** path;
	int path_count;
	int current_path;

} T3F_REMOVE_DIRECTORY_DATA;

static T3F_REMOVE_DIRECTORY_DATA * t3f_create_remove_directory_data(void)
{
	T3F_REMOVE_DIRECTORY_DATA * dir_data;

	dir_data = malloc(sizeof(T3F_REMOVE_DIRECTORY_DATA));
	if(dir_data)
	{
		dir_data->path = NULL;
		dir_data->path_count = 0;
		dir_data->current_path = 0;
	}
	return dir_data;
}

static bool t3f_allocate_directory_data_path(T3F_REMOVE_DIRECTORY_DATA * dir_data)
{
	if(dir_data->path_count > 0)
	{
		dir_data->path = malloc(sizeof(char *) * dir_data->path_count);
		if(dir_data->path)
		{
			return true;
		}
	}
	return false;
}

static void t3f_destroy_remove_directory_data(T3F_REMOVE_DIRECTORY_DATA * dir_data)
{
	int i;

	if(dir_data->path)
	{
		for(i = 0; i < dir_data->path_count; i++)
		{
			free(dir_data->path[i]);
		}
		free(dir_data->path);
	}
	free(dir_data);
}

static bool delete_file(const char * fn, bool isfolder, void * data)
{
	if(!isfolder)
	{
		al_remove_filename(fn);
	}
	return true;
}

static bool count_path(const char * fn, bool isfolder, void * data)
{
	T3F_REMOVE_DIRECTORY_DATA * dir_data = (T3F_REMOVE_DIRECTORY_DATA *)data;

	dir_data->path_count++;
	return true;
}

static bool add_path(const char * fn, bool isfolder, void * data)
{
	T3F_REMOVE_DIRECTORY_DATA * dir_data = (T3F_REMOVE_DIRECTORY_DATA *)data;

	dir_data->path[dir_data->current_path] = malloc(strlen(fn) + 1);
	if(dir_data->path[dir_data->current_path])
	{
		dir_data->current_path++;
		return true;
	}
	return false;
}

static int sort_path(const void * p1, const void * p2)
{
	const char * s1 = (const char *)*(void **)p1;
	const char * s2 = (const char *)*(void **)p2;

	if(strlen(s1) > strlen(s2))
	{
		return -1;
	}
	else if(strlen(s2) > strlen(s1))
	{
		return 1;
	}
	return 0;
}

bool t3f_remove_directory(const char * path)
{
	T3F_REMOVE_DIRECTORY_DATA * dir_data;
	bool ret = false;
	int i;

	/* delete files */
	t3f_scan_files(path, delete_file, false, NULL);

	/* create list of remaining folders, sort by length, and delete paths starting
	   with the longest path names */
	dir_data = t3f_create_remove_directory_data();
	if(dir_data)
	{
		t3f_scan_files(path, count_path, false, dir_data);
		if(t3f_allocate_directory_data_path(dir_data))
		{
			t3f_scan_files(path, add_path, false, dir_data);
			qsort(dir_data->path, dir_data->path_count, sizeof(char *), sort_path);
			for(i = 0; i < dir_data->path_count; i++)
			{
				al_remove_filename(dir_data->path[i]);
			}
			ret = true;
		}
		t3f_destroy_remove_directory_data(dir_data);
	}
	al_remove_filename(path);
	return ret;
}

const char * t3f_get_path_filename(const char * path)
{
	int i, l;

	l = strlen(path);
	for(i = strlen(path) - 1; i >= 0; i--)
	{
		if(path[i] == '/' || path[i] == '\\')
		{
			if(i + 1 >= l)
			{
				return NULL;
			}
			return &path[i + 1];
		}
	}
	if(l > 0)
	{
		return path;
	}
	return NULL;
}

const char * t3f_get_path_extension(const char * path)
{
	int i;

	for(i = strlen(path) - 1; i >= 0; i--)
	{
		if(path[i] == '.')
		{
			return &path[i];
		}
	}
	return path;
}
