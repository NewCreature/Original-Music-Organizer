#include "t3f.h"
#include "resource.h"

static T3F_RESOURCE * t3f_resource[T3F_MAX_RESOURCES] = {NULL};
static int t3f_resources = 0;

bool t3f_bitmap_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy)
{
	ALLEGRO_BITMAP * bitmap = (ALLEGRO_BITMAP *)*ptr;
	ALLEGRO_STATE old_state;
	bool openfp = false; // operating on already open file

	if(destroy)
	{
		al_destroy_bitmap(bitmap);
		return true;
	}

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(al_get_new_bitmap_flags() | ALLEGRO_NO_PRESERVE_TEXTURE);
	if(fp)
	{
		openfp = true;
	}
	if(!openfp && offset == 0)
	{
		bitmap = al_load_bitmap(filename);
	}
	else
	{
		if(!openfp)
		{
			fp = al_fopen(filename, "rb");
			al_fseek(fp, offset, ALLEGRO_SEEK_SET);
		}
		if(fp)
		{
			if(option == 0)
			{
				bitmap = t3f_load_bitmap_f(fp);
			}
			else
			{
				bitmap = al_load_bitmap_f(fp, ".png");
			}
			if(!openfp)
			{
				al_fclose(fp);
			}
		}
	}
	*ptr = bitmap;
	al_restore_state(&old_state);
	return *ptr;
}

bool t3f_font_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy)
{
	ALLEGRO_STATE old_state;
	bool openfp = false;

	if(destroy)
	{
		t3f_destroy_font(*ptr);
		return true;
	}

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(al_get_new_bitmap_flags() | ALLEGRO_NO_PRESERVE_TEXTURE);
	if(fp)
	{
		openfp = true;
	}

	/* load file directly if offset is 0 and open file not passed */
	else if(offset == 0)
	{
		*ptr = t3f_load_font(filename, T3F_FONT_TYPE_AUTO, option, flags);
	}
	else
	{
		if(!openfp)
		{
			fp = al_fopen(filename, "rb");
			if(fp)
			{
				al_fseek(fp, offset, ALLEGRO_SEEK_SET);
			}
		}
		if(fp)
		{
			*ptr = t3f_load_font_f(filename, fp, T3F_FONT_TYPE_AUTO, option, flags);
			if(!openfp)
			{
				al_fclose(fp);
			}
		}
	}
	al_restore_state(&old_state);
	return *ptr;
}

static bool t3f_add_resource(bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), void ** ptr, const char * filename, int option, int flags, unsigned long offset, const ALLEGRO_FILE_INTERFACE * fi)
{
	if(t3f_resources < T3F_MAX_RESOURCES)
	{
		t3f_resource[t3f_resources] = al_malloc(sizeof(T3F_RESOURCE));
		if(t3f_resource[t3f_resources])
		{
			t3f_resource[t3f_resources]->proc = proc;
			t3f_resource[t3f_resources]->ptr = ptr;
			strcpy(t3f_resource[t3f_resources]->filename, filename);
			t3f_resource[t3f_resources]->offset = offset;
			t3f_resource[t3f_resources]->option = option;
			t3f_resource[t3f_resources]->flags = flags;
			t3f_resource[t3f_resources]->fi = fi;
			t3f_resources++;
			return true;
		}
	}
	return false;
}

void t3f_remove_resource(int i)
{
	int j;

	al_free(t3f_resource[i]);
	for(j = i; j < t3f_resources - 1; j++)
	{
		t3f_resource[j] = t3f_resource[j + 1];
	}
	t3f_resources--;
}

void * t3f_load_resource(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), const char * filename, int option, int flags, unsigned long offset)
{
	if(proc)
	{
		proc(ptr, NULL, filename, option, flags, offset, false);
		if(*ptr)
		{
			t3f_add_resource(proc, ptr, filename, option, flags, offset, al_get_new_file_interface());
		}
	}
	return *ptr;
}

void * t3f_load_resource_f(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), ALLEGRO_FILE * fp, const char * filename, int option, int flags)
{
	unsigned long offset;
	if(proc)
	{
		offset = al_ftell(fp);
		proc(ptr, fp, filename, option, flags, offset, false);
		if(*ptr)
		{
			t3f_add_resource(proc, ptr, filename, option, flags, offset, al_get_new_file_interface());
		}
	}
	return *ptr;
}

static void t3f_actually_unload_resource(int i)
{
	if(t3f_resource[i]->proc)
	{
		t3f_resource[i]->proc(t3f_resource[i]->ptr, NULL, NULL, 0, 0, 0, true);
		*t3f_resource[i]->ptr = NULL;
	}
}

int t3f_unload_resource(void * ptr)
{
	int i;

	for(i = 0; i < t3f_resources; i++)
	{
		if(*t3f_resource[i]->ptr == ptr)
		{
			t3f_actually_unload_resource(i);
			return i;
		}
	}
	return -1;
}

bool t3f_destroy_resource(void * ptr)
{
	int i;

	i = t3f_unload_resource(ptr);
	if(i >= 0)
	{
		t3f_remove_resource(i);
		return true;
	}
	return false;
}

void t3f_unload_resources(void)
{
	int i;

	for(i = 0; i < t3f_resources; i++)
	{
		if(*t3f_resource[i]->ptr)
		{
			t3f_actually_unload_resource(i);
		}
	}
}

void t3f_reload_resources(void)
{
	int i;
	const ALLEGRO_FS_INTERFACE * old_fs;

	old_fs = al_get_fs_interface();
	for(i = 0; i < t3f_resources; i++)
	{
		if(t3f_resource[i]->proc)
		{
			al_set_new_file_interface(t3f_resource[i]->fi);
			t3f_resource[i]->proc(t3f_resource[i]->ptr, NULL, t3f_resource[i]->filename, t3f_resource[i]->option, t3f_resource[i]->flags, t3f_resource[i]->offset, false);
		}
	}
	al_set_fs_interface(old_fs);
}

void * t3f_clone_resource(void ** dest, void * ptr)
{
	int i;
	const ALLEGRO_FILE_INTERFACE * old_fi;

	for(i = 0; i < t3f_resources; i++)
	{
		if(*t3f_resource[i]->ptr == ptr)
		{
			old_fi = al_get_new_file_interface();
			al_set_new_file_interface(t3f_resource[i]->fi);
			t3f_resource[i]->proc(dest, NULL, t3f_resource[i]->filename, t3f_resource[i]->option, t3f_resource[i]->flags, t3f_resource[i]->offset, false);
			if(*dest)
			{
				t3f_add_resource(t3f_resource[i]->proc, dest, t3f_resource[i]->filename, t3f_resource[i]->option, t3f_resource[i]->flags, t3f_resource[i]->offset, al_get_new_file_interface());
			}
			al_set_new_file_interface(old_fi);
			break;
		}
	}
	return *dest;
}

void t3f_show_resources(void)
{
	int i;

	t3f_debug_message("Total resources: %d\n", t3f_resources);
	for(i = 0; i < t3f_resources; i++)
	{
		t3f_debug_message("Resource %d: %s\n", i, t3f_resource[i]->filename);
	}
}
