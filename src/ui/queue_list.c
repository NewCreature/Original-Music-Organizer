#include <math.h>
#include "../instance.h"
#include "menu_proc.h"

static void get_path_filename(const char * fn, char * outfn)
{
	int i, j;
	int pos = 0;
	char path_separator;

	#ifdef ALLEGRO_WINDOWS
		path_separator = '\\';
	#else
		path_separator = '/';
	#endif
	for(i = strlen(fn) - 1; i >= 0; i--)
	{
		if(fn[i] == path_separator)
		{
			for(j = i + 1; j < strlen(fn); j++)
			{
				outfn[pos] = fn[j];
				pos++;
			}
			outfn[pos] = 0;
			break;
		}
	}
}

static char sec_to_clock_buffer[64];

static const char * sec_to_clock(double sec)
{
    char hour[16] = {0};
    char minute[4] = {0};
    char second[4] = {0};
	if(sec <= 0.5)
	{
		return "--:--";
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
    sprintf(sec_to_clock_buffer, "%s%s%s", hour, minute, second);
    return sec_to_clock_buffer;
}

char * omo_get_queue_item_text(OMO_QUEUE * qp, int index, char * buffer)
{
	char display_fn[256] = {0};
	char prefix[16] = {0};
	char buf[64];

	sprintf(prefix, "%s", "");
	if(strlen(qp->entry[index]->tags.track))
	{
		strcat(prefix, qp->entry[index]->tags.track);
		strcat(prefix, ". ");
	}
	if(strlen(qp->entry[index]->tags.artist) && strlen(qp->entry[index]->tags.album))
	{
		if(strlen(qp->entry[index]->tags.title))
		{
			sprintf(buffer, "%s%s (%s - %s)", prefix, qp->entry[index]->tags.title, qp->entry[index]->tags.artist, qp->entry[index]->tags.album);
		}
		else if(strlen(qp->entry[index]->tags.track))
		{
			sprintf(buffer, "%sUnknown (%s - %s)", prefix, qp->entry[index]->tags.artist, qp->entry[index]->tags.album);
		}
		else
		{
			sprintf(buffer, "%sUnknown (%s - %s)", prefix, qp->entry[index]->tags.artist, qp->entry[index]->tags.album);
		}
	}
	else if(strlen(qp->entry[index]->tags.album) && strlen(qp->entry[index]->tags.track))
	{
		sprintf(buffer, "%sUnknown (Unknown - %s)", prefix, qp->entry[index]->tags.album);
	}
	else if(strlen(qp->entry[index]->tags.title))
	{
		if(strlen(qp->entry[index]->tags.album))
		{
			sprintf(buffer, "%s%s (Unknown - %s)", prefix, qp->entry[index]->tags.title, qp->entry[index]->tags.album);
		}
		else
		{
			sprintf(buffer, "%s%s", prefix, qp->entry[index]->tags.title);
		}
	}
	else
	{
		get_path_filename(qp->entry[index]->file, display_fn);
		sprintf(buffer, "%s%s", prefix, display_fn);
		if(qp->entry[index]->sub_file)
		{
			strcat(buffer, "/");
			strcat(buffer, qp->entry[index]->sub_file);
		}
		if(qp->entry[index]->track)
		{
			strcat(buffer, ":");
			strcat(buffer, qp->entry[index]->track);
		}
	}
	strcat(buffer, "\t");
	sprintf(buf, "%s", sec_to_clock(qp->entry[index]->tags.length + 0.5));
	strcat(buffer, buf);
	return buffer;
}

void omo_queue_list_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if((!app->player || !app->player->queue) || app->player->queue->entry_count < 1)
	{
		strcpy(app->ui->queue_info_text, "Queue Empty");
	}
	else
	{
		sprintf(app->ui->queue_info_text, "%d/%d", app->player->queue_pos + 1, app->player->queue->entry_count);
	}
	if(app->player->queue && app->ui->ui_queue_list_element->id1 >= 0)
	{
		if(app->player->state == OMO_PLAYER_STATE_PLAYING)
		{
			app->player->queue_pos = app->ui->ui_queue_list_element->id1 - 1;
			omo_play_next_song(app->player);
		}
		else
		{
			app->player->queue_pos = app->ui->ui_queue_list_element->id1;
			omo_start_player(app->player);
		}
		app->ui->ui_queue_list_element->id1 = -1;
	}
	strcpy(app->ui->ui_button_text[0], strlen(app->ui->main_theme->text[0]) ? app->ui->main_theme->text[0] : "|<");
	if(app->player->state == OMO_PLAYER_STATE_PLAYING)
	{
		app->ui->ui_button_element[1]->dp3 = app->ui->main_theme->bitmap[OMO_THEME_BITMAP_PAUSE];
		strcpy(app->ui->ui_button_text[1], strlen(app->ui->main_theme->text[1]) ? app->ui->main_theme->text[1] : "||");
	}
	else
	{
		app->ui->ui_button_element[1]->dp3 = app->ui->main_theme->bitmap[OMO_THEME_BITMAP_PLAY];
		strcpy(app->ui->ui_button_text[1], strlen(app->ui->main_theme->text[2]) ? app->ui->main_theme->text[2] : ">");
	}
	strcpy(app->ui->ui_button_text[2], strlen(app->ui->main_theme->text[3]) ? app->ui->main_theme->text[3] : "[]");
	strcpy(app->ui->ui_button_text[3], strlen(app->ui->main_theme->text[4]) ? app->ui->main_theme->text[4] : ">|");
	strcpy(app->ui->ui_button_text[4], strlen(app->ui->main_theme->text[5]) ? app->ui->main_theme->text[5] : "^");
	strcpy(app->ui->ui_button_text[5], strlen(app->ui->main_theme->text[6]) ? app->ui->main_theme->text[6] : "+");
	switch(app->button_pressed)
	{
		case 0:
		{
			omo_play_previous_song(app->player);
			break;
		}
		case 1:
		{
			switch(app->player->state)
			{
				case OMO_PLAYER_STATE_STOPPED:
				{
					app->player->queue_pos = app->ui->ui_queue_list_element->d1;
					omo_start_player(app->player);
					break;
				}
				case OMO_PLAYER_STATE_PLAYING:
				{
					omo_pause_player(app->player);
					break;
				}
				case OMO_PLAYER_STATE_PAUSED:
				{
					omo_resume_player(app->player);
					break;
				}
			}
			break;
		}
		case 2:
		{
			app->ui->ui_queue_list_element->d1 = app->player->queue_pos;
			omo_stop_player(app->player);
			break;
		}
		case 3:
		{
			omo_play_next_song(app->player);
			break;
		}
		case 4:
		{
			if(OMO_KEY_CTRL)
			{
				omo_menu_file_play_folder(0, data);
				t3f_key[ALLEGRO_KEY_COMMAND] = 0;
				t3f_key[ALLEGRO_KEY_LCTRL] = 0;
				t3f_key[ALLEGRO_KEY_RCTRL] = 0;
			}
			else
			{
				omo_menu_file_play_files(0, data);
			}
			break;
		}
		case 5:
		{
			if(OMO_KEY_CTRL)
			{
				omo_menu_file_queue_folder(0, data);
				t3f_key[ALLEGRO_KEY_COMMAND] = 0;
				t3f_key[ALLEGRO_KEY_LCTRL] = 0;
				t3f_key[ALLEGRO_KEY_RCTRL] = 0;
			}
			else
			{
				omo_menu_file_queue_files(0, data);
			}
			break;
		}
	}
	app->button_pressed = -1;
}
