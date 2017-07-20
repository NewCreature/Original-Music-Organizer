#include "gme/gme.h"
#include "t3f/t3f.h"

#include "../archive_handler.h"

static OMO_ARCHIVE_HANDLER archive_handler;
static char archive_handler_buffer[1024] = {0};

static int count_files(const char * fn)
{
	static Music_Emu * emu = NULL;
	int count = 0;

	gme_open_file(fn, &emu, 44100);
	if(emu)
	{
		count = gme_track_count(emu);
		gme_delete(emu);
	}
	return count;
}

static const char * get_file(const char * fn, int index)
{
	sprintf(archive_handler_buffer, "%d", index);
	return archive_handler_buffer;
}

OMO_ARCHIVE_HANDLER * omo_get_gme_archive_handler(void)
{
	memset(&archive_handler, 0, sizeof(OMO_ARCHIVE_HANDLER));
	archive_handler.count_files = count_files;
	archive_handler.get_file = get_file;
	omo_archive_handler_add_type(&archive_handler, ".nsf");
	return &archive_handler;
}
