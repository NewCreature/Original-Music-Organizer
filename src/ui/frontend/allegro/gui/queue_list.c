#include <math.h>
#include "instance.h"
#include "text_helpers.h"
#include "menu_proc.h"
#include "ui.h"

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

char * omo_get_queue_item_text(OMO_QUEUE * qp, int index, char * buffer)
{
	char display_fn[256] = {0};
	char prefix[16] = {0};
	char buf[64];

	sprintf(prefix, "%d. ", index + 1);
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
	else if(strlen(qp->entry[index]->tags.title))
	{
		if(strlen(qp->entry[index]->tags.album))
		{
			sprintf(buffer, "%s%s (%s)", prefix, qp->entry[index]->tags.title, qp->entry[index]->tags.album);
		}
		else
		{
			sprintf(buffer, "%s%s", prefix, qp->entry[index]->tags.title);
		}
	}
	else if(strlen(qp->entry[index]->tags.album))
	{
		if(strlen(qp->entry[index]->tags.track))
		{
			sprintf(buffer, "%s%s Track %s", prefix, qp->entry[index]->tags.album, qp->entry[index]->tags.track);
		}
		else
		{
			sprintf(buffer, "%sUnknown (%s)", prefix, qp->entry[index]->tags.album);
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
	strcat(buffer, omo_sec_to_clock(qp->entry[index]->tags.length + 0.5, buf, 64));
	return buffer;
}

void omo_queue_list_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char current_length[16];
	char total_length[16];
	char buf[64];
	int queue_pos = -1;

	if((!uip->app->player || !uip->app->player->queue) || uip->app->player->queue->entry_count < 1)
	{
		strcpy(uip->queue_info_text, "Queue Empty");
	}
	else
	{
		if(uip->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			queue_pos = uip->ui_queue_list_element->d1;
		}
		else if(uip->app->player->queue_pos < uip->app->player->queue->entry_count)
		{
			queue_pos = uip->app->player->queue_pos;
		}
		if(queue_pos >= 0)
		{
			strcpy(current_length, omo_sec_to_clock(uip->app->player->queue->entry[queue_pos]->tags.length + 0.5, buf, 64));
		}
		else
		{
			strcpy(current_length, "");
		}
		strcpy(total_length, omo_sec_to_clock(uip->app->player->queue->length + 0.5, buf, 64));
		if(queue_pos >= 0)
		{
			sprintf(uip->queue_info_text, "%d/%d\t%s/%s%s", queue_pos + 1, uip->app->player->queue->entry_count, current_length, total_length, uip->app->player->queue->untallied_length ? "+" : "");
		}
		else
		{
			sprintf(uip->queue_info_text, "%d Track%s\t%s%s", uip->app->player->queue->entry_count, uip->app->player->queue->entry_count == 1 ? "" : "s", total_length, uip->app->player->queue->untallied_length ? "+" : "");
		}
	}
	if(uip->app->player->queue && uip->ui_queue_list_element->id1 >= 0)
	{
		if(uip->app->player->state == OMO_PLAYER_STATE_PLAYING)
		{
			uip->app->player->queue_pos = uip->ui_queue_list_element->id1 - 1;
			omo_play_next_song(uip->app->player);
		}
		else if(uip->app->player->state == OMO_PLAYER_STATE_PAUSED)
		{
			uip->app->player->queue_pos = uip->ui_queue_list_element->id1 - 1;
			omo_play_next_song(uip->app->player);
			omo_start_player(uip->app->player);
		}
		else
		{
			uip->app->player->queue_pos = uip->ui_queue_list_element->id1;
			omo_start_player(uip->app->player);
		}
		uip->ui_queue_list_element->id1 = -1;
	}
	strcpy(uip->ui_button_text[0], strlen(uip->main_theme->text[0]) ? uip->main_theme->text[0] : "|<");
	if(uip->app->player->state == OMO_PLAYER_STATE_PLAYING)
	{
		uip->ui_button_element[1]->dp3 = uip->main_theme->bitmap[OMO_THEME_BITMAP_PAUSE];
		strcpy(uip->ui_button_text[1], strlen(uip->main_theme->text[1]) ? uip->main_theme->text[1] : "||");
	}
	else
	{
		uip->ui_button_element[1]->dp3 = uip->main_theme->bitmap[OMO_THEME_BITMAP_PLAY];
		strcpy(uip->ui_button_text[1], strlen(uip->main_theme->text[2]) ? uip->main_theme->text[2] : ">");
	}
	strcpy(uip->ui_button_text[2], strlen(uip->main_theme->text[3]) ? uip->main_theme->text[3] : "[]");
	strcpy(uip->ui_button_text[3], strlen(uip->main_theme->text[4]) ? uip->main_theme->text[4] : ">|");
	strcpy(uip->ui_button_text[4], strlen(uip->main_theme->text[5]) ? uip->main_theme->text[5] : "^");
	strcpy(uip->ui_button_text[5], strlen(uip->main_theme->text[6]) ? uip->main_theme->text[6] : "+");
	switch(uip->app->button_pressed)
	{
		case 0:
		{
			omo_play_previous_song(uip->app->player);
			break;
		}
		case 1:
		{
			switch(uip->app->player->state)
			{
				case OMO_PLAYER_STATE_STOPPED:
				{
					uip->app->player->queue_pos = uip->ui_queue_list_element->d1;
					omo_start_player(uip->app->player);
					break;
				}
				case OMO_PLAYER_STATE_PLAYING:
				{
					omo_pause_player(uip->app->player);
					break;
				}
				case OMO_PLAYER_STATE_PAUSED:
				{
					omo_resume_player(uip->app->player);
					break;
				}
			}
			break;
		}
		case 2:
		{
			uip->ui_queue_list_element->d1 = uip->app->player->queue_pos;
			omo_stop_player(uip->app->player);
			break;
		}
		case 3:
		{
			omo_play_next_song(uip->app->player);
			break;
		}
		case 4:
		{
			if(OMO_KEY_CTRL)
			{
				omo_menu_file_play_folder(0, data);
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
			}
			else
			{
				omo_menu_file_queue_files(0, data);
			}
			break;
		}
	}
	uip->app->button_pressed = -1;
}
