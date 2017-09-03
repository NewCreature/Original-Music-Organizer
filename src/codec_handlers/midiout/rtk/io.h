#ifndef LIBRTK_IO_H
#define LIBRTK_IO_H

#include <stdio.h>

typedef struct
{

	void * (*open)(const char * fn, const char * mode);
	int (*close)(void * fp);
	int (*read)(void * fp, void * buffer, int n);
	int (*write)(void * fp, void * buffer, int n);

} RTK_IO_DRIVER;

/* basic I/O functionality */
void * rtk_io_fopen(const char * fn, const char * mode);
int rtk_io_fclose(void * fp);
int rtk_io_fread(void * fp, void * buffer, int n);
int rtk_io_fwrite(void * fp, void * buffer, int n);

/* special I/O functionality for easier loading of MIDI file data */
int rtk_io_fgetc(void * fp);
short rtk_io_mgetw(void * fp);
long rtk_io_mgetl(void * fp);
int rtk_io_fputc(int c, void * fp);
int rtk_io_mputw(int w, void * fp);
int rtk_io_mputl(long l, void * fp);

/* driver functions */
int rtk_set_io_driver(RTK_IO_DRIVER * dp);
RTK_IO_DRIVER * rtk_get_io_driver(void);

#endif
