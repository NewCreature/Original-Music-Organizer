#include "t3f/t3f.h"
#include "io.h"

static RTK_IO_DRIVER rtk_io_allegro_driver;

void * rtk_io_fopen_allegro(const char * fn, const char * mode)
{
	return al_fopen(fn, mode);
}

int rtk_io_fclose_allegro(void * fp)
{
	return al_fclose((ALLEGRO_FILE *)fp);
}

int rtk_io_fread_allegro(void * fp, void * buffer, int n)
{
	return al_fread((ALLEGRO_FILE *)fp, buffer, n);
}

int rtk_io_fwrite_allegro(void * fp, void * buffer, int n)
{
	return al_fwrite((ALLEGRO_FILE *)fp, buffer, n);
}

void rtk_io_set_allegro_driver(void)
{
	rtk_io_allegro_driver.open = rtk_io_fopen_allegro;
	rtk_io_allegro_driver.close = rtk_io_fclose_allegro;
	rtk_io_allegro_driver.read = rtk_io_fread_allegro;
	rtk_io_allegro_driver.write = rtk_io_fwrite_allegro;
	rtk_set_io_driver(&rtk_io_allegro_driver);
}
