#ifndef T3NET_DLC_H
#define T3NET_DLC_H

#define T3NET_DLC_MAX_ITEMS 1024

typedef struct
{

	unsigned long hash;
	char name[256];
	char author[256];
	char description[1024];
	char url[1024];
	char preview_url[1024];
	void * preview;
	int flags;
	
} T3NET_DLC_ITEM;

typedef struct
{

	T3NET_DLC_ITEM * item[T3NET_DLC_MAX_ITEMS];
	int items;

} T3NET_DLC_LIST;

T3NET_DLC_LIST * t3net_get_dlc_list(const char * url, const char * game, int type);
void t3net_destroy_dlc_list(T3NET_DLC_LIST * lp, void (*callback)(void * data));
void t3net_remove_dlc_item(T3NET_DLC_LIST * lp, unsigned long hash);

/* download a file */
void t3net_set_download_callback(void (*callback)());
int t3net_download_file(const char * url, const char * fn);

#endif
