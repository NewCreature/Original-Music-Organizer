#include "t3f/t3f.h"
#include "t3gui/t3gui.h"
#include "../frontend.h"
#include "gui/dialog_proc.h"
#include "gui/ui.h"
#include "instance.h"
#include "allegro.h"
#include "gui/menu_init.h"
#include "gui/tags_dialog.h"
#include "gui/split_track_dialog.h"
#include "gui/tagger_key_dialog.h"
#include "gui/new_profile_dialog.h"
#include "gui/multi_tags_dialog.h"
#include "gui/album_tags_dialog.h"
#include "gui/rebase_song_folder_dialog.h"
#include "gui/filter_dialog.h"
#include "gui/about_dialog.h"
#include "gui/library.h"
#include "gui/queue_list.h"
#include "gui/shortcut.h"
#include "gui/player.h"
#include "instance.h"
#include "init.h"

#define OMO_BEZEL_TOP    1
#define OMO_BEZEL_BOTTOM 2
#define OMO_BEZEL_LEFT   4
#define OMO_BEZEL_RIGHT  8

#define OMO_UI_MAX_TAGS            16
#define OMO_UI_MAX_TAG_LENGTH    1024
#define OMO_UI_SEEK_RESOLUTION   2000
#define OMO_UI_VOLUME_RESOLUTION  100

typedef struct
{

  OMO_UI * ui;

} OMO_FRONTEND_DATA;

static void * frontend_init(void * app_instance, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = NULL;
  const char * val;

  frontend_data = malloc(sizeof(OMO_FRONTEND_DATA));
  if(!frontend_data)
  {
    goto fail;
  }
  memset(frontend_data, 0, sizeof(OMO_FRONTEND_DATA));
  frontend_data->ui = omo_create_ui(app_instance);
  if(!frontend_data->ui)
  {
    goto fail;
  }
  if(!omo_create_main_dialog(frontend_data->ui, frontend_data->ui->app->library_view ? 1 : 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), app_instance))
	{
		printf("Unable to create main dialog!\n");
    goto fail;
	}
  omo_set_window_constraints(frontend_data->ui, frontend_data->ui->app->library_view);
	t3gui_show_dialog(frontend_data->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app_instance);
  if(!omo_setup_menus(frontend_data->ui))
	{
		printf("Error setting up menus!\n");
    goto fail;
	}
	val = al_get_config_value(t3f_config, "Settings", "disable_menu");
	if(!val || strcmp(val, "true"))
	{
		t3f_attach_menu(frontend_data->ui->menu[OMO_MENU_MAIN]);
	}

  /* set initial state of UI */
  frontend_data->ui->ui_queue_list_element->d1 = frontend_data->ui->app->player->queue_pos;
  val = al_get_config_value(t3f_config, "Settings", "queue_scroll_position");
  if(val)
  {
    frontend_data->ui->ui_queue_list_element->d2 = atoi(val);
  }
  else
  {
    frontend_data->ui->ui_queue_list_element->d2 = frontend_data->ui->ui_queue_list_element->d1;
  }
  t3f_set_menu_data(frontend_data->ui);

  return frontend_data;

  fail:
  {
    if(frontend_data)
    {
      free(frontend_data);
    }
    return NULL;
  }
}

