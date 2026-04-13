#include "t3f/t3f.h"
#include "t3f/file_utils.h"

#include "instance.h"
#include "archive_handlers/registry.h"
#include "codec_handlers/registry.h"
#include "queue.h"
#include "library.h"
#include "library_helpers.h"
#include "init.h"
#include "file_chooser.h"
#include "constants.h"
#include "queue_helpers.h"
#include "cloud.h"
#include "profile.h"
#include "menu_init.h"
#include "tags_dialog.h"
#include "multi_tags_dialog.h"
#include "album_tags_dialog.h"
#include "tagger_key_dialog.h"
#include "filter_dialog.h"
#include "new_profile_dialog.h"
#include "split_track_dialog.h"
#include "about_dialog.h"
#include "dialog_proc.h"
#include "ui.h"

static char type_buf[1024] = {0};

static const char * omo_get_type_string(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i, j;

	strcpy(type_buf, "");
	for(i = 0; i < uip->app->codec_handler_registry->codec_handlers; i++)
	{
		for(j = 0; j < uip->app->codec_handler_registry->codec_handler[i].types; j++)
		{
			strcat(type_buf, "*");
			strcat(type_buf, uip->app->codec_handler_registry->codec_handler[i].type[j]);
			strcat(type_buf, ";");
		}
	}
	for(i = 0; i < uip->app->archive_handler_registry->archive_handlers; i++)
	{
		for(j = 0; j < uip->app->archive_handler_registry->archive_handler[i].types; j++)
		{
			strcat(type_buf, "*");
			strcat(type_buf, uip->app->archive_handler_registry->archive_handler[i].type[j]);
			strcat(type_buf, ";");
		}
	}
	type_buf[strlen(type_buf) - 1] = '\0';
	return type_buf;
}

static void open_tags_dialog(void * data, const char * fullfn)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * val2;
	int i;
	const char * script_url;

	uip->tags_entry = omo_get_database_value(uip->app->library->file_database, fullfn, "id");
	if(uip->tags_entry)
	{
		if(omo_backup_entry_tags(uip->app->library, uip->tags_entry, true))
		{
			if(uip->app->prefetch_tags)
			{
				script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
				if(script_url)
				{
					omo_retrieve_track_tags(uip->app->library, uip->tags_entry, script_url);
				}
			}
		}
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			strcpy(uip->tags_text[i], "");
			if(omo_tag_type[i])
			{
				val2 = omo_get_database_value(uip->app->library->entry_database, uip->tags_entry, omo_tag_type[i]);
				if(val2)
				{
					strcpy(uip->tags_text[i], val2);
				}
			}
		}
		omo_open_tags_dialog(uip, uip->app);
	}
}

static void open_multi_tags_dialog(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * val2;
	int i, j, c = 0;
	bool first = true;
	const char * script_url;

	for(j = 0; j < uip->app->player->queue->entry_count; j++)
	{
		if(omo_queue_item_selected(uip->ui_queue_list_element, j))
		{
			uip->tags_entry = omo_get_queue_entry_id(uip->app->player->queue, j, uip->app->library);
			if(uip->tags_entry)
			{
				c++;
				if(omo_backup_entry_tags(uip->app->library, uip->tags_entry, first))
				{
					if(uip->app->prefetch_tags && first)
					{
						script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
						omo_retrieve_track_tags(uip->app->library, uip->tags_entry, script_url);
					}
				}
				first = false;
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					strcpy(uip->tags_text[i], "");
					if(omo_tag_type[i])
					{
						val2 = omo_get_database_value(uip->app->library->entry_database, uip->tags_entry, omo_tag_type[i]);
						if(val2)
						{
							strcpy(uip->tags_text[i], val2);
						}
					}
				}
			}
		}
	}
	if(c)
	{
		omo_open_multi_tags_dialog( uip, uip->app);
	}
}

