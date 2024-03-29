#include "../t3f/t3f.h"
#include "../t3f/file.h"
#include "../t3f/file_utils.h"

#include "../instance.h"
#include "../archive_handlers/registry.h"
#include "../codec_handlers/registry.h"
#include "../queue.h"
#include "../library.h"
#include "../library_helpers.h"
#include "../init.h"
#include "../file_chooser.h"
#include "../constants.h"
#include "../queue_helpers.h"
#include "../cloud.h"
#include "../profile.h"
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

static char type_buf[1024] = {0};

static const char * omo_get_type_string(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i, j;

	strcpy(type_buf, "");
	for(i = 0; i < app->codec_handler_registry->codec_handlers; i++)
	{
		for(j = 0; j < app->codec_handler_registry->codec_handler[i].types; j++)
		{
			strcat(type_buf, "*");
			strcat(type_buf, app->codec_handler_registry->codec_handler[i].type[j]);
			strcat(type_buf, ";");
		}
	}
	for(i = 0; i < app->archive_handler_registry->archive_handlers; i++)
	{
		for(j = 0; j < app->archive_handler_registry->archive_handler[i].types; j++)
		{
			strcat(type_buf, "*");
			strcat(type_buf, app->archive_handler_registry->archive_handler[i].type[j]);
			strcat(type_buf, ";");
		}
	}
	type_buf[strlen(type_buf) - 1] = '\0';
	return type_buf;
}

static void open_tags_dialog(void * data, const char * fullfn)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val2;
	int i;
	const char * script_url;

	app->ui->tags_entry = omo_get_database_value(app->library->file_database, fullfn, "id");
	if(app->ui->tags_entry)
	{
		if(omo_backup_entry_tags(app->library, app->ui->tags_entry, true))
		{
			if(app->prefetch_tags)
			{
				script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
				if(script_url)
				{
					omo_retrieve_track_tags(app->library, app->ui->tags_entry, script_url);
				}
			}
		}
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			strcpy(app->ui->tags_text[i], "");
			if(omo_tag_type[i])
			{
				val2 = omo_get_database_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
				if(val2)
				{
					strcpy(app->ui->tags_text[i], val2);
				}
			}
		}
		omo_open_tags_dialog(app->ui, app);
	}
}

static void open_multi_tags_dialog(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val2;
	int i, j, c = 0;
	bool first = true;
	const char * script_url;

	for(j = 0; j < app->player->queue->entry_count; j++)
	{
		if(omo_queue_item_selected(app->ui->ui_queue_list_element, j))
		{
			app->ui->tags_entry = omo_get_queue_entry_id(app->player->queue, j, app->library);
			if(app->ui->tags_entry)
			{
				c++;
				if(omo_backup_entry_tags(app->library, app->ui->tags_entry, first))
				{
					if(app->prefetch_tags && first)
					{
						script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
						omo_retrieve_track_tags(app->library, app->ui->tags_entry, script_url);
					}
				}
				first = false;
				for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
				{
					strcpy(app->ui->tags_text[i], "");
					if(omo_tag_type[i])
					{
						val2 = omo_get_database_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
						if(val2)
						{
							strcpy(app->ui->tags_text[i], val2);
						}
					}
				}
			}
		}
	}
	if(c)
	{
		omo_open_multi_tags_dialog(app->ui, app);
	}
}

static void open_album_tags_dialog(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * val2;
	int i, j;
	bool first = true;
	const char * script_url;

	for(j = 0; j < app->library->filtered_song_entry_count; j++)
	{
		app->ui->tags_entry = app->library->entry[app->library->filtered_song_entry[j]]->id;
		if(app->ui->tags_entry)
		{
			if(omo_backup_entry_tags(app->library, app->ui->tags_entry, first))
			{
				if(app->prefetch_tags && first)
				{
					script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
					if(script_url)
					{
						omo_retrieve_track_tags(app->library, app->ui->tags_entry, script_url);
					}
				}
			}
			first = false;
			for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
			{
				strcpy(app->ui->tags_text[i], "");
				if(omo_tag_type[i])
				{
					val2 = omo_get_database_value(app->library->entry_database, app->ui->tags_entry, omo_tag_type[i]);
					if(val2)
					{
						strcpy(app->ui->tags_text[i], val2);
					}
				}
			}
		}
	}
	omo_open_album_tags_dialog(app->ui, app);
}

