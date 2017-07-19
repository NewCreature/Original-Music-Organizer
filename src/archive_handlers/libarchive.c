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

	printf("count!\n");
	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, fn, 10240); // Note 1
	if(r == ARCHIVE_OK)
	{
		while(1)
		{
			r2 = archive_read_next_header(a, &entry);
			printf("r: %d (%d, %d)\n", r2, ARCHIVE_WARN, ARCHIVE_FATAL);
			if(r2 == ARCHIVE_OK)
			{
				total++;
				printf("%s\n",archive_entry_pathname(entry));
//				archive_read_data_skip(a);  // Note 2
			}
			else
			{
				break;
			}
		}
		archive_read_free(a);  // Note 3
	}
	printf("total: %d\n", total);

	return total;
}

static const char * get_file(const char * fn, int index)
{
	return NULL;
}

OMO_ARCHIVE_HANDLER * omo_get_libarchive_archive_handler(void)
{
	memset(&archive_handler, 0, sizeof(OMO_ARCHIVE_HANDLER));
	archive_handler.count_files = count_files;
	archive_handler.get_file = get_file;
	omo_archive_handler_add_type(&archive_handler, ".zip");
	omo_archive_handler_add_type(&archive_handler, ".zsn");
	omo_archive_handler_add_type(&archive_handler, ".rar");
	omo_archive_handler_add_type(&archive_handler, ".rsn");
	return &archive_handler;
}