static void open_album_tags_dialog(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * val2;
	int i, j;
	bool first = true;
	const char * script_url;

	for(j = 0; j < uip->app->library->filtered_song_entry_count; j++)
	{
		uip->tags_entry = uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->id;
		if(uip->tags_entry)
		{
			if(omo_backup_entry_tags(uip->app->library, uip->tags_entry, first))
			{
				if(uip->app->prefetch_tags && first)
				{
					script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
					if(script_url)
					{
						omo_retrieve_track_tags(uip->app->library, uip->tags_entry, script_url);
					}
				}
			}
			first = false;
			for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
			{
				strcpy(uip->tags_text[i], "");
				if(omo_tag_type[i])
				{
					val2 = omo_get_database_value(uip->app->library->entry_database, uip->tags_entry, omo_tag_type[i]);
					if(val2)
					{
						strcpy(uip->tags_text[i], val2);
					}
				}
			}
		}
	}
	omo_open_album_tags_dialog( uip, uip->app);
}

static void open_split_track_dialog(void * data, const char * basefn, const char * fullfn)
{
	OMO_UI * uip = (OMO_UI *)data;
	char buffer[1024];
	const char * base_id;
	const char * val2;
	const char * script_url;

	uip->split_track_entry = omo_get_database_value(uip->app->library->file_database, basefn, "id");
	if(uip->split_track_entry)
	{
		/* get the file ID of the base file */
		base_id = omo_get_library_file_base_id(uip->app->library, basefn, buffer);
		if(base_id)
		{
			if(omo_backup_entry_tags(uip->app->library, base_id, true))
			{
				script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
				if(script_url)
				{
					omo_retrieve_track_tags(uip->app->library, base_id, script_url);
				}
			}
			uip->split_track_fn = basefn;
			val2 = omo_get_database_value(uip->app->library->entry_database, base_id, "Split Track Info");
			if(val2)
			{
				strcpy(uip->split_track_text, val2);
			}
			else
			{
				strcpy(uip->split_track_text, "");
			}
			omo_open_split_track_dialog( uip, uip->app);
		}
	}
}

int omo_menu_file_play_files(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_PLAY_FILES;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
	return 1;
}

int omo_menu_file_queue_files(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_QUEUE_FILES;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
	return 1;
}

int omo_menu_file_play_folder(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_PLAY_FOLDER;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_file_queue_folder(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_QUEUE_FOLDER;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_file_save_playlist(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_EXPORT_PLAYLIST;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_playlist_filename"), "Enter playlist file name.", NULL, ALLEGRO_FILECHOOSER_SAVE, true);
	return 1;
}

int omo_menu_file_get_tagger_key(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_open_tagger_key_dialog( uip, data);
	return 1;
}

int omo_menu_file_load_theme(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_LOAD_THEME;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select theme file.", "*.ini", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST, true);

	return 1;
}

int omo_menu_file_exit(int id, void * data)
{
	t3f_exit();
	return 1;
}

int omo_menu_edit_copy_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(omo_tag_type[i])
		{
			strcpy(uip->tags_clipboard[i], uip->tags_text[i]);
		}
	}
	return 1;
}

int omo_menu_edit_paste_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(omo_tag_type[i])
		{
			strcpy(uip->tags_text[i], uip->tags_clipboard[i]);
		}
	}
	return 1;
}

int omo_menu_playback_previous_track(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_play_previous_song(uip->app->player);
	return 1;
}

int omo_menu_playback_play(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_resume_player(uip->app->player);
	return 1;
}

int omo_menu_playback_pause(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_pause_player(uip->app->player);
	return 1;
}

int omo_menu_playback_stop(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_stop_player(uip->app->player);
	return 1;
}

int omo_menu_playback_next_track(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_play_next_song(uip->app->player);
	return 1;
}

