#ifndef T3NET_SERVER_LIST_H
#define T3NET_SERVER_LIST_H

#define T3NET_MAX_SERVERS 256

typedef struct
{

	char name[256];
	char address[256];
	int port;
	char capacity[32];
	int private;

} T3NET_SERVER_LIST_ENTRY;

typedef struct
{

	int curl_mode;
	char url[1024];
	char game[64];
	char version[64];

	T3NET_SERVER_LIST_ENTRY * entry[T3NET_MAX_SERVERS];
	int entries;

} T3NET_SERVER_LIST;

/* server list download functions */
T3NET_SERVER_LIST * t3net_get_server_list(int curl_mode, char * url, char * game, char * version);
int t3net_update_server_list_2(T3NET_SERVER_LIST * lp);
void t3net_clear_server_list(T3NET_SERVER_LIST * lp);
void t3net_destroy_server_list(T3NET_SERVER_LIST * lp);

/* server registration functions */
char * t3net_register_server(int curl_mode, char * url, int port, char * game, char * version, char * name, char * password, int permanent);
int t3net_update_server(int curl_mode, char * url, int port, char * key, char * capacity);
int t3net_unregister_server(int curl_mode, char * url, int port, char * key);

#endif
