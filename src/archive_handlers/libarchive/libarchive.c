#include <archive.h>
#include <archive_entry.h>
#include "t3f/t3f.h"

#include "../archive_handler.h"

static OMO_ARCHIVE_HANDLER archive_handler;

static int count_files(const char * fn)
{
	struct archive *a;
	struct archive_entry *entry;
	int r, r2;
	int total = 0;

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, fn, 10240); // Note 1
	if(r == ARCHIVE_OK)
	{
		while(1)
		{
			r2 = archive_read_next_header(a, &entry);
			if(r2 == ARCHIVE_OK)
			{
				total++;
				archive_entry_pathname(entry);
//				archive_read_data_skip(a);  // Note 2
			}
			else
			{
				break;
			}
		}
		archive_read_free(a);  // Note 3
	}

	return total;
}

static char returnfn[1024];

static const char * get_file(const char * fn, int index)
{
	struct archive *a;
	struct archive_entry *entry;
	int r, r2;
	int total = 0;

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, fn, 10240); // Note 1
	if(r == ARCHIVE_OK)
	{
		while(1)
		{
			r2 = archive_read_next_header(a, &entry);
			if(r2 == ARCHIVE_OK)
			{
				if(total == index)
				{
					strcpy(returnfn, archive_entry_pathname(entry));
					break;
				}
				total++;
//				archive_read_data_skip(a);  // Note 2
			}
			else
			{
				break;
			}
		}
		archive_read_free(a);  // Note 3
	}

	return returnfn;
}

static int copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	la_int64_t offset;

	for(;;)
	{
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if(r == ARCHIVE_EOF)
		{
			return (ARCHIVE_OK);
		}
		if(r < ARCHIVE_OK)
		{
			return (r);
		}
		r = archive_write_data_block(aw, buff, size, offset);
		if(r < ARCHIVE_OK)
		{
			fprintf(stderr, "%s\n", archive_error_string(aw));
			return (r);
		}
	}
}

static const char * extract_current_file(struct archive * a, struct archive_entry * entry)
{
	struct archive *ext;
	int flags;
	int r;

	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	flags |= ARCHIVE_EXTRACT_ACL;
	flags |= ARCHIVE_EXTRACT_FFLAGS;

	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);
//	archive_entry_set_pathname(entry, "test");
	r = archive_write_header(ext, entry);
    if(r == ARCHIVE_OK)
	{
    	if(archive_entry_size(entry) > 0)
		{
			r = copy_data(a, ext);
		}
		r = archive_write_finish_entry(ext);
	}
	return NULL;
}

static const char * extract_file(const char * fn, int index)
{
	struct archive *a;
	struct archive_entry *entry;
	int r, r2;
	int total = 0;
	char * cwd = al_get_current_directory();
	strcpy(returnfn, "");
	al_change_directory(al_path_cstr(t3f_data_path, '/'));

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, fn, 10240); // Note 1
	if(r == ARCHIVE_OK)
	{
		while(1)
		{
			r2 = archive_read_next_header(a, &entry);
			if(r2 == ARCHIVE_OK)
			{
				if(total == index)
				{
					extract_current_file(a, entry);
					strcpy(returnfn, t3f_get_filename(t3f_data_path, archive_entry_pathname(entry)));
					break;
				}
				total++;
//				archive_read_data_skip(a);  // Note 2
			}
			else
			{
				break;
			}
		}
		archive_read_free(a);  // Note 3
	}
	al_change_directory(cwd);
	free(cwd);

	return returnfn;
}

OMO_ARCHIVE_HANDLER * omo_get_libarchive_archive_handler(void)
{
	memset(&archive_handler, 0, sizeof(OMO_ARCHIVE_HANDLER));
	archive_handler.count_files = count_files;
	archive_handler.get_file = get_file;
	archive_handler.extract_file = extract_file;
	omo_archive_handler_add_type(&archive_handler, ".zip");
	omo_archive_handler_add_type(&archive_handler, ".zsn");
//	omo_archive_handler_add_type(&archive_handler, ".rar");
//	omo_archive_handler_add_type(&archive_handler, ".rsn");
	return &archive_handler;
}
