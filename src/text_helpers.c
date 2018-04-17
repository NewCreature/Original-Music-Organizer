#include <stdio.h>
#include <string.h>
#include <math.h>

char * omo_sec_to_clock(double sec, char * buffer, int size)
{
	const char * no_time_text = "--:--";
    char hour[16] = {0};
    char minute[4] = {0};
    char second[4] = {0};
	if(sec <= 0.5)
	{
		if(strlen(no_time_text) < size)
		{
			strcpy(buffer, no_time_text);
			return buffer;
		}
		strcpy(buffer, "");
		return buffer;
	}
    if(sec >= 3600.0)
    {
        sprintf(hour, "%d:", (int)(sec + 0.5) / 3600);
		if(sec >= 60.0)
		{
        	sprintf(minute, "%02d", (int)(fmod(sec, 3600.0)) / 60);
		}
    }
    else
    {
    	sprintf(minute, "%d", (int)(fmod(sec, 3600.0)) / 60);
    }
    strcat(minute, ":");
	if(sec > 0.0)
	{
		sprintf(second, "%02d", ((int)(fmod(sec, 3600.0))) % 60);
	}
	if(strlen(hour) + strlen(minute) + strlen(second) < size)
	{
    	sprintf(buffer, "%s%s%s", hour, minute, second);
		return buffer;
	}
	strcpy(buffer, "");
	return buffer;
}
