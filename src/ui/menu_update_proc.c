#include "../t3f/t3f.h"
#include "../instance.h"

int omo_menu_playback_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

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

int omo_menu_playback_edit_tags_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

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

	if(app->library && app->ui->ui_song_list_element->flags & D_GOTFOCUS)
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

int omo_menu_cloud_update_proc(ALLEGRO_MENU * mp, int item, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;

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