static void open_split_track_dialog(void * data, const char * basefn, const char * fullfn)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buffer[1024];
	const char * base_id;
	const char * val2;
	const char * script_url;

	app->ui->split_track_entry = omo_get_database_value(app->library->file_database, basefn, "id");
	if(app->ui->split_track_entry)
	{
		/* get the file ID of the base file */
		base_id = omo_get_library_file_base_id(app->library, basefn, buffer);
		if(base_id)
		{
			if(omo_backup_entry_tags(app->library, base_id, true))
			{
				script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
				if(script_url)
				{
					omo_retrieve_track_tags(app->library, base_id, script_url);
				}
			}
			app->ui->split_track_fn = basefn;
			val2 = omo_get_database_value(app->library->entry_database, base_id, "Split Track Info");
			if(val2)
			{
				strcpy(app->ui->split_track_text, val2);
			}
			else
			{
				strcpy(app->ui->split_track_text, "");
			}
			omo_open_split_track_dialog(app->ui, app);
		}
	}
}

int omo_menu_file_play_files(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_PLAY_FILES;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
	return 1;
}

int omo_menu_file_queue_files(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_QUEUE_FILES;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music files.", omo_get_type_string(data), ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_MULTIPLE, true);
	return 1;
}

int omo_menu_file_play_folder(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_PLAY_FOLDER;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_file_queue_folder(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_QUEUE_FOLDER;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select music folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_file_save_playlist(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_EXPORT_PLAYLIST;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_playlist_filename"), "Enter playlist file name.", NULL, ALLEGRO_FILECHOOSER_SAVE, true);
	return 1;
}

int omo_menu_file_get_tagger_key(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_open_tagger_key_dialog(app->ui, data);
	return 1;
}

int omo_menu_file_load_theme(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_LOAD_THEME;
	app->file_chooser_done = false;
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
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(omo_tag_type[i])
		{
			strcpy(app->tags_clipboard[i], app->ui->tags_text[i]);
		}
	}
	return 1;
}

int omo_menu_edit_paste_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
	{
		if(omo_tag_type[i])
		{
			strcpy(app->ui->tags_text[i], app->tags_clipboard[i]);
		}
	}
	return 1;
}

int omo_menu_playback_previous_track(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_play_previous_song(app->player);
	return 1;
}

int omo_menu_playback_play(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_resume_player(app->player);
	return 1;
}

int omo_menu_playback_pause(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_pause_player(app->player);
	return 1;
}

int omo_menu_playback_stop(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_stop_player(app->player);
	return 1;
}

int omo_menu_playback_next_track(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_play_next_song(app->player);
	return 1;
}

int omo_menu_playback_shuffle(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	OMO_QUEUE * new_queue;
	int i, r;
	int old_state;

	if(app->player->queue)
	{
		/* stop currently playing song */
		old_state = app->player->state;
		omo_stop_player(app->player);

		/* create new queue */
		new_queue = omo_create_queue(app->player->queue->entry_count);
		if(new_queue)
		{
			al_destroy_thread(app->player->queue->thread);
			app->player->queue->thread = NULL;
			for(i = 0; i < new_queue->entry_size; i++)
			{
				r = t3f_rand(&app->rng_state) % app->player->queue->entry_count;
				omo_add_file_to_queue(new_queue, app->player->queue->entry[r]->file, app->player->queue->entry[r]->sub_file, app->player->queue->entry[r]->track, app->player->queue->entry[r]->skip_scan);
				omo_delete_queue_item(app->player->queue, r);
			}
			omo_destroy_queue(app->player->queue);
			app->player->queue = new_queue;
			app->player->queue_pos = 0;
			if(old_state == OMO_PLAYER_STATE_PLAYING)
			{
				omo_start_player(app->player);
			}
			app->spawn_queue_thread = true;
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

static void conditionally_enable_tag(APP_INSTANCE * app, const char * tag_name, OMO_LIBRARY * lp)
{
	int i;
	int first = -1;
	int tag_slot;

	tag_slot = get_tag_slot(tag_name);
	if(tag_slot >= 0)
	{
		for(i = 0; i < app->player->queue->entry_count; i++)
		{
			if(omo_queue_item_selected(app->ui->ui_queue_list_element, i))
			{
				if(first < 0)
				{
					first = i;
				}
				if(tag_matches(app->player->queue, i, first, tag_slot, lp))
				{
					app->ui->tag_enabled[tag_slot] = true;
				}
			}
		}
	}
}

static void enable_tags(APP_INSTANCE * app, bool multi, OMO_LIBRARY * lp)
{
	int i;

	if(multi)
	{
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			app->ui->tag_enabled[i] = false;
		}
		conditionally_enable_tag(app, "Album Artist", lp);
		conditionally_enable_tag(app, "Artist", lp);
		conditionally_enable_tag(app, "Album", lp);
		conditionally_enable_tag(app, "Disambiguation", lp);
		conditionally_enable_tag(app, "Genre", lp);
		conditionally_enable_tag(app, "Year", lp);
		conditionally_enable_tag(app, "Copyright", lp);
	}
	else
	{
		for(i = 0; i < OMO_MAX_TAG_TYPES; i++)
		{
			app->ui->tag_enabled[i] = true;
		}
	}
}

int omo_menu_playback_edit_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	int j;

	app->ui->tags_queue_entry = -1;
	if(app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		if(omo_queue_items_selected(app->ui->ui_queue_list_element, app->player->queue->entry_count) > 1)
		{
			enable_tags(app, true, app->library);
			open_multi_tags_dialog(app);
		}
		else
		{
			j = app->ui->ui_queue_list_element->d1;
			if(omo_get_full_filename(app->player->queue->entry[j]->file, app->player->queue->entry[j]->sub_file, app->player->queue->entry[j]->track, fullfn, 1024))
			{
				enable_tags(app, false, app->library);
				app->ui->tags_queue_entry = app->ui->ui_queue_list_element->d1;
				open_tags_dialog(app, fullfn);
			}
		}
	}
	return 1;
}

int omo_menu_playback_split_track(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	int j;

	app->ui->split_track_queue_entry = -1;
	if(app->player->queue && app->ui->ui_queue_list_element->flags & D_GOTFOCUS)
	{
		j = app->ui->ui_queue_list_element->d1;
		if(omo_get_full_filename(app->player->queue->entry[j]->file, app->player->queue->entry[j]->sub_file, app->player->queue->entry[j]->track, fullfn, 1024))
		{
			app->ui->split_track_queue_entry = app->ui->ui_queue_list_element->d1;
			open_split_track_dialog(app, app->player->queue->entry[j]->file, fullfn);
		}
	}
	return 1;
}

int omo_menu_playback_find_track(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];

	if(omo_get_full_filename(app->player->queue->entry[app->ui->ui_queue_list_element->d1]->file, app->player->queue->entry[app->ui->ui_queue_list_element->d1]->sub_file, app->player->queue->entry[app->ui->ui_queue_list_element->d1]->track, fullfn, 1024))
	{
		omo_find_track(app, omo_get_database_value(app->library->file_database, fullfn, "id"));
	}
	return 1;
}

