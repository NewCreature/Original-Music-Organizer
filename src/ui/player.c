#include "../instance.h"
#include "menu_proc.h"
#include "queue_list.h"
#include "../text_helpers.h"

void omo_player_ui_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char text[1024] = {0};

	if(app->player->track)
	{
		omo_get_queue_item_text(app->player->queue, app->player->queue_pos, text);
		strcpy(app->ui->song_info_text[0], text);
		if(app->player->track->codec_handler->get_info)
		{
			strcpy(app->ui->song_info_text[1], app->player->track->codec_handler->get_info(app->player->track->codec_data));
		}
		else
		{
			strcpy(app->ui->song_info_text[1], "N/A");
		}
		if(app->player->track->codec_handler->get_position)
		{
			sprintf(app->ui->current_time_text, "\t%s", omo_sec_to_clock(app->player->track->codec_handler->get_position(app->player->track->codec_data), text, 1024));
		}
		else
		{
			strcpy(app->ui->current_time_text, "");
		}
	}
	else
	{
		strcpy(app->ui->song_info_text[0], "");
		strcpy(app->ui->song_info_text[1], "");
	}
}
