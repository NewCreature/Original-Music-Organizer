#ifndef T3NET_H
#define T3NET_H

#define T3NET_TIMEOUT_TIME      10
#define T3NET_MAX_ARGUMENTS    256

#define T3NET_CURL_LIBCURL       0
#define T3NET_CURL_SYSTEM        1
#ifndef T3NET_NO_LIBCURL
  #define T3NET_CURL_DEFAULT T3NET_CURL_LIBCURL
#else
  #define T3NET_CURL_DEFAULT T3NET_CURL_SYSTEM
#endif

/* A chunk of memory this size will be allocated when retrieving data. Each
   time the size of the data exceeds this chunk size, the chunk will be
   reallocated to add this many bytes to the chunk. */
#define T3NET_DATA_CHUNK_SIZE 4096

typedef struct
{

    char * name;
    char * data;

} T3NET_DATA_ENTRY_FIELD;

typedef struct
{

    T3NET_DATA_ENTRY_FIELD ** field;
    int fields;

} T3NET_DATA_ENTRY;

typedef struct
{

    char * header;
    T3NET_DATA_ENTRY ** entry;
    int entries;

} T3NET_DATA;

typedef struct
{

    char * key[T3NET_MAX_ARGUMENTS];
    char * val[T3NET_MAX_ARGUMENTS];
    int count;

} T3NET_ARGUMENTS;

extern char t3net_server_message[1024];

/* initialization */
int t3net_setup(const char * curl_command, const char * temp_dir);
const char * t3net_get_curl_command(void);
char * t3net_escape(const char * s);

/* internet operations */
T3NET_ARGUMENTS * t3net_create_arguments(void);
void t3net_destroy_arguments(T3NET_ARGUMENTS * arguments);
int t3net_add_argument(T3NET_ARGUMENTS * arguments, const char * key, const char * val);
char * t3net_get_raw_data(int method, const char * url, const T3NET_ARGUMENTS * arguments);
T3NET_DATA * t3net_get_data_from_string(const char * s);
T3NET_DATA * t3net_get_data(int method, const char * url, const T3NET_ARGUMENTS * arguments);
void t3net_destroy_data(T3NET_DATA * data);

/* data set operations */
const char * t3net_get_error(T3NET_DATA * data);
const char * t3net_get_data_entry_field(T3NET_DATA * data, int entry, const char * field_name);

#endif
