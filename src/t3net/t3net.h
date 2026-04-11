#ifndef _T3NET_H
#define _T3NET_H

#define T3NET_TIMEOUT_TIME      10
#define T3NET_MAX_ARGUMENTS    256
#define T3NET_MAX_POST_DATA    256

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

typedef struct
{

    char ** data;
    int count;

} T3NET_POST_DATA;

/* initialization */
int _t3net_setup(int (*url_runner)(const char * url, const char ** post_data, const char * out_path, char ** out_data), void (*exit_proc)(void));

/* utility */
char * _t3net_load_file(const char * fn);
int _t3net_run_system_command(char * command, const char * log_file);

/* debug logging */
int t3net_open_log_file(const char * fn);
void t3met_close_log_file(void);

/* build arguments list */
T3NET_ARGUMENTS * t3net_create_arguments(void);
void t3net_destroy_arguments(T3NET_ARGUMENTS * arguments);
int t3net_add_argument(T3NET_ARGUMENTS * arguments, const char * key, const char * val);

/* build post data */
T3NET_POST_DATA * t3net_create_post_data(void);
void t3net_destroy_post_data(T3NET_POST_DATA * post_data);
int t3net_add_post_data(T3NET_POST_DATA * post_data, const char * data);

/* low-level API */
int t3net_http_request(const char * url, T3NET_ARGUMENTS * arguments, T3NET_POST_DATA * post_data, char ** out_data);
T3NET_DATA * t3net_get_dataset(const char * raw_data);

/* high level API */
int t3net_download(const char * url, T3NET_ARGUMENTS * arguments, T3NET_POST_DATA * post_data, const char * out_path, char * error_out, int error_size);
T3NET_DATA * t3net_get_data(const char * url, T3NET_ARGUMENTS * arguments, T3NET_POST_DATA * post_data);
void t3net_destroy_data(T3NET_DATA * data);

/* dataset operations */
const char * t3net_get_error(T3NET_DATA * data);
const char * t3net_get_data_entry_field(T3NET_DATA * data, int entry, const char * field_name);

#endif