int omo_menu_playback_shuffle(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	OMO_QUEUE * new_queue;
	int i, r;
	int old_state;

	if(uip->app->player->queue)
	{
		/* stop currently playing song */
		old_state = uip->app->player->state;
		omo_stop_player(uip->app->player);

		/* create new queue */
		new_queue = omo_create_queue(uip->app->player->queue->entry_count);
		if(new_queue)
		{
			al_destroy_thread(uip->app->player->queue->thread);
			uip->app->player->queue->thread = NULL;
			for(i = 0; i < new_queue->entry_size; i++)
			{
				r = t3f_rand(&uip->app->rng_state) % uip->app->player->queue->entry_count;
				omo_add_file_to_queue(new_queue, uip->app->player->queue->entry[r]->file, uip->app->player->queue->entry[r]->sub_file, uip->app->player->queue->entry[r]->track, uip->app->player->queue->entry[r]->skip_scan);
				omo_delete_queue_item(uip->app->player->queue, r);
			}
			omo_destroy_queue(uip->app->player->queue);
			uip->app->player->queue = new_queue;
			uip->app->player->queue_pos = 0;
			if(old_state == OMO_PLAYER_STATE_PLAYING)
			{
				omo_start_player(uip->app->player);
			}
			uip->app->spawn_queue_thread = true;
		}
	}
	return 1;
}

static int get_tag_slot(const char * tag_name)
{
	int i;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(!strcmp(tag_name, omo_tag_type[i]))
		{
			return i;
		}
	}
	return -1;
}

static bool tag_matches(OMO_QUEUE * qp, int e1, int e2, int tag_slot, OMO_LIBRARY * lp)
{
	const char * id1;
	const char * id2;
	const char * val1;
	const char * val2;

	id1 = omo_get_queue_entry_id(qp, e1, lp);
	if(id1)
	{
		id2 = omo_get_queue_entry_id(qp, e2, lp);
		if(id2)
		{
			val1 = omo_get_database_value(lp->entry_database, id1, omo_tag_type[tag_slot]);
			val2 = omo_get_database_value(lp->entry_database, id2, omo_tag_type[tag_slot]);
			if(!val1 && !val2)
			{
				return true;
			}
			else if(val1 && val2 && !strcmp(val1, val2))
			{
				return true;
			}
		}
	}
	return false;
}

static void conditionally_enable_tag(OMO_UI * uip, const char * tag_name, OMO_LIBRARY * lp)
{
	int i;
	int first = -1;
	int tag_slot;

	tag_slot = get_tag_slot(tag_name);
	if(tag_slot >= 0)
	{
		for(i = 0; i < uip->app->player->queue->entry_count; i++)
		{
			if(omo_queue_item_selected(uip->ui_queue_list_element, i))
			{
				if(first < 0)
				{
					first = i;
				}
				if(tag_matches(uip->app->player->queue, i, first, tag_slot, lp))
				{
					uip->tag_enabled[tag_slot] = true;
				}
			}
		}
	}
}

static void enable_tags(OMO_UI * uip, bool multi, OMO_LIBRARY * lp)
{
	int i;

	if(multi)
	{
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			uip->tag_enabled[i] = false;
		}
		conditionally_enable_tag(uip, "Album Artist", lp);
		conditionally_enable_tag(uip, "Artist", lp);
		conditionally_enable_tag(uip, "Album", lp);
		conditionally_enable_tag(uip, "Disambiguation", lp);
		conditionally_enable_tag(uip, "Genre", lp);
		conditionally_enable_tag(uip, "Year", lp);
		conditionally_enable_tag(uip, "Copyright", lp);
	}
	else
	{
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			uip->tag_enabled[i] = true;
		}
	}
}

int omo_menu_playback_edit_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char fullfn[1024];
	int j;

	uip->tags_queue_entry = -1;
	if(uip->app->player->queue && uip->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		if(omo_queue_items_selected(uip->ui_queue_list_element, uip->app->player->queue->entry_count) > 1)
		{
			enable_tags(uip, true, uip->app->library);
			open_multi_tags_dialog(uip);
		}
		else
		{
			j = uip->ui_queue_list_element->d1;
			if(omo_get_full_filename(uip->app->player->queue->entry[j]->file, uip->app->player->queue->entry[j]->sub_file, uip->app->player->queue->entry[j]->track, fullfn, 1024))
			{
				enable_tags(uip, false, uip->app->library);
				uip->tags_queue_entry = uip->ui_queue_list_element->d1;
				open_tags_dialog(uip, fullfn);
			}
		}
	}
	return 1;
}

