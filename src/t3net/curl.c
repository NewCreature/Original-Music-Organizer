#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#include <windows.h>
#endif
#include "t3net.h"

static char * _t3net_curl_temp_dir = NULL;

static int _t3net_get_post_data_length(const char ** post_data)
{
	int i;
	int l = 0;

	if(post_data)
	{
		for(i = 0; post_data[i]; i++)
		{
			l += strlen(post_data[i]);
		}
	}
	return l;
}

static int _t3net_get_curl_command(char * out, int out_size)
{
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		strcpy(out, "./curl.exe");
	#elif __APPLE__
		strcpy(out, "/usr/bin/curl");
	#else
		strcpy(out, "LD_LIBRARY_PATH=\"/lib\" curl");
	#endif
	return 1;
}

static int _t3net_append_to_command(char * out, int out_size, const char * in)
{
  if(strlen(in) + strlen(out) >= out_size - 1)
  {
    return 0;
  }
  strcat(out, in);
  return 1;
}

static int _t3net_curl_url_runner(const char * url, const char ** post_data, const char * out_path, char ** out_data)
{
	char * curl_command = malloc(strlen(url) + _t3net_get_post_data_length(post_data) + 1024);
	char temp_path[1024] = {0};
	char buf[256];
	int ret = 0;
	int i;

	if(curl_command)
	{
		if(out_path)
		{
			sprintf(temp_path, "%s", out_path);
		}
		else
		{
			sprintf(temp_path, "%st3net.out", _t3net_curl_temp_dir);
		}
    if(!_t3net_get_curl_command(curl_command, 1024))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " -L"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --connect-timeout"))
    {
      goto fail;
    }
		sprintf(buf, " %d", T3NET_TIMEOUT_TIME);
		if(!_t3net_append_to_command(curl_command, 1024, buf))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --silent"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --output"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " \""))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, temp_path))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, "\""))
    {
      goto fail;
    }
		if(post_data)
		{
			for(i = 0; post_data[i]; i++)
			{
				if(!_t3net_append_to_command(curl_command, 1024, " -F \""))
        {
          goto fail;
        }
				if(!_t3net_append_to_command(curl_command, 1024, post_data[i]))
        {
          goto fail;
        }
				if(!_t3net_append_to_command(curl_command, 1024, "\""))
        {
          goto fail;
        }
			}
		}
		if(!_t3net_append_to_command(curl_command, 1024, " \""))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, url))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, "\""))
    {
      goto fail;
    }

		ret = !_t3net_run_system_command(curl_command, NULL);
		free(curl_command);
		if(out_data)
		{
			*out_data = _t3net_load_file(temp_path);
			if(!*out_data)
			{
				ret = 0;
			}
		}
	}
	return ret;

  fail:
  {
    return 0;
  }
}

static void _t3net_curl_exit_proc(void)
{
  if(_t3net_curl_temp_dir)
  {
    free(_t3net_curl_temp_dir);
  }
}

int t3net_setup_with_curl(const char * temp_dir)
{
  if(temp_dir)
  {
    _t3net_curl_temp_dir = strdup(temp_dir);
    if(!_t3net_curl_temp_dir)
    {
      goto fail;
    }
  }
  return _t3net_setup(_t3net_curl_url_runner, _t3net_curl_exit_proc);

  fail:
  {
    return 0;
  }
}
