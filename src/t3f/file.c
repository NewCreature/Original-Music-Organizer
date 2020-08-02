#include "t3f.h"

size_t t3f_file_size(const char * fn)
{
	ALLEGRO_FS_ENTRY * fp;
	size_t size = 0;

	fp = al_create_fs_entry(fn);
	if(fp)
	{
		size = al_get_fs_entry_size(fp);
		al_destroy_fs_entry(fp);
	}
	return size;
}

time_t t3f_get_file_mtime(const char * fn)
{
	ALLEGRO_FS_ENTRY * fp;
	time_t mtime = 0;

	fp = al_create_fs_entry(fn);
	if(fp)
	{
		mtime = al_get_fs_entry_mtime(fp);
		al_destroy_fs_entry(fp);
	}
	return mtime;
}

float t3f_fread_float(ALLEGRO_FILE * fp)
{
	char buffer[256] = {0};
	int l;

	l = al_fgetc(fp);
	al_fread(fp, buffer, l);
	buffer[l] = '\0';

	return atof(buffer);
}

bool t3f_fwrite_float(ALLEGRO_FILE * fp, float f)
{
	char buffer[256] = {0};
	int l;

	sprintf(buffer, "%f", f);
	l = strlen(buffer);
	al_fputc(fp, l);
	al_fwrite(fp, buffer, l);

	return true;
}

char * t3f_load_string_f(ALLEGRO_FILE * fp)
{
    char * sp;
    short l;

    l = al_fread16le(fp);
    if(al_feof(fp))
    {
        return NULL;
    }
    sp = malloc(l + 1);
    if(!sp)
    {
        return NULL;
    }
    al_fread(fp, sp, l);
    sp[l] = '\0';

    return sp;
}

bool t3f_save_string_f(ALLEGRO_FILE * fp, const char * sp)
{
    short l;

    if(!sp)
    {
        if(al_fwrite16le(fp, 0) < 2)
        {
            return false;
        }
        return true;
    }
    l = strlen(sp);
    if(al_fwrite16le(fp, l) < 2)
    {
        return false;
    }
    if(al_fwrite(fp, sp, l) != l)
    {
        return false;
    }
    return true;
}
