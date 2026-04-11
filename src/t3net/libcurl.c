#include "t3net.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#include "t3f/t3f.h"
#include "t3f/debug.h"

#define _T3NET_LIBCURL_SPLIT_POST_FAIL   0
#define _T3NET_LIBCURL_SPLIT_POST_NORMAL 1
#define _T3NET_LIBCURL_SPLIT_POST_FILE   2

static char * _t3net_libcurl_temp_dir = NULL;
static char * _t3net_libcurl_ca_bundle_path = NULL;

static int _get_val_offset(const char * str)
{
  int i;

  for(i = 0; i < strlen(str); i++)
  {
    if(str[i] == '=')
    {
      return i + 1;
    }
  }
  return -1;
}

static int _t3net_split_post_data(const char * post_item, char * out_key, char * out_val, int out_max)
{
  int val_offset = _get_val_offset(post_item);
  int ret = _T3NET_LIBCURL_SPLIT_POST_NORMAL;

  if(val_offset < 0 || val_offset >= out_max - 1)
  {
    goto fail;
  }
  if(post_item[val_offset] == '@')
  {
    val_offset++;
    ret = _T3NET_LIBCURL_SPLIT_POST_FILE;
  }
  memset(out_key, 0, out_max);
  memcpy(out_key, post_item, val_offset - 2);
  strcpy(out_val, &post_item[val_offset]);
  if(strlen(out_key) <= 0 || strlen(out_val) <= 0)
  {
    goto fail;
  }

  return ret;

  fail:
  {
    return _T3NET_LIBCURL_SPLIT_POST_FAIL;
  }
}

static size_t _t3net_libcurl_write_proc(void * ptr, size_t size, size_t n, void * out)
{
    return fwrite(ptr, size, n, (FILE *)out);
}

static int _t3net_libcurl_url_runner(const char * url, const char ** post_data, const char * out_path, char ** out_data)
{
  CURL * curl = NULL;
  CURLcode ret;
  curl_mime * post_form = NULL;
  curl_mimepart * post_field = NULL;
  FILE * fp = NULL;
  char out_key[1024];
  char out_val[1024];
  char temp_path[1024];
  int split_ret;
  int i;

  curl = curl_easy_init();
  if(!curl)
  {
    goto fail;
  }

  /* contert POST data to correct format for libcurl */
  if(post_data)
  {
    post_form = curl_mime_init(curl);
    if(!post_form)
    {
      goto fail;
    }
    for(i = 0; post_data[i]; i++)
    {
      post_field = curl_mime_addpart(post_form);
      split_ret = _t3net_split_post_data(post_data[i], out_key, out_val, 1024);
      switch(split_ret)
      {
        case _T3NET_LIBCURL_SPLIT_POST_NORMAL:
        {
          curl_mime_name(post_field, out_key);
          curl_mime_data(post_field, out_val, CURL_ZERO_TERMINATED);
          break;
        }
        case _T3NET_LIBCURL_SPLIT_POST_FILE:
        {
          curl_mime_name(post_field, out_key);
          curl_mime_filedata(post_field, out_val);
          break;
        }
        default:
        {
          goto fail;
        }
      }
    }
  }

  /* open output file for writing */
  if(out_path)
  {
    sprintf(temp_path, "%s", out_path);
  }
  else
  {
    sprintf(temp_path, "%st3net.out", _t3net_libcurl_temp_dir);
  }
  fp = fopen(temp_path, "wb");
  if(!fp)
  {
    goto fail;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  if(post_data)
  {
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, post_form);
  }
  if(_t3net_libcurl_ca_bundle_path)
  {
    curl_easy_setopt(curl, CURLOPT_CAINFO, _t3net_libcurl_ca_bundle_path);
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _t3net_libcurl_write_proc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  ret = curl_easy_perform(curl);
  if(ret != CURLE_OK)
  {
    goto fail;
  }
  fclose(fp);

  /* copy output to 'out_data' */
  if(out_data)
  {
    *out_data = _t3net_load_file(temp_path);
    if(!*out_data)
    {
      goto fail;
    }
  }

  curl_easy_cleanup(curl);
  if(post_form)
  {
    curl_mime_free(post_form);
  }
  return 1;

  fail:
  {
    if(post_form)
    {
      curl_mime_free(post_form);
    }
    if(curl)
    {
      curl_easy_cleanup(curl);
    }
    if(fp)
    {
      fclose(fp);
    }
    return 0;
  }

}

static void _t3net_libcurl_exit_proc(void)
{
  if(_t3net_libcurl_temp_dir)
  {
    free(_t3net_libcurl_temp_dir);
    _t3net_libcurl_temp_dir = NULL;
  }
  if(_t3net_libcurl_ca_bundle_path)
  {
    free(_t3net_libcurl_ca_bundle_path);
    _t3net_libcurl_ca_bundle_path = NULL;
  }
  curl_global_cleanup();
}

int t3net_setup_with_libcurl(const char * temp_dir, const char * ca_bundle_path)
{
  if(curl_global_init(CURL_GLOBAL_ALL))
  {
    goto fail;
  }
  if(!temp_dir)
  {
    goto fail;
  }
  _t3net_libcurl_temp_dir = strdup(temp_dir);
  if(!_t3net_libcurl_temp_dir)
  {
    goto fail;
  }
  if(ca_bundle_path)
  {
    _t3net_libcurl_ca_bundle_path = strdup(ca_bundle_path);
    if(!_t3net_libcurl_ca_bundle_path)
    {
      goto fail;
    }
  }
  return _t3net_setup(_t3net_libcurl_url_runner, _t3net_libcurl_exit_proc);

  fail:
  {
    return 0;
  }
}