int omo_menu_library_select_profile(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	if(id != app->selected_profile_id)
	{
		for(i = 0; i < OMO_MAX_PROFILES; i++)
		{
			if(app->profile_select_id[i] == id)
			{
				omo_set_current_profile(i - 1);
				app->spawn_library_thread = true;
				omo_configure_codec_handlers(app);
				break;
			}
		}
		app->selected_profile_id = id;
	}
	return 1;
}

int omo_menu_library_add_profile(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_open_new_profile_dialog(app->ui, data);

	return 1;
}

int omo_menu_library_remove_profile(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char buffer[1024];
	char fn[256];
	const char * name;
	int i;

	i = omo_get_current_profile();
	if(i >= 0)
	{
		omo_cancel_library_setup(app);
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
	app->spawn_library_thread = true;

	return 1;
}

int omo_menu_library_add_folder(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_ADD_LIBRARY_FOLDER;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, al_get_config_value(t3f_config, "Settings", "last_music_filename"), "Select library folder.", al_get_config_value(t3f_config, "Settings", "last_music_folder"), ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_library_clear_folders(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char section_buffer[1024];
	char buf[4];

	sprintf(buf, "%d", 0);
	al_set_config_value(t3f_config, omo_get_profile_section(t3f_config, omo_get_profile(omo_get_current_profile()), section_buffer), "library_folders", buf);
	omo_clear_library_cache();
	app->spawn_library_thread = true;
	sprintf(app->status_bar_text, "No Library Folders");
	return 1;
}

int omo_menu_library_rescan_folders(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	for(i = 0; i < app->library->entry_count; i++)
	{
		omo_remove_database_key(app->library->entry_database, app->library->entry[i]->id, "scanned");
	}
	omo_save_library(app->library);
	omo_clear_library_cache();
	app->spawn_library_thread = true;
	return 1;
}

int omo_menu_library_import_file_database(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_IMPORT_FILE_DATABASE;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select database file.", "*.ini", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST, true);
	return 1;
}

int omo_menu_library_import_entry_database(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_IMPORT_ENTRY_DATABASE;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select database file.", "*.ini", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST, true);
	return 1;
}

int omo_menu_library_rebase_song_folder(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->file_chooser_mode = OMO_FILE_CHOOSER_REBASE_SONG_FOLDER;
	app->file_chooser_done = false;
	omo_start_file_chooser(data, NULL, "Select song folder.", NULL, ALLEGRO_FILECHOOSER_FOLDER, true);
	return 1;
}

int omo_menu_library_edit_filter(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_open_filter_dialog(app->ui, data);

	return 1;
}

int omo_menu_library_edit_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	int j;

	if(app->library)
	{
		if(app->ui->ui_song_list_element->flags & D_GOTFOCUS && app->ui->ui_song_list_element->d1 > 0)
		{
			j = app->ui->ui_song_list_element->d1 - 1;
			if(j >= 0)
			{
				strcpy(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->filename);
				if(app->library->entry[app->library->filtered_song_entry[j]]->sub_filename)
				{
					strcat(fullfn, "/");
					strcat(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->sub_filename);
				}
				if(app->library->entry[app->library->filtered_song_entry[j]]->track)
				{
					strcat(fullfn, ":");
					strcat(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->track);
				}
			}
			open_tags_dialog(app, fullfn);
		}
	}
	return 1;
}

int omo_menu_library_split_track(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	char fullfn[1024];
	int j;

	if(app->library)
	{
		if(app->ui->ui_song_list_element->flags & D_GOTFOCUS)
		{
			j = app->ui->ui_song_list_element->d1 - 1;
			if(j >= 0)
			{
				strcpy(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->filename);
				if(app->library->entry[app->library->filtered_song_entry[j]]->sub_filename)
				{
					strcat(fullfn, "/");
					strcat(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->sub_filename);
				}
				if(app->library->entry[app->library->filtered_song_entry[j]]->track)
				{
					strcat(fullfn, ":");
					strcat(fullfn, app->library->entry[app->library->filtered_song_entry[j]]->track);
				}
			}
			open_split_track_dialog(app, app->library->entry[app->library->filtered_song_entry[j]]->filename, fullfn);
		}
	}
	return 1;
}

int omo_menu_library_submit_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->spawn_cloud_thread = true;

	return 1;
}

