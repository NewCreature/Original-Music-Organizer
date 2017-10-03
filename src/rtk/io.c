#include <stdio.h>
#include "io.h"

static RTK_IO_DRIVER * rtk_io_current_driver = NULL;

int rtk_set_io_driver(RTK_IO_DRIVER * dp)
{
	rtk_io_current_driver = dp;
	return 1;
}

RTK_IO_DRIVER * rtk_get_io_driver(void)
{
	return rtk_io_current_driver;
}

void * rtk_io_fopen(const char * fn, const char * mode)
{
	return rtk_io_current_driver->open(fn, mode);
}

int rtk_io_fclose(void * fp)
{
	return rtk_io_current_driver->close(fp);
}

int rtk_io_fread(void * fp, void * buffer, int n)
{
	return rtk_io_current_driver->read(fp, buffer, n);
}

int rtk_io_fwrite(void * fp, void * buffer, int n)
{
	return rtk_io_current_driver->write(fp, buffer, n);
}

int rtk_io_fgetc(void * fp)
{
	unsigned char buf[1] = {0};
	int r;
	
	r = rtk_io_current_driver->read(fp, buf, 1);
	if(r >= 1)
	{
		return buf[0];
	}
	return EOF;
}

short rtk_io_mgetw(void * fp)
{
	int b1, b2;

	if((b1 = rtk_io_fgetc(fp)) != EOF)
	{
		if((b2 = rtk_io_fgetc(fp)) != EOF)
		{
			return ((b1 << 8) | b2);
		}
	}

	return EOF;
}

long rtk_io_mgetl(void * fp)
{
	int b1, b2, b3, b4;

	if((b1 = rtk_io_fgetc(fp)) != EOF)
	{
		if((b2 = rtk_io_fgetc(fp)) != EOF)
		{
			if((b3 = rtk_io_fgetc(fp)) != EOF)
			{
				if((b4 = rtk_io_fgetc(fp)) != EOF)
				{
					return (((long)b1 << 24) | ((long)b2 << 16) | ((long)b3 << 8) | (long)b4);
				}
			}
		}
	}

	return EOF;
}

int rtk_io_fputc(int c, void * fp)
{
	char buf[1];
	int r;
	
	buf[0] = c;
	r = rtk_io_current_driver->write(fp, buf, 1);
	if(r == 1)
	{
		return 1;
	}
	return EOF;
}

int rtk_io_mputw(int w, void * fp)
{
	int b1, b2;

	b1 = (w & 0xFF00) >> 8;
	b2 = w & 0x00FF;

	if(rtk_io_fputc(b1, fp) == b1)
	{
		if (rtk_io_fputc(b2, fp) == b2)
		{
			return 1;
		}
	}

	return EOF;
}

int rtk_io_mputl(long l, void * fp)
{
	int b1, b2, b3, b4;

	b1 = (int)((l & 0xFF000000L) >> 24);
	b2 = (int)((l & 0x00FF0000L) >> 16);
	b3 = (int)((l & 0x0000FF00L) >> 8);
	b4 = (int)l & 0x00FF;

	if(rtk_io_fputc(b1, fp) == b1)
	{
		if(rtk_io_fputc(b2, fp) == b2)
		{
			if(rtk_io_fputc(b3, fp) == b3)
			{
				if(rtk_io_fputc(b4, fp) == b4)
				{
					return 1;
				}
			}
		}
	}

	return EOF;
}
