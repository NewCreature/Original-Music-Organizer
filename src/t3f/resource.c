#include "t3f.h"
#include "resource.h"

static T3F_RESOURCE ** _t3f_resource = NULL;
static int _t3f_max_resources = 0;
static int _t3f_resources = 0;
ALLEGRO_MUTEX * _t3f_resource_mutex = NULL;

bool _t3f_initialize_resource_manager(int max_resources)
{
	if(!_t3f_resource)
	{
		_t3f_max_resources = max_resources;
		
		_t3f_resource = malloc(sizeof(T3F_RESOURCE *) * _t3f_max_resources);
		if(!_t3f_resource)
		{
			goto fail;
		}

		_t3f_resource_mutex = al_create_mutex();
		if(!_t3f_resource_mutex)
		{
			goto fail;
		}
	}
	return true;

	fail:
	{
		_t3f_uninitialize_resource_manager();
		return false;
	}
}

void _t3f_uninitialize_resource_manager(void)
{
	if(_t3f_resource_mutex)
	{
		al_destroy_mutex(_t3f_resource_mutex);
	}
	if(_t3f_resource)
	{
		free(_t3f_resource);
	}
}

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
		bitmap = _t3f_load_allegro_bitmap_padded(filename, flags);
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
				bitmap = t3f_load_allegro_bitmap_f(fp, flags);
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

bool t3f_create_bitmap_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy)
{
	ALLEGRO_BITMAP * bitmap = (ALLEGRO_BITMAP *)*ptr;
	int width, height;
	ALLEGRO_STATE old_state;

	if(destroy)
	{
		al_destroy_bitmap(bitmap);
		return true;
	}

	/* extract dimensions from 'option' */
	width = option & 0xFFFF;
	height = (option >> 16) & 0xFFFF;

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(al_get_new_bitmap_flags() | ALLEGRO_NO_PRESERVE_TEXTURE);
	if(flags & T3F_BITMAP_FLAG_PADDED)
	{
		bitmap = al_create_bitmap(width + 2, height + 2);
	}
	else
	{
		bitmap = al_create_bitmap(width, height);
	}
	if(!bitmap)
	{
		goto fail;
	}
	*ptr = bitmap;
	al_restore_state(&old_state);

	return *ptr;

	fail:
	{
		al_restore_state(&old_state);
		return false;
	}
}

bool t3f_font_t3f_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy)
{
	T3F_FONT * font = (T3F_FONT *)*ptr;
	ALLEGRO_STATE old_state;
	bool openfp = false;

	if(destroy)
	{
		t3f_destroy_font_data(font, T3F_FONT_TYPE_T3F);
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
		font = t3f_load_font_data(filename, T3F_FONT_TYPE_T3F, option, flags);
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
			font = t3f_load_font_data_f(filename, fp, T3F_FONT_TYPE_T3F, option, flags);
			if(!openfp)
			{
				al_fclose(fp);
			}
		}
	}
	*ptr = font;
	al_restore_state(&old_state);
	return *ptr;
}

bool t3f_font_allegro_resource_handler_proc(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy)
{
	T3F_FONT * font = (T3F_FONT *)*ptr;
	ALLEGRO_STATE old_state;
	bool openfp = false;

	if(destroy)
	{
		t3f_destroy_font_data(font, T3F_FONT_TYPE_ALLEGRO);
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
		font = t3f_load_font_data(filename, T3F_FONT_TYPE_ALLEGRO, option, flags);
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
			font = t3f_load_font_data_f(filename, fp, T3F_FONT_TYPE_ALLEGRO, option, flags);
			if(!openfp)
			{
				al_fclose(fp);
			}
		}
	}
	*ptr = font;
	al_restore_state(&old_state);
	return *ptr;
}

static bool t3f_add_resource(bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), void ** ptr, const char * filename, int option, int flags, unsigned long offset, const ALLEGRO_FILE_INTERFACE * fi)
{
	if(_t3f_resources < _t3f_max_resources)
	{
		_t3f_resource[_t3f_resources] = malloc(sizeof(T3F_RESOURCE));
		if(!_t3f_resource[_t3f_resources])
		{
			goto fail;
		}
		_t3f_resource[_t3f_resources]->proc = proc;
		_t3f_resource[_t3f_resources]->ptr = ptr;
		_t3f_resource[_t3f_resources]->filename = strdup(filename);
		if(!_t3f_resource[_t3f_resources]->filename)
		{
			goto fail;
		}
		_t3f_resource[_t3f_resources]->offset = offset;
		_t3f_resource[_t3f_resources]->option = option;
		_t3f_resource[_t3f_resources]->flags = flags;
		_t3f_resource[_t3f_resources]->fi = fi;
		_t3f_resources++;
		return true;
	}
	return false;

	fail:
	{
		if(_t3f_resource[_t3f_resources])
		{
			if(_t3f_resource[_t3f_resources]->filename)
			{
				free(_t3f_resource[_t3f_resources]->filename);
			}
			free(_t3f_resource[_t3f_resources]);
		}
		return false;
	}
}

static void _t3f_remove_resource(int i)
{
	int j;

	free(_t3f_resource[i]->filename);
	free(_t3f_resource[i]);
	for(j = i; j < _t3f_resources - 1; j++)
	{
		_t3f_resource[j] = _t3f_resource[j + 1];
	}
	_t3f_resources--;
}