int omo_menu_library_retrieve_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	const char * script_url;

	script_url = al_get_config_value(t3f_config, "Settings", "get_track_tags_url");
	if(script_url)
	{
		omo_retrieve_library_tags(app, script_url);
	}

	return 1;
}

int omo_menu_library_edit_album_tags(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	if(app->library)
	{
		if(app->ui->ui_album_list_element->flags & D_GOTFOCUS && app->ui->ui_album_list_element->d1 > 1)
		{
			enable_tags(app, true, app->library);
			open_album_tags_dialog(app);
		}
	}
	return 1;
}

static char * get_old_selection(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int nelem;
	bool multi;
	char * ret;

	ui_queue_list_proc(-1, &nelem, &multi, data);
	ret = malloc(sizeof(char) * nelem);
	if(ret)
	{
		memcpy(ret, app->ui->ui_queue_list_element->dp2, sizeof(char) * nelem);
	}
	return ret;
}

static void put_old_selection(void * data, char * in)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int nelem;
	bool multi;

	ui_queue_list_proc(-1, &nelem, &multi, data);
	memcpy(app->ui->ui_queue_list_element->dp2, in, sizeof(char) * nelem);
	free(in);
}

int omo_menu_view_basic(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
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
	old_index = app->ui->ui_queue_list_element->d2;
	if(app->library_view)
	{
		t3gui_close_dialog(app->ui->ui_dialog);

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

		if(!t3f_key[ALLEGRO_KEY_LCTRL] && !t3f_key[ALLEGRO_KEY_RCTRL] && !t3f_key[ALLEGRO_KEY_COMMAND] && v_x && v_y && v_width && v_height)
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
		app->library_view = false;
		al_resize_display(t3f_display, c_width, c_height);
		al_set_window_position(t3f_display, c_x, c_y);
		omo_create_main_dialog(app->ui, 0, c_width, c_height, app);
		omo_set_window_constraints(app);
		t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
	}
	if(old_selection)
	{
		put_old_selection(data, old_selection);
		app->ui->ui_queue_list_element->d2 = old_index;
		t3gui_set_focus_element(app->ui->ui_queue_list_element);
	}
	return 1;
}

int omo_menu_view_library(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
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
	old_index = app->ui->ui_queue_list_element->d2;
	if(!app->library_view)
	{
		t3gui_close_dialog(app->ui->ui_dialog);

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

		if(!t3f_key[ALLEGRO_KEY_LCTRL] && !t3f_key[ALLEGRO_KEY_RCTRL] && !t3f_key[ALLEGRO_KEY_COMMAND] && v_x && v_y && v_width && v_height)
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
		app->library_view = true;
		al_resize_display(t3f_display, c_width, c_height);
		al_set_window_position(t3f_display, c_x, c_y);
		omo_create_main_dialog(app->ui, 1, c_width, c_height, app);
		omo_set_window_constraints(app);
		t3gui_show_dialog(app->ui->ui_dialog, t3f_queue, T3GUI_PLAYER_CLEAR | T3GUI_PLAYER_NO_ESCAPE, app);
	}
	if(old_selection)
	{
		put_old_selection(data, old_selection);
		app->ui->ui_queue_list_element->d2 = old_index;
		t3gui_set_focus_element(app->ui->ui_queue_list_element);
	}

	return 1;
}

int omo_menu_help_about(int id, void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	omo_open_about_dialog(app->ui, data);

	return 1;
}