int omo_menu_playback_split_track(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char fullfn[1024];
	int j;

	uip->split_track_queue_entry = -1;
	if(uip->app->player->queue && uip->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		j = uip->ui_queue_list_element->d1;
		if(omo_get_full_filename(uip->app->player->queue->entry[j]->file, uip->app->player->queue->entry[j]->sub_file, uip->app->player->queue->entry[j]->track, fullfn, 1024))
		{
			uip->split_track_queue_entry = uip->ui_queue_list_element->d1;
			open_split_track_dialog(uip, uip->app->player->queue->entry[j]->file, fullfn);
		}
	}
	return 1;
}

int omo_menu_playback_find_track(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char fullfn[1024];

	if(omo_get_full_filename(uip->app->player->queue->entry[uip->ui_queue_list_element->d1]->file, uip->app->player->queue->entry[uip->ui_queue_list_element->d1]->sub_file, uip->app->player->queue->entry[uip->ui_queue_list_element->d1]->track, fullfn, 1024))
	{
		omo_find_track(uip->app, omo_get_database_value(uip->app->library->file_database, fullfn, "id"));
	}
	return 1;
}

int omo_menu_library_select_profile(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i;

	if(id != uip->selected_profile_id)
	{
		for(i = 0; i < OMO_MAX_PROFILES; i++)
		{
			if(uip->profile_select_id[i] == id)
			{
				omo_set_current_profile(i - 1);
				uip->app->spawn_library_thread = true;
				omo_configure_codec_handlers(uip->app);
				break;
			}
		}
		uip->selected_profile_id = id;
	}
	return 1;
}

int omo_menu_library_add_profile(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_open_new_profile_dialog( uip, data);

	return 1;
}

int omo_menu_library_remove_profile(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char buffer[1024];
	char fn[256];
	const char * name;
	int i;

	i = omo_get_current_profile();
	if(i >= 0)
	{
		omo_cancel_library_setup(uip->app);
		name = omo_get_profile(i);
		if(name)
		{
			sprintf(fn, "Profiles/%s", name);
			t3f_remove_directory(t3f_get_filename(t3f_data_path, fn, buffer, 1024));
		}
		omo_get_profile_section(t3f_config, name, buffer);
		if(strcmp(buffer, "Profile Default"))
		{
			al_remove_config_section(t3f_config, buffer);
		}
		omo_clear_profile_menu(data);
		if(omo_get_current_profile() >= omo_get_profile_count())
		{
			omo_set_current_profile(omo_get_profile_count() - 1);
		}
		omo_delete_profile(omo_get_current_profile());
		omo_set_current_profile(omo_get_current_profile());
	}
	omo_update_profile_menu(data);
	uip->app->spawn_library_thread = true;

	return 1;
}

int omo_menu_library_add_folder(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_ADD_LIBRARY_FOLDER;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select library folder.", al_get_config_value(t3f_config, "Settings", "last_music_folder"), ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_library_clear_folders(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char section_buffer[1024];
	char buf[4];

	sprintf(buf, "%d", 0);
	al_set_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), "library_folders", buf);
	omo_clear_library_cache();
	uip->app->spawn_library_thread = true;
	sprintf(uip->app->status_bar_text, "No Library Folders");
	return 1;
}

int omo_menu_library_rescan_folders(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int i;

	for(i = 0; i < uip->app->library->entry_count; i++)
	{
		omo_remove_database_key(uip->app->library->entry_database, uip->app->library->entry[i]->id, "scanned");
	}
	omo_save_library(uip->app->library);
	omo_clear_library_cache();
	uip->app->spawn_library_thread = true;
	return 1;
}

int omo_menu_library_import_file_database(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_IMPORT_FILE_DATABASE;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select database file.", "*.ini", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST, true);
	return 1;
}

int omo_menu_library_import_entry_database(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_IMPORT_ENTRY_DATABASE;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select database file.", "*.ini", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST, true);
	return 1;
}

