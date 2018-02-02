#include "../t3f/t3f.h"

#include "../instance.h"
#include "menu_proc.h"
#include "../profile.h"

void omo_clear_profile_menu(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	/* remove all items except Default */
	for(i = 0; i < omo_get_profile_count(); i++)
	{
		al_remove_menu_item(app->menu[OMO_MENU_PROFILE], -4);
	}
}

void omo_update_profile_menu(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val;
	int i;

	/* add menu items for all profiles */
	for(i = 0; i < omo_get_profile_count(); i++)
	{
		val = omo_get_profile(i);
		if(val)
		{
			app->profile_select_id[i + 1] = t3f_add_menu_item(app->menu[OMO_MENU_PROFILE], val, ALLEGRO_MENU_ITEM_CHECKBOX, NULL, omo_menu_library_select_profile, omo_menu_library_profile_update_proc);
		}
	}
	i = omo_get_current_profile();
	app->selected_profile_id = app->profile_select_id[i + 1];
}

bool omo_setup_menus(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->menu[OMO_MENU_PROFILE] = al_create_menu();
	if(!app->menu[OMO_MENU_PROFILE])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_PROFILE], "Add", 0, NULL, omo_menu_library_add_profile, NULL);
	app->profile_delete_id = t3f_add_menu_item(app->menu[OMO_MENU_PROFILE], "Remove", 0, NULL, omo_menu_library_remove_profile, omo_menu_library_profile_delete_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PROFILE], NULL, 0, NULL, NULL, NULL);
	app->profile_select_id[0] = t3f_add_menu_item(app->menu[OMO_MENU_PROFILE], "Default", ALLEGRO_MENU_ITEM_CHECKBOX, NULL, omo_menu_library_select_profile, omo_menu_library_profile_update_proc);
	omo_clear_profile_menu(data);
	omo_update_profile_menu(data);

	app->menu[OMO_MENU_FILE] = al_create_menu();
	if(!app->menu[OMO_MENU_FILE])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Play Files", 0, NULL, omo_menu_file_play_files, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Queue Files", 0, NULL, omo_menu_file_queue_files, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Play Folder", 0, NULL, omo_menu_file_play_folder, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Queue Folder", 0, NULL, omo_menu_file_queue_folder, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Get Tagger Key", 0, NULL, omo_menu_file_get_tagger_key, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Load Theme", 0, NULL, omo_menu_file_load_theme, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_FILE], "Exit", 0, NULL, omo_menu_file_exit, NULL);

	app->menu[OMO_MENU_PLAYER] = al_create_menu();
	if(!app->menu[OMO_MENU_PLAYER])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Previous Track", 0, NULL, omo_menu_playback_previous_track, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Play", 0, NULL, omo_menu_playback_play, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Pause", 0, NULL, omo_menu_playback_pause, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Stop", 0, NULL, omo_menu_playback_stop, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Next Track", 0, NULL, omo_menu_playback_next_track, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Shuffle", 0, NULL, omo_menu_playback_shuffle, omo_menu_playback_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Split Song", 0, NULL, omo_menu_playback_split_track, omo_menu_playback_edit_tags_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_PLAYER], "Edit Song Tags", 0, NULL, omo_menu_playback_edit_tags, omo_menu_playback_edit_tags_update_proc);

	app->menu[OMO_MENU_LIBRARY] = al_create_menu();
	if(!app->menu[OMO_MENU_LIBRARY])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Profile", 0, app->menu[OMO_MENU_PROFILE], NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Add Library Folder", 0, NULL, omo_menu_library_add_folder, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Clear Library Folders", 0, NULL, omo_menu_library_clear_folders, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Submit Library Tags", 0, NULL, omo_menu_library_submit_tags, omo_menu_cloud_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Retrieve Library Tags", 0, NULL, omo_menu_library_retrieve_tags, omo_menu_cloud_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Split Song", 0, NULL, omo_menu_library_split_track, omo_menu_library_edit_tags_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], NULL, 0, NULL, NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_LIBRARY], "Edit Song Tags", 0, NULL, omo_menu_library_edit_tags, omo_menu_library_edit_tags_update_proc);

	app->menu[OMO_MENU_VIEW] = al_create_menu();
	if(!app->menu[OMO_MENU_VIEW])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_VIEW], "Basic", ALLEGRO_MENU_ITEM_CHECKBOX, NULL, omo_menu_view_basic, omo_menu_view_basic_update_proc);
	t3f_add_menu_item(app->menu[OMO_MENU_VIEW], "Library", ALLEGRO_MENU_ITEM_CHECKBOX, NULL, omo_menu_view_library, omo_menu_view_library_update_proc);

	app->menu[OMO_MENU_MAIN] = al_create_menu();
	if(!app->menu[OMO_MENU_MAIN])
	{
		return false;
	}
	t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "File", 0, app->menu[OMO_MENU_FILE], NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "View", 0, app->menu[OMO_MENU_VIEW], NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "Player", 0, app->menu[OMO_MENU_PLAYER], NULL, NULL);
	t3f_add_menu_item(app->menu[OMO_MENU_MAIN], "Library", 0, app->menu[OMO_MENU_LIBRARY], NULL, NULL);

	return true;
}
