#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fix_description(char * description)
{
	char original_description[1024] = {0};
	int i, j = 0, w = 0, last_space = 0, last_jspace = 0;

	strcpy(original_description, description);
	for(i = 0; i < strlen(original_description); i++)
	{
		/* copy current letter to destination */
		description[j] = original_description[i];

		/* if we find a space, store it so we can go back if we go past 80 characters */
		if(original_description[i] == ' ')
		{
			last_space = i;
			last_jspace = j;
			j++;
		}
		else if(original_description[i] == '\n')
		{
			last_space = i;
			last_jspace = j;
			j++;
			description[j] = ' ';
			j++;
			w = 1;
		}
		else
		{
			j++;
		}

		w++;

		/* if the width of the current line is more than 80 characters, make a new line */
		if(w > 80)
		{
			i = last_space;
			j = last_jspace;
			description[j] = '\n';
			j++;
			description[j] = ' ';
			j++;
			w = 1;
		}
	}
}

void generate_control_file(int argc, char * argv[])
{
	FILE * fp;
	char fn[1024] = {0};
	char line[1024] = {0};

	sprintf(fn, "%s/control", argv[1]);
	fp = fopen(fn, "wb");
	if(!fp)
	{
		printf("file open error\n");
		exit(-2);
	}
	sprintf(line, "Package: %s\n", argv[2]);
	fputs(line, fp);
	sprintf(line, "Version: %s\n", argv[3]);
	fputs(line, fp);
	sprintf(line, "Section: %s\n", argv[4]);
	fputs(line, fp);
	sprintf(line, "Priority: optional\n");
	fputs(line, fp);
	sprintf(line, "Architecture: %s\n", argv[6]);
	fputs(line, fp);
	sprintf(line, "Depends: %s\n", argv[7]);
	fputs(line, fp);
	sprintf(line, "Installed-Size: %d\n", atoi(argv[8]));
	fputs(line, fp);
	sprintf(line, "Maintainer: %s\n", argv[9]);
	fputs(line, fp);
	memset(line, 0, 1024);
	sprintf(line, "Description: %s\n%s\n", argv[10], argv[11]);
	fix_description(line);
	fputs(line, fp);
	fclose(fp);
}

void generate_menu(int argc, char * argv[])
{
}

void generate_desktop(int argc, char * argv[])
{
}

int main(int argc, char * argv[])
{
	if(argc < 12)
	{
		printf("Usage: control_gen <out_path> <package_name> <package_version> <section> <priority> <architecture> <depends> <install_size> <maintainer> <description> <long_description>\n");
		return -1;
	}
	else if(strcasecmp(argv[6], "i386") && strcasecmp(argv[6], "amd64"))
	{
		printf("Usage: make package DEBIAN_ARCHITECTURE=arch (i386 or amd64)\n");
		return -2;
	}
	generate_control_file(argc, argv);
	return 0;
}
