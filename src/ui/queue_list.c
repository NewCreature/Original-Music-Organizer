#include <math.h>
#include "../instance.h"
#include "../text_helpers.h"
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
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char current_length[16];
	char total_length[16];
	char buf[64];
	int queue_pos = -1;

	if((!app->player || !app->player->queue) || app->player->queue->entry_count < 1)
	{
		strcpy(app->ui->queue_info_text, "Queue Empty");
	}
	else
	{
		if(app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
		{
			queue_pos = app->ui->ui_queue_list_element->d1;
		}
		else if(app->player->queue_pos < app->player->queue->entry_count)
		{
			queue_pos = app->player->queue_pos;
		}
		if(queue_pos >= 0)
		{
			strcpy(current_length, omo_sec_to_clock(app->player->queue->entry[queue_pos]->tags.length + 0.5, buf, 64));
		}
		else
		{
			strcpy(current_length, "");
		}
		strcpy(total_length, omo_sec_to_clock(app->player->queue->length + 0.5, buf, 64));
		if(queue_pos >= 0)
		{
			sprintf(app->ui->queue_info_text, "%d/%d\t%s/%s%s", queue_pos + 1, app->player->queue->entry_count, current_length, total_length, app->player->queue->untallied_length ? "+" : "");
		}
		else
		{
			sprintf(app->ui->queue_info_text, "%d Track%s\t%s%s", app->player->queue->entry_count, app->player->queue->entry_count == 1 ? "" : "s", total_length, app->player->queue->untallied_length ? "+" : "");
		}
	}
	if(app->player->queue && app->ui->ui_queue_list_element->id1 >= 0)
	{
		if(app->player->state == OMO_PLAYER_STATE_PLAYING)
		{
			app->player->queue_pos = app->ui->ui_queue_list_element->id1 - 1;
			omo_play_next_song(app->player);
		}
		else if(app->player->state == OMO_PLAYER_STATE_PAUSED)
		{
			app->player->queue_pos = app->ui->ui_queue_list_element->id1 - 1;
			omo_play_next_song(app->player);
			omo_start_player(app->player);
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
