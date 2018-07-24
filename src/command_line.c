#include "t3f/t3f.h"
#include "t3f/file_utils.h"
#include "instance.h"

static void disable_codec_handler(OMO_CODEC_HANDLER_REGISTRY * rp, const char * name)
{
	int i;

	for(i = 0; i < rp->codec_handlers; i++)
	{
		if(!strcmp(rp->codec_handler[i].id, name))
		{
			rp->codec_handler[i].disabled = true;
		}
	}
}

bool omo_process_command_line_arguments(APP_INSTANCE * app, int argc, char * argv[])
{
	OMO_FILE_HELPER_DATA file_helper_data;
	bool used_arg[1024] = {false};
	int i;

	app->test_mode = -1;
	if(argc > 1)
	{
		/* check for command line options */
		for(i = 1; i < argc; i++)
		{
			if(!strcmp(argv[i], "--test"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --test <test_files_path>\n\n");
					return false;
				}
				else
				{
					app->test_path_arg = i + 1;
					app->test_mode = 0;
				}
			}
			else if(!strcmp(argv[i], "--quick-test"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --quick-test <test_files_path>\n\n");
					return false;
				}
				else
				{
					app->test_path_arg = i + 1;
					app->test_mode = 1;
				}
			}
			else if(!strcmp(argv[i], "--prune-library"))
			{
				app->prune_library = true;
				used_arg[i] = true;
			}
			else if(!strcmp(argv[i], "--ignore-genre"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --ignore-genre <genre name>\n\n");
					return false;
				}
				else
				{
					if(!strcmp(argv[i + 1], "none"))
					{
						al_remove_config_key(t3f_config, "Settings", "Ignore Genre");
					}
					else
					{
						al_set_config_value(t3f_config, "Settings", "Ignore Genre", argv[i + 1]);
					}
				}
				used_arg[i] = true;
				used_arg[i + 1] = true;
			}
			else if(!strcmp(argv[i], "--disable-codec-handler"))
			{
				if(argc < i + 2)
				{
					printf("Usage: omo --disable-codec-handler <codec handler id>\n\n");
					return false;
				}
				else
				{
					disable_codec_handler(app->codec_handler_registry, argv[i + 1]);
				}
				used_arg[i] = true;
				used_arg[i + 1] = true;
			}
			else if(!strcmp(argv[i], "--set"))
			{
				if(argc < i + 4)
				{
					printf("Usage: omo --set <section> <key> <value>\n\n");
					return false;
				}
				al_set_config_value(t3f_config, argv[i + 1], argv[i + 2], argv[i + 3]);
				used_arg[i] = true;
				used_arg[i + 1] = true;
				used_arg[i + 2] = true;
				used_arg[i + 3] = true;
			}
			else if(!strcmp(argv[i], "--version"))
			{
				printf("%s v%s %s.\n\n", T3F_APP_TITLE, T3F_APP_VERSION, T3F_APP_COPYRIGHT);
				return false;
			}
		}

		/* don't add files if we are running the test suite */
		if(app->test_mode < 0)
		{
			omo_setup_file_helper_data(&file_helper_data, app->archive_handler_registry, app->codec_handler_registry, NULL, app->library, app->player->queue, app->queue_temp_path, NULL);
			for(i = 1; i < argc; i++)
			{
				if(!used_arg[i])
				{
					if(!t3f_scan_files(argv[i], omo_count_file, false, &file_helper_data))
					{
						omo_count_file(argv[i], false, &file_helper_data);
					}
				}
			}
			if(file_helper_data.file_count > 0)
			{
				app->player->queue = omo_create_queue(file_helper_data.file_count);
				if(app->player->queue)
				{
					file_helper_data.queue = app->player->queue;
					for(i = 1; i < argc; i++)
					{
						if(!used_arg[i])
						{
							if(!t3f_scan_files(argv[i], omo_queue_file, false, &file_helper_data))
							{
								omo_queue_file(argv[i], false, &file_helper_data);
							}
						}
					}
					if(app->player->queue->entry_count)
					{
						app->player->queue_pos = 0;
						omo_start_player(app->player);
					}
					app->spawn_queue_thread = true;
				}
			}
		}
	}
	return true;
}
