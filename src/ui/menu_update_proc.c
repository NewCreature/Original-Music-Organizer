#include "../t3f/t3f.h"
#include "../instance.h"

/* called by other update procs */
int omo_menu_base_update_disable_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->ui->tags_popup_dialog || app->ui->multi_tags_popup_dialog || app->ui->split_track_popup_dialog || app->ui->tagger_key_popup_dialog || app->ui->new_profile_popup_dialog || app->ui->filter_popup_dialog)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
		return 0;
	}
	return 1;
}

/* called by any menu items without their own update ogic */
int omo_menu_base_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->ui->tags_popup_dialog || app->ui->multi_tags_popup_dialog || app->ui->split_track_popup_dialog || app->ui->tagger_key_popup_dialog || app->ui->new_profile_popup_dialog || app->ui->filter_popup_dialog)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
		return 0;
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	return 1;
}

int omo_menu_playlist_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	else if(app->player->queue && app->player->queue->entry_count > 0)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
		return 0;
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_playback_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->player->queue)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_playback_find_track_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library && app->library->loaded && app->library_view && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_playback_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library && app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_library_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library && app->library_view && app->ui->ui_song_list_element->d1 > 0 && app->ui->ui_song_list_element->flags & D_GOTFOCUS)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_library_profile_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(item == app->selected_profile_id)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_CHECKED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, 0);
	}
	return 1;
}

int omo_menu_library_profile_delete_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->selected_profile_id == app->profile_select_id[0])
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, 0);
	}
	return 1;
}

int omo_menu_library_edit_album_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library && app->library_view && app->ui->ui_album_list_element->d1 > 1 && app->ui->ui_album_list_element->flags & D_GOTFOCUS)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_ENABLED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_cloud_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	val = al_get_config_value(t3f_config, "Settings", "tagger_id");
	if(app->library && val)
	{
		t3f_set_menu_item_flags(mp, item, 0);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_DISABLED);
	}
	return 1;
}

int omo_menu_view_basic_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library_view)
	{
		t3f_set_menu_item_flags(mp, item, 0);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_CHECKED);
	}
	return 1;
}

int omo_menu_view_library_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(!omo_menu_base_update_disable_proc(mp, item, data))
	{
		return 0;
	}
	if(app->library_view)
	{
		t3f_set_menu_item_flags(mp, item, ALLEGRO_MENU_ITEM_CHECKED);
	}
	else
	{
		t3f_set_menu_item_flags(mp, item, 0);
	}
	return 1;
}