void * t3f_load_resource(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), const char * filename, int option, int flags, unsigned long offset)
{
	void * temp_ptr;
	void ** temp_ptr_ptr = ptr;

	if(!temp_ptr_ptr)
	{
		temp_ptr_ptr = &temp_ptr;
	}
	al_lock_mutex(_t3f_resource_mutex);
	if(proc)
	{
		proc(temp_ptr_ptr, NULL, filename, option, flags, offset, false);

		/* register resource if user supplied 'ptr' */
		if(*temp_ptr_ptr && ptr == temp_ptr_ptr)
		{
			t3f_add_resource(proc, temp_ptr_ptr, filename, option, flags, offset, al_get_new_file_interface());
		}
	}
	al_unlock_mutex(_t3f_resource_mutex);
	return *temp_ptr_ptr;
}

void * t3f_load_resource_f(void ** ptr, bool (*proc)(void ** ptr, ALLEGRO_FILE * fp, const char * filename, int option, int flags, unsigned long offset, bool destroy), ALLEGRO_FILE * fp, const char * filename, int option, int flags)
{
	unsigned long offset;
	al_lock_mutex(_t3f_resource_mutex);
	if(proc)
	{
		offset = al_ftell(fp);
		proc(ptr, fp, filename, option, flags, offset, false);
		if(*ptr)
		{
			t3f_add_resource(proc, ptr, filename, option, flags, offset, al_get_new_file_interface());
		}
	}
	al_unlock_mutex(_t3f_resource_mutex);
	return *ptr;
}

static void _t3f_actually_unload_resource(int i)
{
	if(_t3f_resource[i]->proc)
	{
		_t3f_resource[i]->proc(_t3f_resource[i]->ptr, NULL, NULL, 0, 0, 0, true);
		*_t3f_resource[i]->ptr = NULL;
	}
}

static int _t3f_unload_resource(void * ptr)
{
	int i;

	for(i = 0; i < _t3f_resources; i++)
	{
		if(*_t3f_resource[i]->ptr == ptr)
		{
			_t3f_actually_unload_resource(i);
			return i;
		}
	}
	return -1;
}

bool t3f_destroy_resource(void * ptr)
{
	int i;

	al_lock_mutex(_t3f_resource_mutex);
	i = _t3f_unload_resource(ptr);
	if(i >= 0)
	{
		al_unlock_mutex(_t3f_resource_mutex);
		_t3f_remove_resource(i);
		return true;
	}
	al_unlock_mutex(_t3f_resource_mutex);
	return false;
}

void t3f_remap_resource(void ** original_ptr, void ** new_ptr)
{
	int i;

	for(i = 0; i < _t3f_resources; i++)
	{
		if(_t3f_resource[i]->ptr == original_ptr)
		{
			_t3f_resource[i]->ptr = new_ptr;
		}
	}
}

void t3f_unload_resources(void)
{
	int i;

	al_lock_mutex(_t3f_resource_mutex);
	for(i = 0; i < _t3f_resources; i++)
	{
		if(*_t3f_resource[i]->ptr)
		{
			_t3f_actually_unload_resource(i);
		}
	}
	al_unlock_mutex(_t3f_resource_mutex);
}

void t3f_reload_resources(void)
{
	int i;
	const ALLEGRO_FS_INTERFACE * old_fs;

	al_lock_mutex(_t3f_resource_mutex);
	old_fs = al_get_fs_interface();
	for(i = 0; i < _t3f_resources; i++)
	{
		if(_t3f_resource[i]->proc)
		{
			al_set_new_file_interface(_t3f_resource[i]->fi);
			_t3f_resource[i]->proc(_t3f_resource[i]->ptr, NULL, _t3f_resource[i]->filename, _t3f_resource[i]->option, _t3f_resource[i]->flags, _t3f_resource[i]->offset, false);
		}
	}
	al_set_fs_interface(old_fs);
	al_unlock_mutex(_t3f_resource_mutex);
}

void * t3f_clone_resource(void ** dest, void ** original_ptr)
{
	int i;
	const ALLEGRO_FILE_INTERFACE * old_fi;

	al_lock_mutex(_t3f_resource_mutex);
	for(i = 0; i < _t3f_resources; i++)
	{
		if(_t3f_resource[i]->ptr == original_ptr)
		{
			old_fi = al_get_new_file_interface();
			al_set_new_file_interface(_t3f_resource[i]->fi);
			_t3f_resource[i]->proc(dest, NULL, _t3f_resource[i]->filename, _t3f_resource[i]->option, _t3f_resource[i]->flags, _t3f_resource[i]->offset, false);
			if(*dest)
			{
				t3f_add_resource(_t3f_resource[i]->proc, dest, _t3f_resource[i]->filename, _t3f_resource[i]->option, _t3f_resource[i]->flags, _t3f_resource[i]->offset, al_get_new_file_interface());
			}
			al_set_new_file_interface(old_fi);
			break;
		}
	}
	al_unlock_mutex(_t3f_resource_mutex);
	return *dest;
}

void t3f_show_resources(void)
{
	int i;

	al_lock_mutex(_t3f_resource_mutex);
	t3f_debug_message("Total resources: %d\n", _t3f_resources);
	for(i = 0; i < _t3f_resources; i++)
	{
		t3f_debug_message("Resource %d: %s (%lu)\n", i, _t3f_resource[i]->filename, *_t3f_resource[i]->ptr);
	}
	al_unlock_mutex(_t3f_resource_mutex);
}
