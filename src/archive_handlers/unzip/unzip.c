#include "t3f/t3f.h"
#ifdef ALLEGRO_WINDOWS
	#include <windows.h>
#endif
#include "../archive_handler.h"

typedef struct
{

	const char * filename;
	ALLEGRO_PATH * temp_path;
	char command_prefix[1024];
	char command_postfix[8];

} ARCHIVE_HANDLER_DATA;

static int my_system(char * command, const char * log_file)
{
	int ret;

	#ifdef ALLEGRO_WINDOWS
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		HANDLE log_handle = CreateFile(log_file, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdInput = NULL;
		si.hStdOutput = log_handle;
		si.hStdError = log_handle;
		si.wShowWindow = SW_HIDE;
		ret = CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);

	#else
		char final_command[1024];
		strcpy(final_command, command);
		if(log_file)
		{
			strcat(final_command, " > \"");
			strcat(final_command, log_file);
			strcat(final_command, "\"");
		}
		ret = system(final_command);
	#endif

	return ret;
}

static void get_command_prefix(void * data)
{
	ARCHIVE_HANDLER_DATA * archive_data = (ARCHIVE_HANDLER_DATA *)data;

	strcpy(archive_data->command_prefix, "");
	strcpy(archive_data->command_postfix, "");
	#ifdef ALLEGRO_MACOSX
		ALLEGRO_PATH * path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
		if(path)
		{
			al_set_path_filename(path, "");
//			sprintf(archive_data->command_prefix, "\"%s", al_path_cstr(path, '/'));
//			strcpy(archive_data->command_postfix, "\"");
			al_destroy_path(path);
		}
	#endif
}

static void * open_archive(const char * fn, ALLEGRO_PATH * temp_path)
{
	ARCHIVE_HANDLER_DATA * data;
	ALLEGRO_PATH * path;
	char system_command[1024];

	data = malloc(sizeof(ARCHIVE_HANDLER_DATA));
	if(data)
	{
		data->filename = fn;
		data->temp_path = temp_path;
		get_command_prefix(data);

		/* create ziplist.txt */
		path = al_clone_path(temp_path);
		if(path)
		{
			al_set_path_filename(path, "ziplist.txt");
			al_remove_filename(al_path_cstr(path, '/'));
			sprintf(system_command, "%sunzip%s -l \"%s\"", data->command_prefix, data->command_postfix, fn);
			my_system(system_command, al_path_cstr(path, '/'));
			if(!al_filename_exists(al_path_cstr(path, '/')))
			{
				free(data);
				data = NULL;
			}
			al_destroy_path(path);
		}
	}
	return data;
}

static void close_archive(void * data)
{
	ARCHIVE_HANDLER_DATA * archive_data = (ARCHIVE_HANDLER_DATA *)data;
	ALLEGRO_PATH * path;

	/* destroy ziplist.txt */
	path = al_clone_path(archive_data->temp_path);
	if(path)
	{
		al_set_path_filename(path, "ziplist.txt");
		al_remove_filename(al_path_cstr(path, '/'));
		al_destroy_path(path);
	}
	free(data);
}

static int count_files(void * data)
{
	ARCHIVE_HANDLER_DATA * archive_data = (ARCHIVE_HANDLER_DATA *)data;
	ALLEGRO_FILE * fp;
	ALLEGRO_PATH * path;
	char line_buffer[256];
	char * line_pointer;
	int line_count = 0;

	path = al_clone_path(archive_data->temp_path);
	if(!path)
	{
		return 0;
	}
	al_set_path_filename(path, "ziplist.txt");
	fp = al_fopen(al_path_cstr(path, '/'), "r");
	if(fp)
	{
		while(1)
		{
			line_pointer = al_fgets(fp, line_buffer, 256);
			if(line_pointer)
			{
				if(!strstr(line_pointer, "__MACOSX"))
				{
					line_count++;
				}
			}
			else
			{
				break;
			}
		}
		al_fclose(fp);
	}
	al_destroy_path(path);
	return line_count - 4;
}

static void remove_line_endings(char * buffer)
{
	int i, l;

  l = strlen(buffer);
	for(i = 0; i < l; i++)
	{
		if(buffer[i] == '\r' || buffer[i] == '\n')
		{
			buffer[i] = 0;
		}
	}
}

static const char * get_file(void * data, int index, char * buffer)
{
	ARCHIVE_HANDLER_DATA * archive_data = (ARCHIVE_HANDLER_DATA *)data;
	ALLEGRO_FILE * fp;
	ALLEGRO_PATH * path;
	char line_buffer[256];
	char * line_pointer;
	int line_count = 0;
	int skip_lines = 4;
	int fn_offset = 30;
	int i;
	path = al_clone_path(archive_data->temp_path);
	if(!path)
	{
		return NULL;
	}
	al_set_path_filename(path, "ziplist.txt");

	strcpy(buffer, "");
	fp = al_fopen(al_path_cstr(path, '/'), "r");
	if(fp)
	{
		while(1)
		{
			line_pointer = al_fgets(fp, line_buffer, 256);
			if(line_pointer)
			{
				if(!strstr(line_pointer, "__MACOSX"))
				{
					line_count++;
					if(line_count - skip_lines == index)
					{
						strcpy(buffer, &line_buffer[fn_offset]);
						remove_line_endings(buffer);
						break;
					}
				}
			}
			else
			{
				break;
			}
		}
		al_fclose(fp);
	}
	al_destroy_path(path);

	return buffer;
}

static const char * extract_file(void * data, int index, char * buffer)
{
	ARCHIVE_HANDLER_DATA * archive_data = (ARCHIVE_HANDLER_DATA *)data;
	ALLEGRO_PATH * path;
	char system_command[1024];
	char subfile[1024];
	char path_separator;

	#ifdef ALLEGRO_WINDOWS
		path_separator = '\\';
	#else
		path_separator = '/';
	#endif

	path = al_clone_path(archive_data->temp_path);
	if(!path)
	{
		return 0;
	}

	strcpy(subfile, get_file(archive_data, index, buffer));
	al_set_path_filename(path, subfile);
	sprintf(system_command, "%sunzip%s -o \"%s\" \"%s\" -d \"%s\"", archive_data->command_prefix, archive_data->command_postfix, archive_data->filename, subfile, al_path_cstr(archive_data->temp_path, path_separator));
	my_system(system_command, NULL);
	strcpy(buffer, al_path_cstr(path, '/'));
	al_destroy_path(path);

	return buffer;
}

static OMO_ARCHIVE_HANDLER archive_handler;

OMO_ARCHIVE_HANDLER * omo_get_unzip_archive_handler(void)
{
	memset(&archive_handler, 0, sizeof(OMO_ARCHIVE_HANDLER));
	strcpy(archive_handler.id, "unzip");
	archive_handler.open_archive = open_archive;
	archive_handler.close_archive = close_archive;
	archive_handler.count_files = count_files;
	archive_handler.get_file = get_file;
	archive_handler.extract_file = extract_file;
	omo_archive_handler_add_type(&archive_handler, ".zip");
	omo_archive_handler_add_type(&archive_handler, ".zsn");
	return &archive_handler;
}
