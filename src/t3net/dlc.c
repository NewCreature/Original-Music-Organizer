#include <curl/curl.h>
#include <memory.h>
#include <stdlib.h>
#include "t3net.h"
#include "dlc.h"

T3NET_DLC_LIST * t3net_get_dlc_list(const char * url, const char * game, int type)
{
	T3NET_DLC_LIST * lp = NULL;
	T3NET_ARGUMENTS * args = NULL;
	T3NET_DATA * data = NULL;
	const char * val;
	int ecount = 0;
	char buf[256] = {0};
	int i;

	lp = malloc(sizeof(T3NET_DLC_LIST));
	if(!lp)
	{
		goto fail_out;
	}
	lp->items = 0;

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail_out;
	}
	if(!t3net_add_argument(args, "project_id", game))
	{
		goto fail_out;
	}
	sprintf(buf, "%d", type);
	if(!t3net_add_argument(args, "type", buf))
	{
		goto fail_out;
	}
	data = t3net_get_data(url, args);
	if(!data)
	{
		goto fail_out;
	}

    /* create the DLC list */
	for(i = 0; i < data->entries; i++)
	{
		lp->item[ecount] = malloc(sizeof(T3NET_DLC_ITEM));
		memset(lp->item[ecount], 0, sizeof(T3NET_DLC_ITEM));
		val = t3net_get_data_entry_field(data, i, "name");
		if(val)
		{
			strcpy(lp->item[ecount]->name, val);
		}
		val = t3net_get_data_entry_field(data, i, "author");
		if(val)
		{
			strcpy(lp->item[ecount]->author, val);
		}
		val = t3net_get_data_entry_field(data, i, "description");
		if(val)
		{
			strcpy(lp->item[ecount]->description, val);
		}
		val = t3net_get_data_entry_field(data, i, "url");
		if(val)
		{
			strcpy(lp->item[ecount]->url, val);
		}
		val = t3net_get_data_entry_field(data, i, "preview_url");
		if(val)
		{
			strcpy(lp->item[ecount]->preview_url, val);
		}
		val = t3net_get_data_entry_field(data, i, "hash");
		if(val)
		{
			lp->item[ecount]->hash = atol(val);
		}
		ecount++;
	}
	lp->items = ecount;

	/* free memory */
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);

	return lp;

	fail_out:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		if(lp)
		{
			free(lp);
		}
		return NULL;
	}
}

void t3net_destroy_dlc_list(T3NET_DLC_LIST * lp, void (*callback)(void * data))
{
	int i;

	for(i = 0; i < lp->items; i++)
	{
		if(lp->item[i]->preview && callback)
		{
			callback(lp->item[i]->preview);
		}
		free(lp->item[i]);
	}
	free(lp);
}

/* easy way to remove DLC items we already have from the list */
void t3net_remove_dlc_item(T3NET_DLC_LIST * lp, unsigned long hash)
{
	int i, j;

	for(i = 0; i < lp->items; i++)
	{
		if(lp->item[i]->hash == hash)
		{
			free(lp->item[i]);
			for(j = i; j < lp->items - 1; j++)
			{
				lp->item[j] = lp->item[j + 1];
			}
			lp->items--;
			return;
		}
	}
}

static void (*t3net_download_callback)() = NULL;

void t3net_set_download_callback(void (*callback)())
{
	t3net_download_callback = callback;
}

static size_t t3net_file_write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	if(t3net_download_callback)
	{
		t3net_download_callback();
	}
	return written;
}

int t3net_download_file(const char * url, const char * fn)
{
	CURL *curl;
	FILE * fp;
	CURLcode res;
	curl = curl_easy_init();
    if(curl)
    {
		fp = fopen(fn, "wb");
		if(fp)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,   1); // follow redirects
			curl_easy_setopt(curl, CURLOPT_AUTOREFERER,      1); // set the Referer: field in requests where it follows a Location: redirect.
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS,        20);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_file_write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
			if(res == 0)
			{
				return 1;
			}
		}
    }
	return 0;
}
