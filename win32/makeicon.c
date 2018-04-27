#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

int file_size(const char * fn)
{
	ALLEGRO_FS_ENTRY * entry;
	int size = 0;

	entry = al_create_fs_entry(fn);
	if(entry)
	{
		size = al_get_fs_entry_size(entry);
		al_destroy_fs_entry(entry);
	}
	return size;
}

bool embed_file(ALLEGRO_FILE * fp, const char * fn)
{
	ALLEGRO_FILE * efp;
	int i;

	efp = al_fopen(fn, "rb");
	if(efp)
	{
		for(i = 0; i < al_fsize(efp); i++)
		{
			al_fputc(fp, al_fgetc(efp));
		}
		al_fclose(efp);
		return true;
	}
	return false;
}

bool create_windows_icon(ALLEGRO_BITMAP ** bp, ALLEGRO_PATH * path)
{
	ALLEGRO_FILE * fp;
	int i;
	int count = 0;
	int w, h;
	char buf[32];
	int size[256] = {0};
	int current_size = 0;

	for(i = 0; bp[i]; i++)
	{
		count++;
	}
	for(i = 0; i < count; i++)
	{
		sprintf(buf, "%d.png", i);
		al_save_bitmap(buf, bp[i]);
		size[i] = file_size(buf);
	}
	fp = al_fopen(al_path_cstr(path, '/'), "wb");
	if(fp)
	{
		/* icon header */
		al_fwrite16le(fp, 0);
		al_fwrite16le(fp, 1);
		al_fwrite16le(fp, count);

		/* image directory */
		for(i = 0; i < count; i++)
		{
			w = al_get_bitmap_width(bp[i]);
			h = al_get_bitmap_height(bp[i]);
			al_save_bitmap("temp.png", bp[i]);
			al_fputc(fp, w < 256 ? w : 0);
			al_fputc(fp, h < 256 ? h : 0);
			al_fputc(fp, 0);
			al_fputc(fp, 0);
			al_fwrite16le(fp, 0);
			al_fwrite16le(fp, 32);
			al_fwrite32le(fp, size[i]);
			al_fwrite32le(fp, 6 + 16 * count + current_size);
			current_size += size[i];
		}
		for(i = 0; i < count; i++)
		{
			sprintf(buf, "%d.png", i);
			embed_file(fp, buf);
			al_remove_filename(buf);
		}
		al_fclose(fp);
		return true;
	}
	return false;
}

bool process_arguments(int argc, char * argv[], char * used)
{
	ALLEGRO_PATH * output_path = NULL;
	ALLEGRO_BITMAP * bitmap[256] = {NULL};
	int pos = 0;
	int i;

	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "-o"))
		{
			if(i < argc - 1)
			{
				output_path = al_create_path(argv[i + 1]);
				if(!output_path)
				{
					printf("Could not allocate memory for output path!\n");
					return false;
				}
				used[i] = 1;
				used[i + 1] = 1;
			}
		}
	}
	if(!output_path)
	{
		printf("Usage: makeicon ... -o <output filename>\n\n");
		return false;
	}
	for(i = 1; i < argc; i++)
	{
		if(!used[i])
		{
			bitmap[pos] = al_load_bitmap(argv[i]);
			if(!bitmap[pos])
			{
				printf("Failed to load bitmap %s!\n", argv[i]);
				goto fail;
			}
			if(al_get_bitmap_width(bitmap[pos]) > 256 || al_get_bitmap_height(bitmap[pos]) > 256)
			{
				printf("Bitmap %s too large!\n", argv[i]);
				goto fail;
			}
			pos++;
		}
	}
	if(!strcmp(al_get_path_extension(output_path), ".ico"))
	{
		if(!create_windows_icon(bitmap, output_path))
		{
			printf("Failed to create %s!\n", al_path_cstr(output_path, '/'));
			goto fail;
		}
	}
	return true;

	fail:
	{
		for(i = 0; i < pos; i++)
		{
			if(bitmap[i])
			{
				al_destroy_bitmap(bitmap[i]);
			}
		}
		return false;
	}
}

bool init(void)
{
	if(!al_init())
	{
		printf("Could not initialize Allegro!\n");
		return false;
	}
	if(!al_init_image_addon())
	{
		printf("Failed to initialize image add-on!\n");
		return false;
	}
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	return true;
}

int main(int argc, char * argv[])
{
	char * used;
	int ret = 0;

	if(!init())
	{
		printf("Failed to initialize program!\n");
		return -1;
	}

	used = malloc(argc);
	if(used)
	{
		memset(used, 0, argc);
		if(!process_arguments(argc, argv, used))
		{
			printf("Failed to complete task!\n");
			ret = -1;
		}
		free(used);
	}
	return ret;
}