static void frontend_exit(void * data)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;
  char buf[1024];

  if(frontend_data)
  {
    sprintf(buf, "%d", frontend_data->ui->ui_queue_list_element->d2);
    al_set_config_value(t3f_config, "Settings", "queue_scroll_position", buf);

    if(frontend_data->ui->tags_popup_dialog)
    {
      omo_close_tags_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    if(frontend_data->ui->multi_tags_popup_dialog)
    {
      omo_close_tags_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    if(frontend_data->ui->split_track_popup_dialog)
    {
      omo_close_split_track_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    if(frontend_data->ui->tagger_key_popup_dialog)
    {
      omo_close_tagger_key_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    if(frontend_data->ui->new_profile_popup_dialog)
    {
      omo_close_new_profile_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    if(frontend_data->ui->filter_popup_dialog)
    {
      omo_close_filter_dialog(frontend_data->ui, frontend_data->ui->app);
    }
    t3gui_close_dialog(frontend_data->ui->ui_dialog);
    free(frontend_data);
  }
}

static int queue_list_visible_elements(T3GUI_ELEMENT * element)
{
	return element->h / element->ed2;
}

static void update_seek_pos(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	double pos, length;

	if(uip->ui_seeked)
	{
		if(uip->app->player->track->codec_handler->seek)
		{
			length = uip->app->player->track->codec_handler->get_length(uip->app->player->track->codec_data);
			pos = ((double)uip->ui_seek_control_element->d2 / (double)OMO_UI_SEEK_RESOLUTION) * length;
			uip->app->player->track->codec_handler->seek(uip->app->player->track->codec_data, pos);
		}
		uip->ui_seeked = false;
	}
	else
	{
		if(uip->app->player->track)
		{
			if(uip->app->player->track->codec_handler->get_length && uip->app->player->track->codec_handler->get_position)
			{
				uip->ui_seek_control_element->flags &= ~D_DISABLED;
				if(!(uip->ui_seek_control_element->flags & D_TRACKMOUSE))
				{
					length = uip->app->player->track->codec_handler->get_length(uip->app->player->track->codec_data);
					pos = uip->app->player->track->codec_handler->get_position(uip->app->player->track->codec_data);
					uip->ui_seek_control_element->d2 = (pos / length) * (double)OMO_UI_SEEK_RESOLUTION;
				}
			}
			else
			{
				uip->ui_seek_control_element->flags |= D_DISABLED;
			}
		}
		else
		{
			uip->ui_seek_control_element->flags |= D_DISABLED;
		}
	}
}

static void update_volume_pos(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	double volume;
	char buf[128];
	const char * val;

	if(uip->ui_volume_changed)
	{
		volume = 1.0 - ((double)uip->ui_volume_control_element->d2 / (double)OMO_UI_VOLUME_RESOLUTION);
		if(uip->app->player->track && uip->app->player->track->codec_handler->set_volume)
		{
			uip->app->player->track->codec_handler->set_volume(uip->app->player->track->codec_data, volume);
		}
		sprintf(buf, "%f", volume);
		al_set_config_value(t3f_config, "Settings", "volume", buf);
		uip->ui_volume_changed = false;
	}
	else
	{
		val = al_get_config_value(t3f_config, "Settings", "volume");
		if(val)
		{
			uip->ui_volume_control_element->d2 = (1.0 - atof(val)) * OMO_UI_VOLUME_RESOLUTION;
		}
	}
	if(uip->app->player->track && !uip->app->player->track->codec_handler->set_volume)
	{
		uip->ui_volume_control_element->flags |= D_DISABLED;
	}
	else
	{
		uip->ui_volume_control_element->flags &= ~D_DISABLED;
	}
}

static void fix_scroll_position(T3GUI_ELEMENT * element, int entries)
{
	int visible = element->h / al_get_font_line_height(element->theme->state[T3GUI_ELEMENT_STATE_NORMAL].font[0]) - 1;

	if(entries > visible)
	{
		element->d2 = element->d1;
		if(element->d2 + visible > entries)
		{
			element->d2 = entries - visible;
		}
	}
}

static void find_logic(OMO_FRONTEND_DATA * frontend_data)
{
  int i;

  if(frontend_data->ui->app->find_track_id)
  {
    strcpy(frontend_data->ui->ui_artist_search_element->dp, "");
    strcpy(frontend_data->ui->ui_album_search_element->dp, "");
    strcpy(frontend_data->ui->ui_song_search_element->dp, "");
		frontend_data->ui->ui_artist_list_element->d1 = 0;
		frontend_data->ui->ui_artist_list_element->d2 = 0;
		frontend_data->ui->ui_album_list_element->d1 = 0;
		frontend_data->ui->ui_album_list_element->d2 = 0;
		frontend_data->ui->ui_song_list_element->d1 = 0;
		frontend_data->ui->ui_song_list_element->d2 = 0;

    /* select artist */
		if(frontend_data->ui->app->find_track_artist)
		{
			for(i = 0; i < frontend_data->ui->app->library->artist_entry_count; i++)
			{
				if(!strcmp(frontend_data->ui->app->find_track_artist, frontend_data->ui->app->library->artist_entry[i]))
				{
					frontend_data->ui->ui_artist_list_element->d1 = i;
					fix_scroll_position(frontend_data->ui->ui_artist_list_element, frontend_data->ui->app->library->artist_entry_count);
					break;
				}
			}
      frontend_data->ui->app->find_track_artist = NULL;
		}

    /* select album */
    if(frontend_data->ui->app->find_track_album)
		{
			for(i = 0; i < frontend_data->ui->app->library->album_entry_count; i++)
			{
				if(!strcmp(frontend_data->ui->app->find_track_album, frontend_data->ui->app->library->album_entry[i].name) && (frontend_data->ui->app->find_track_disambiguation ? !strcmp(frontend_data->ui->app->find_track_disambiguation, frontend_data->ui->app->library->album_entry[i].disambiguation) : 1))
				{
					frontend_data->ui->ui_album_list_element->d1 = i;
					fix_scroll_position(frontend_data->ui->ui_album_list_element, frontend_data->ui->app->library->album_entry_count);
					break;
				}
			}
      frontend_data->ui->app->find_track_album = NULL;
      frontend_data->ui->app->find_track_disambiguation = NULL;
		}

    /* select song */
		for(i = 0; i < frontend_data->ui->app->library->song_entry_count; i++)
		{
			if(!strcmp(frontend_data->ui->app->library->entry[frontend_data->ui->app->library->song_entry[i]]->id, frontend_data->ui->app->find_track_id))
			{
				frontend_data->ui->ui_song_list_element->d1 = i + 1;
				fix_scroll_position(frontend_data->ui->ui_song_list_element, frontend_data->ui->app->library->song_entry_count);
				break;
			}
		}

    frontend_data->ui->app->find_track_id = NULL;
  }
}

static void frontend_logic(void * data, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;
  int old_queue_list_pos = -1;
  int visible = 0;
  int seek_flags;
  int volume_pos;
  bool disable_shortcuts = false;

  omo_library_pre_gui_logic(frontend_data->ui);
  if(frontend_data->ui->app->player->queue)
  {
    old_queue_list_pos = frontend_data->ui->app->player->queue_pos;
    visible = queue_list_visible_elements(frontend_data->ui->ui_queue_list_element);
    if(visible > frontend_data->ui->app->player->queue->entry_count)
    {
      visible = frontend_data->ui->app->player->queue->entry_count;
    }
  }
  seek_flags = frontend_data->ui->ui_seek_control_element->flags;
  volume_pos = frontend_data->ui->ui_volume_control_element->d2;
  t3gui_logic();
  if(seek_flags & D_TRACKMOUSE && !(frontend_data->ui->ui_seek_control_element->flags & D_TRACKMOUSE))
  {
    frontend_data->ui->ui_seeked = true;
  }
  if(volume_pos != frontend_data->ui->ui_volume_control_element->d2)
  {
    frontend_data->ui->ui_volume_changed = true;
  }
  if(frontend_data->ui->tags_popup_dialog)
  {
    omo_tags_dialog_logic(data);
  }
  else if(frontend_data->ui->multi_tags_popup_dialog)
  {
    omo_multi_tags_dialog_logic(data);
  }
  else if(frontend_data->ui->album_tags_popup_dialog)
  {
    omo_album_tags_dialog_logic(data);
  }
  else if(frontend_data->ui->split_track_popup_dialog)
  {
    omo_split_track_dialog_logic(data);
  }
  else if(frontend_data->ui->tagger_key_popup_dialog)
  {
    omo_tagger_key_dialog_logic(data);
  }
  else if(frontend_data->ui->new_profile_popup_dialog)
  {
    omo_new_profile_dialog_logic(data);
  }
  else if(frontend_data->ui->rebase_song_folder_popup_dialog)
  {
    omo_rebase_song_folder_dialog_logic(data);
  }
  else if(frontend_data->ui->filter_popup_dialog)
  {
    omo_filter_dialog_logic(data);
  }
  else if(frontend_data->ui->about_popup_dialog)
  {
    omo_about_dialog_logic(data);
  }
  else
  {
    if(frontend_data->ui->app->library_view)
    {
      if(frontend_data->ui->app->library && frontend_data->ui->app->library->loaded)
      {
        omo_library_logic(frontend_data->ui);
      }
      if(frontend_data->ui->ui_artist_search_element->flags & D_GOTFOCUS)
      {
        disable_shortcuts = true;
      }
      else if(frontend_data->ui->ui_album_search_element->flags & D_GOTFOCUS)
      {
        disable_shortcuts = true;
      }
      else if(frontend_data->ui->ui_song_search_element->flags & D_GOTFOCUS)
      {
        disable_shortcuts = true;
      }
    }
    omo_queue_list_logic(frontend_data->ui);
    if(!disable_shortcuts)
    {
      omo_shortcut_logic(frontend_data->ui);
    }
    omo_player_ui_logic(frontend_data->ui);
    if(frontend_data->ui->app->library_view)
    {
      frontend_data->ui->ui_artist_list_element->id2 = frontend_data->ui->ui_artist_list_element->d1;
      frontend_data->ui->ui_album_list_element->id2 = frontend_data->ui->ui_album_list_element->d1;
      frontend_data->ui->ui_song_list_element->id2 = frontend_data->ui->ui_song_list_element->d1;
    }
  }
  update_seek_pos(frontend_data->ui);
  update_volume_pos(frontend_data->ui);
  frontend_data->ui->ui_queue_list_element->id2 = frontend_data->ui->app->player->queue_pos;

  /* see if we should scroll the queue list */
  if(frontend_data->ui->app->player->queue && frontend_data->ui->app->player->queue_pos != old_queue_list_pos)
  {
    if(old_queue_list_pos >= frontend_data->ui->ui_queue_list_element->d2 && old_queue_list_pos < frontend_data->ui->ui_queue_list_element->d2 + visible)
    {
      /* go to previous page */
      if(frontend_data->ui->app->player->queue_pos < frontend_data->ui->ui_queue_list_element->d2)
      {
        frontend_data->ui->ui_queue_list_element->d2 -= visible;
        if(frontend_data->ui->ui_queue_list_element->d2 < 0)
        {
          frontend_data->ui->ui_queue_list_element->d2 = 0;
        }
      }

      /* go to next page */
      if(frontend_data->ui->app->player->queue_pos > frontend_data->ui->ui_queue_list_element->d2 + visible - 1)
      {
        frontend_data->ui->ui_queue_list_element->d2 += visible;
        if(frontend_data->ui->ui_queue_list_element->d2 + visible > frontend_data->ui->app->player->queue->entry_count)
        {
          frontend_data->ui->ui_queue_list_element->d2 = frontend_data->ui->app->player->queue->entry_count - visible;
        }
      }
    }
  }
  if(frontend_data->ui->app->player->queue)
  {
    if(frontend_data->ui->ui_queue_list_element->d2 + visible > frontend_data->ui->app->player->queue->entry_count)
    {
      frontend_data->ui->ui_queue_list_element->d2 = frontend_data->ui->app->player->queue->entry_count - visible - 1;
      if(frontend_data->ui->ui_queue_list_element->d2 < 0)
      {
        frontend_data->ui->ui_queue_list_element->d2 = 0;
      }
    }
  }

  /* see if we found a track in the library */
  find_logic(frontend_data);
}

static void frontend_render(void * data, int flags)
{
  OMO_FRONTEND_DATA * frontend_data = (OMO_FRONTEND_DATA *)data;

  t3gui_render(NULL);
  if(frontend_data->ui->tags_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->tags_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->multi_tags_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->multi_tags_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->album_tags_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->album_tags_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->split_track_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->split_track_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->tagger_key_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->tagger_key_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->new_profile_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->new_profile_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->rebase_song_folder_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->rebase_song_folder_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->filter_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->filter_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
  if(frontend_data->ui->about_popup_dialog)
  {
    al_set_target_bitmap(al_get_backbuffer(frontend_data->ui->about_popup_dialog->display));
    al_flip_display();
    al_set_target_bitmap(al_get_backbuffer(t3f_display));
  }
}

static OMO_FRONTEND frontend;

OMO_FRONTEND * omo_get_allegro_frontend(void * app, int flags)
{
	memset(&frontend, 0, sizeof(OMO_FRONTEND));
	strcpy(frontend.id, "Allegro");
	frontend.init = frontend_init;
	frontend.exit = frontend_exit;
  frontend.logic = frontend_logic;
  frontend.render = frontend_render;
  frontend.data = frontend.init(app, flags);
  if(!frontend.data)
  {
    goto fail;
  }
  frontend.app = app;
  return &frontend;

  fail:
  {
    return NULL;
  }
}