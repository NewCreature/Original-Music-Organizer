#include "instance.h"
#include "menu_proc.h"
#include "queue_list.h"
#include "text_helpers.h"
#include "ui.h"

void omo_player_ui_logic(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char text[1024] = {0};

	if(uip->app->player->track)
	{
		omo_get_queue_item_text(uip->app->player->queue, uip->app->player->queue_pos, text);
		strcpy(uip->song_info_text[0], text);
		if(uip->app->player->track->codec_handler->get_info)
		{
			strcpy(uip->song_info_text[1], uip->app->player->track->codec_handler->get_info(uip->app->player->track->codec_data));
		}
		else
		{
			strcpy(uip->song_info_text[1], "N/A");
		}
		if(uip->app->player->track->codec_handler->get_position)
		{
			sprintf(uip->current_time_text, "\t%s", omo_sec_to_clock(uip->app->player->track->codec_handler->get_position(uip->app->player->track->codec_data), text, 1024));
		}
		else
		{
			strcpy(uip->current_time_text, "");
		}
	}
	else
	{
		strcpy(uip->song_info_text[0], "");
		strcpy(uip->song_info_text[1], "");
	}
}