int omo_menu_library_rebase_song_folder(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->file_chooser_mode = OMO_FILE_CHOOSER_REBASE_SONG_FOLDER;
	uip->app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select song folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_library_edit_filter(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_open_filter_dialog( uip, data);

	return 1;
}

int omo_menu_library_edit_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char fullfn[1024];
	int j;

	if(uip->app->library)
	{
		if(uip->ui_song_list_element->flags & D_GOTFOCUS && uip->ui_song_list_element->d1 > 0)
		{
			j = uip->ui_song_list_element->d1 - 1;
			if(j >= 0)
			{
				strcpy(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->filename);
				if(uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->sub_filename)
				{
					strcat(fullfn, "/");
					strcat(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->sub_filename);
				}
				if(uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->track)
				{
					strcat(fullfn, ":");
					strcat(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->track);
				}
			}
			open_tags_dialog(uip, fullfn);
		}
	}
	return 1;
}

int omo_menu_library_split_track(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	char fullfn[1024];
	int j;

	if(uip->app->library)
	{
		if(uip->ui_song_list_element->flags & D_GOTFOCUS)
		{
			j = uip->ui_song_list_element->d1 - 1;
			if(j >= 0)
			{
				strcpy(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->filename);
				if(uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->sub_filename)
				{
					strcat(fullfn, "/");
					strcat(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->sub_filename);
				}
				if(uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->track)
				{
					strcat(fullfn, ":");
					strcat(fullfn, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->track);
				}
			}
			open_split_track_dialog(uip, uip->app->library->entry[uip->app->library->filtered_song_entry[j]]->filename, fullfn);
		}
	}
	return 1;
}

int omo_menu_library_submit_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	uip->app->spawn_cloud_thread = true;

	return 1;
}

int omo_menu_library_retrieve_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * script_url;

	script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
	if(script_url)
	{
		omo_retrieve_library_tags(uip->app, script_url);
	}

	return 1;
}

int omo_menu_library_edit_album_tags(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	if(uip->app->library)
	{
		if(uip->ui_album_list_element->flags & D_GOTFOCUS && uip->ui_album_list_element->d1 > 1)
		{
			enable_tags(uip, true, uip->app->library);
			open_album_tags_dialog(uip);
		}
	}
	return 1;
}

static char * get_old_selection(void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	int nelem;
	bool multi;
	char * ret;

	ui_queue_list_proc(-1, &nelem, &multi, data);
	ret = malloc(sizeof(char) * nelem);
	if(ret)
	{
		memcpy(ret, uip->ui_queue_list_element->dp2, sizeof(char) * nelem);
	}
	return ret;
}

static void put_old_selection(void * data, char * in)
{
	OMO_UI * uip = (OMO_UI *)data;
	int nelem;
	bool multi;

	ui_queue_list_proc(-1, &nelem, &multi, data);
	memcpy(uip->ui_queue_list_element->dp2, in, sizeof(char) * nelem);
	free(in);
}

int omo_menu_view_basic(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * v_x;
	const char * v_y;
	const char * v_width;
	const char * v_height;
	int c_x, c_y, c_width, c_height, c_old_width;
	char buf[32] = {0};
	ALLEGRO_MONITOR_INFO monitor_info;
	char * old_selection;
	int old_index;

	old_selection = get_old_selection(data);
	old_index = uip->ui_queue_list_element->d2;
	if(uip->app->library_view)
	{
		t3gui_close_dialog(uip->ui_dialog);

		al_get_window_position(t3f_display, &c_x, &c_y);
		c_width = al_get_display_width(t3f_display);
		c_height = al_get_display_height(t3f_display);

		v_x = al_get_config_value(t3f_config, "Settings", "basic_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "basic_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "basic_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "basic_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "library_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "library_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "library_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "library_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "basic");

		if(!t3f_key_pressed(ALLEGRO_KEY_LCTRL) && !t3f_key_pressed(ALLEGRO_KEY_RCTRL) && !t3f_key_pressed(ALLEGRO_KEY_COMMAND) && v_x && v_y && v_width && v_height)
		{
			c_x = atoi(v_x);
			c_y = atoi(v_y);
			c_width = atoi(v_width);
			c_height = atoi(v_height);
		}
		else
		{
			c_old_width = c_width;
			c_width /= 4;
			c_x += c_old_width - c_width;
			if(c_x + c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_x = monitor_info.x2 - c_width;
			}
		}
		uip->app->library_view = false;
		al_resize_display(t3f_display, c_width, c_height);
		al_set_window_position(t3f_display, c_x, c_y);
		omo_create_main_dialog( uip, 0, c_width, c_height, uip->app);
		omo_set_window_constraints(uip, uip->app->library_view);
		t3gui_show_dialog(uip->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, uip->app);
	}
	if(old_selection)
	{
		put_old_selection(data, old_selection);
		uip->ui_queue_list_element->d2 = old_index;
		t3gui_set_focus_element(uip->ui_queue_list_element);
	}
	return 1;
}

