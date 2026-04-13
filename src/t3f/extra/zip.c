#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "minizip/zip.h"

static char * _t3f_zip_root = NULL;

bool _t3f_zip_add(const char * fn, bool isfolder, void * data)
{
  zipFile zip = (zipFile)data;
  zip_fileinfo zip_info = {0};
  ALLEGRO_PATH * path;
  const char * offset_path;
  ALLEGRO_FILE * fp = NULL;
  char buf[1024];
  int bytes_read;
  bool writing_file = false;

  /* set root */
  if(!_t3f_zip_root)
  {
    path = al_create_path(fn);
    if(!path)
    {
      goto fail;
    }
    al_set_path_filename(path, NULL);
    _t3f_zip_root = strdup(al_path_cstr(path, '/'));
    al_destroy_path(path);
  }

  /* add file */
  if(!isfolder)
  {
    path = al_create_path(fn);
    if(!path)
    {
      goto fail;
    }
    al_set_path_filename(path, NULL);
    offset_path = fn;
    offset_path += strlen(_t3f_zip_root);
    if(zipOpenNewFileInZip(zip, offset_path, &zip_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK)
    {
      writing_file = true;
      fp = al_fopen(fn, "rb");
      if(!fp)
      {
        goto fail;
      }
      while(!al_feof(fp))
      {
        bytes_read = al_fread(fp, buf, 1024);
        if(bytes_read < 1024 && !al_feof(fp))
        {
          goto fail;
        }
        if(bytes_read > 0)
        {
          if(zipWriteInFileInZip(zip, buf, bytes_read))
          {
            goto fail;
          }
        }
      }
      al_fclose(fp);
      zipCloseFileInZip(zip);      
    }
    else
    {
      goto fail;
    }
    al_destroy_path(path);
  }
  return true;

  fail:
  {
    if(path)
    {
      al_destroy_path(path);
    }
    if(fp)
    {
      al_fclose(fp);
    }
    if(writing_file)
    {
      zipCloseFileInZip(zip);
    }
    return false;
  }
}

bool t3f_create_zip(const char * zip_path, const char * dir, int flags)
{
  zipFile zip;
  ALLEGRO_PATH * path = NULL;

  zip = zipOpen(zip_path, 0);
  if(!zip)
  {
    goto fail;
  }
  path = al_create_path(dir);
  if(!path)
  {
    goto fail;
  }
  if(!t3f_scan_files(dir, _t3f_zip_add, true, zip))
  {
    goto fail;
  }
  if(_t3f_zip_root)
  {
    free(_t3f_zip_root);
    _t3f_zip_root = NULL;
  }
  zipClose(zip, NULL);

  return true;

  fail:
  {
    if(path)
    {
      al_destroy_path(path);
    }
    if(zip)
    {
      zipClose(zip, NULL);
    }
    if(_t3f_zip_root)
    {
      free(_t3f_zip_root);
    }
    return false;
  }
}
