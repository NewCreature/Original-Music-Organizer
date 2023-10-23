#include <stdio.h>

int main(int argc, char * argv[])
{
	FILE * fp;
	FILE * out;
	int c, next;
	int newline_count = 0;
	int line_pos = 0;
	
	fp = fopen(argv[1], "rb");
	if(!fp)
	{
		return 1;
	}
	out = fopen(argv[2], "wb");
	if(!out)
	{
		return 1;
	}
	
	while(1)
	{
		c = fgetc(fp);
		if(c == EOF)
		{
			break;
		}
		else if(c == '\r')
		{
		}
		else if(c == '\n')
		{
			newline_count++;
			line_pos = 0;
		}
		else
		{
			if(newline_count > 1)
			{
				fputc('\r', out);
				fputc('\n', out);
				fputc('\r', out);
				fputc('\n', out);
			}
			else if(newline_count == 1)
			{
				fputc(' ', out);
			}
			newline_count = 0;
			if(c == '-' && line_pos == 0)
			{
			}
			else
			{
				fputc(c, out);
				line_pos++;
			}
		}
	}
	fclose(fp);
	fclose(out);
	
	return 0;
}