int omo_menu_view_library(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;
	const char * v_x;
	const char * v_y;
	const char * v_width;
	const char * v_height;
	int c_x, c_y, c_width, c_height, c_old_width;
	char buf[32] = {0};
	ALLEGRO_MONITOR_INFO monitor_info;
	char * old_selection;
	int old_index;

	old_selection = get_old_selection(data);
	old_index = uip->ui_queue_list_element->d2;
	if(!uip->app->library_view)
	{
		t3gui_close_dialog(uip->ui_dialog);

		al_get_window_position(t3f_display, &c_x, &c_y);
		c_width = al_get_display_width(t3f_display);
		c_height = al_get_display_height(t3f_display);

		v_x = al_get_config_value(t3f_config, "Settings", "library_view_x");
		v_y = al_get_config_value(t3f_config, "Settings", "library_view_y");
		v_width = al_get_config_value(t3f_config, "Settings", "library_view_width");
		v_height = al_get_config_value(t3f_config, "Settings", "library_view_height");
		sprintf(buf, "%d", c_x);
		al_set_config_value(t3f_config, "Settings", "basic_view_x", buf);
		sprintf(buf, "%d", c_y);
		al_set_config_value(t3f_config, "Settings", "basic_view_y", buf);
		sprintf(buf, "%d", c_width);
		al_set_config_value(t3f_config, "Settings", "basic_view_width", buf);
		sprintf(buf, "%d", c_height);
		al_set_config_value(t3f_config, "Settings", "basic_view_height", buf);
		al_set_config_value(t3f_config, "Settings", "last_view", "library");

		if(!t3f_key_pressed(ALLEGRO_KEY_LCTRL) && !t3f_key_pressed(ALLEGRO_KEY_RCTRL) && !t3f_key_pressed(ALLEGRO_KEY_COMMAND) && v_x && v_y && v_width && v_height)
		{
			c_x = atoi(v_x);
			c_y = atoi(v_y);
			c_width = atoi(v_width);
			c_height = atoi(v_height);
		}
		else
		{
			al_get_monitor_info(0, &monitor_info);
			c_old_width = c_width;
			c_width *= 4;
			if(c_width > monitor_info.x2 - monitor_info.x1)
			{
				c_width = monitor_info.x2 - monitor_info.x1;
			}
			c_x -= c_width - c_old_width;
			if(c_x < 0)
			{
				c_x = 0;
			}
		}
		uip->app->library_view = true;
		al_resize_display(t3f_display, c_width, c_height);
		al_set_window_position(t3f_display, c_x, c_y);
		omo_create_main_dialog( uip, 1, c_width, c_height, uip->app);
		omo_set_window_constraints(uip, uip->app->library_view);
		t3gui_show_dialog(uip->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, uip->app);
	}
	if(old_selection)
	{
		put_old_selection(data, old_selection);
		uip->ui_queue_list_element->d2 = old_index;
		t3gui_set_focus_element(uip->ui_queue_list_element);
	}

	return 1;
}

int omo_menu_help_about(int id, void * data)
{
	OMO_UI * uip = (OMO_UI *)data;

	omo_open_about_dialog( uip, data);

	return 1;
}
