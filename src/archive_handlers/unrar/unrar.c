#include "t3f/t3f.h"
#ifdef ALLEGRO_WINDOWS
	#include <windows.h>
#endif
#include "../archive_handler.h"

static OMO_ARCHIVE_HANDLER archive_handler;
static char cached_rar_file[1024];
#ifdef ALLEGRO_MACOSX
	static const char * command_prefix = "/usr/local/bin/";
#else
	static const char * command_prefix = "";
#endif

static int my_system(char * command)
{
	int ret;

	#ifdef ALLEGRO_WINDOWS
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.wShowWindow = SW_HIDE;
		ret = CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
	#else
		ret = system(command);
	#endif

	return ret;
}

static int count_files(const char * fn)
{
	ALLEGRO_FILE * fp;
	char system_command[1024];
	char line_buffer[256];
	char * line_pointer;
	int line_count = 0;
	if(strcmp(fn, cached_rar_file))
	{
		sprintf(system_command, "%sunrar l \"%s\" > \"%s\"", command_prefix, fn, t3f_get_filename(t3f_data_path, "rarlist.txt"));
		my_system(system_command);
		strcpy(cached_rar_file, fn);
	}
	fp = al_fopen(t3f_get_filename(t3f_data_path, "rarlist.txt"), "r");
	if(fp)
	{
		while(1)
		{
			line_pointer = al_fgets(fp, line_buffer, 256);
			if(line_pointer)
			{
				line_count++;
			}
			else
			{
				break;
			}
		}
		al_fclose(fp);
	}
	return line_count - 12;
}

static char returnfn[1024] = {0};

static void remove_line_endings(char * buffer)
{
	int i;

	for(i = 0; i < strlen(buffer); i++)
	{
		if(buffer[i] == '\r' || buffer[i] == '\n')
		{
			buffer[i] = 0;
		}
	}
	for(i = strlen(buffer) - 1; i >= 0; i--)
	{
		if(buffer[i] == ' ')
		{

			buffer[i] = '0';
		}
		else
		{
			break;
		}
	}
}

static const char * get_file(const char * fn, int index)
{
	ALLEGRO_FILE * fp;
	char system_command[1024];
	char line_buffer[256];
	char * line_pointer;
	int line_count = 0;
	int skip_lines = 9;
	int fn_offset;
	int version, subversion;
	int i;

	if(strcmp(fn, cached_rar_file))
	{
		sprintf(system_command, "%sunrar l \"%s\" > \"%s\"", command_prefix, fn, t3f_get_filename(t3f_data_path, "rarlist.txt"));
		my_system(system_command);
		strcpy(cached_rar_file, fn);
	}
	fp = al_fopen(t3f_get_filename(t3f_data_path, "rarlist.txt"), "r");
	if(fp)
	{
		while(1)
		{
			line_pointer = al_fgets(fp, line_buffer, 256);
			if(line_pointer)
			{
				line_count++;
				if(line_count == 2)
				{
					version = line_buffer[6];
					if(version == '4')
					{
						skip_lines = 8;
						fn_offset = 1;
					}
					else if(version == '5')
					{
						subversion = line_buffer[8];
						if(subversion == '0')
						{
							fn_offset = 39;
						}
						else
						{
							fn_offset = 41;
						}
					}
				}
				if(line_count - skip_lines == index)
				{
					strcpy(returnfn, &line_buffer[fn_offset]);

					/* remove leading or trailing characters depending on version */
					if(version != '4')
					{
						remove_line_endings(returnfn);
					}
					else
					{
						for(i = 0; i < strlen(returnfn); i++)
						{
							if(returnfn[i] == ' ')
							{
								returnfn[i] = 0;
								break;
							}
						}
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

	return returnfn;
}

static const char * extract_file(const char * fn, int index)
{
	char system_command[1024];
	char subfile[1024];
	char path_separator;

	#ifdef ALLEGRO_WINDOWS
		path_separator = '\\';
	#else
		path_separator = '/';
	#endif

	strcpy(subfile, get_file(fn, index));
	sprintf(system_command, "%sunrar x -inul -y \"%s\" \"%s\" \"%s\"", command_prefix, fn, subfile, al_path_cstr(t3f_data_path, path_separator));
//	printf(">%s\n", system_command);
	my_system(system_command);
	strcpy(returnfn, t3f_get_filename(t3f_data_path, subfile));
	return returnfn;
}

OMO_ARCHIVE_HANDLER * omo_get_unrar_archive_handler(void)
{
	memset(&archive_handler, 0, sizeof(OMO_ARCHIVE_HANDLER));
	archive_handler.count_files = count_files;
	archive_handler.get_file = get_file;
	archive_handler.extract_file = extract_file;
	omo_archive_handler_add_type(&archive_handler, ".rar");
	omo_archive_handler_add_type(&archive_handler, ".rsn");
	return &archive_handler;
}
