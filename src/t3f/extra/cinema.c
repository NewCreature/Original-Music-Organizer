#include "t3f/t3f.h"
#include "t3f/animation.h"
#include "t3f/music.h"
#include "cinema.h"

void t3f_process_cinema_event(T3F_CINEMA * cp, int event);

T3F_CINEMA * t3f_create_cinema(void)
{
	T3F_CINEMA * cp = NULL;

	cp = al_malloc(sizeof(T3F_CINEMA));
	if(cp)
	{
		memset(cp, 0, sizeof(T3F_CINEMA));
		cp->view = t3f_create_view(t3f_default_view, 0, 0, t3f_default_view->virtual_width, t3f_default_view->virtual_height, t3f_default_view->virtual_width / 2, t3f_default_view->virtual_height / 2, t3f_flags);
	}
	return cp;
}

void t3f_destroy_cinema(T3F_CINEMA * cp)
{
	int i;

	if(cp)
	{
		for(i = 0; i < T3F_CINEMA_MAX_BITMAPS; i++)
		{
			if(cp->bitmap[i])
			{
				t3f_destroy_animation(cp->bitmap[i]);
			}
		}
		for(i = 0; i < cp->events; i++)
		{
			al_free(cp->event[i]);
		}
		t3f_destroy_view(cp->view);
		if(cp->path)
		{
			al_destroy_path(cp->path);
		}
		if(cp->script)
		{
			if(cp->flags & T3F_CINEMA_FLAG_DEVELOPER_MODE)
			{
				al_save_config_file(cp->script_path, cp->script);
			}
			al_destroy_config(cp->script);
		}
		al_free(cp);
	}
}

bool t3f_cinema_add_bitmap(T3F_CINEMA * cp, ALLEGRO_BITMAP * bp)
{
	return false;
}

void t3f_cinema_remove_bitmap(T3F_CINEMA * cp, int bitmap)
{
}

bool t3f_cinema_add_event(T3F_CINEMA * cp, unsigned long id, unsigned long tick, int type, T3F_CINEMA_EVENT_DATA data, int flags)
{
	cp->event[cp->events] = al_malloc(sizeof(T3F_CINEMA_EVENT));
	if(cp->event[cp->events])
	{
		cp->event[cp->events]->id = id;
		cp->event[cp->events]->tick = tick;
		cp->event[cp->events]->type = type;
		cp->event[cp->events]->flags = flags;
		cp->event[cp->events]->data = data;
		cp->events++;
		return true;
	}
	return false;
}

void t3f_cinema_remove_event(T3F_CINEMA * cp, int event)
{
	int i;

	al_free(cp->event[event]);
	for(i = event; i < cp->events - 1; i++)
	{
		cp->event[i] = cp->event[i + 1];
	}
	cp->events--;
}

T3F_CINEMA * t3f_load_cinema(const char * fn)
{
	return NULL;
}

static bool get_config_bool(ALLEGRO_CONFIG * cp, const char * section, const char * key)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(val)
	{
		return !strcasecmp(val, "true");
	}
	return 0;
}

static int get_config_integer(ALLEGRO_CONFIG * cp, const char * section, const char * key)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(val)
	{
		return atoi(val);
	}
	return 0;
}

static bool get_config_float(ALLEGRO_CONFIG * cp, const char * section, const char * key, float * fp)
{
	const char * val;

	val = al_get_config_value(cp, section, key);
	if(val)
	{
		*fp = atof(val);
		return true;
	}
	*fp = 0.0;
	return false;
}

static void _t3f_load_cinema_event(T3F_CINEMA * cp, unsigned long id, const char * buf, int number, unsigned long * current_tick)
{
	T3F_CINEMA_EVENT_DATA event_data;
	int b, e, t, scene;
	float x, y, z, a, vx, vy, vz, va, s, is, jy, gy;
	bool use_x, use_y, use_z;
	float focus_x, focus_y;
	bool use_focus_x, use_focus_y;
	int etick = 0;
	bool use_vx, use_vy, use_vz, use_va;
	float cr, cg, cb, ca;
	bool use_c;
	unsigned long tick = 0;
	unsigned long target_tick = 0;
	const char * efn;
	const char * name;
	const char * val;
	int flags = 0;
	bool focus = false;

	/* determine if event is a break point */
	val = al_get_config_value(cp->script, buf, "break_point");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			flags |= T3F_CINEMA_EVENT_FLAG_BREAK;
		}
	}
	val = al_get_config_value(cp->script, buf, "wait_point");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			flags |= T3F_CINEMA_EVENT_FLAG_WAIT;
		}
	}

	tick = get_config_integer(cp->script, buf, "tick");
	*current_tick += tick; // use this to keep track of where we are
	target_tick = get_config_integer(cp->script, buf, "target_tick");
	name = al_get_config_value(cp->script, buf, "name");
	b = get_config_integer(cp->script, buf, "bitmap");
	t = get_config_integer(cp->script, buf, "type");
	e = get_config_integer(cp->script, buf, "entity");
	focus = get_config_bool(cp->script, buf, "focus");
	use_focus_x = get_config_float(cp->script, buf, "focus_x", &focus_x);
	use_focus_y = get_config_float(cp->script, buf, "focus_y", &focus_y);
	etick = get_config_integer(cp->script, buf, "etick");
	use_x = get_config_float(cp->script, buf, "x", &x);
	use_y = get_config_float(cp->script, buf, "y", &y);
	use_z = get_config_float(cp->script, buf, "z", &z);
	use_vx = get_config_float(cp->script, buf, "vx", &vx);
	use_vy = get_config_float(cp->script, buf, "vy", &vy);
	use_vz = get_config_float(cp->script, buf, "vz", &vz);
	use_va = get_config_float(cp->script, buf, "va", &va);
	get_config_float(cp->script, buf, "a", &a);
	get_config_float(cp->script, buf, "s", &s);
	get_config_float(cp->script, buf, "cr", &cr);
	get_config_float(cp->script, buf, "cg", &cg);
	get_config_float(cp->script, buf, "cb", &cb);
	get_config_float(cp->script, buf, "jump", &jy);
	get_config_float(cp->script, buf, "gravity", &gy);
	use_c = get_config_float(cp->script, buf, "ca", &ca);
	scene = get_config_integer(cp->script, buf, "scene");
	efn = al_get_config_value(cp->script, buf, "filename");
	memset(&event_data, 0, sizeof(T3F_CINEMA_EVENT_DATA));
	switch(t)
	{
		case T3F_CINEMA_EVENT_CLEAR:
		{
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_LOAD_SCENE:
		{
			event_data.load_scene.scene = scene;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_ADD_ENTITY:
		{
			event_data.add_entity.name = name;
			event_data.add_entity.bitmap = b;
			event_data.add_entity.x = x;
			event_data.add_entity.y = y;
			event_data.add_entity.z = z;
			event_data.add_entity.a = a;
			event_data.add_entity.tick = etick;
			if(s > 0.0)
			{
				event_data.add_entity.s = s;
			}

			/* use automatic scaling */
			else if(s < 0.0)
			{
				event_data.add_entity.s = 1.0 / (t3f_project_x(1.0, z) - t3f_project_x(0.0, z));
				event_data.add_entity.x -= (t3f_project_x(x, z) - x) * event_data.add_entity.s;
				event_data.add_entity.y -= (t3f_project_y(y, z) - y) * event_data.add_entity.s;
			}
			else
			{
				event_data.add_entity.s = 1.0;
			}
			if(use_c)
			{
				event_data.add_entity.color = al_map_rgba_f(cr, cg, cb, ca);
			}
			else
			{
				event_data.add_entity.color = al_map_rgba_f(1.0, 1.0, 1.0, 1.0);
			}
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_CHANGE_ENTITY:
		{
			event_data.change_entity.name = name;
			event_data.change_entity.entity = e;
			event_data.change_entity.bitmap = b;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_REMOVE_ENTITY:
		{
			event_data.remove_entity.name = name;
			event_data.remove_entity.entity = e;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_MOVE_ENTITY:
		{
			event_data.move_entity.name = name;
			event_data.move_entity.entity = e;
			event_data.move_entity.tick = *current_tick + target_tick;
			event_data.move_entity.ux = use_x;
			event_data.move_entity.uy = use_y;
			event_data.move_entity.uz = use_z;
			event_data.move_entity.x = x;
			event_data.move_entity.y = y;
			event_data.move_entity.z = z;
			event_data.move_entity.jy = jy;
			event_data.move_entity.gy = gy;

			/* use automatic scaling */
			if(s < 0.0)
			{
				is = 1.0 / (t3f_project_x(1.0, z) - t3f_project_x(0.0, z));
				event_data.move_entity.x -= (t3f_project_x(x, z) - x) * is;
				event_data.move_entity.y -= (t3f_project_y(y, z) - y) * is;
			}

			event_data.move_entity.a = a;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			event_data.stop_entity.entity = e;
			t3f_cinema_add_event(cp, id, *current_tick + target_tick, T3F_CINEMA_EVENT_STOP_ENTITY, event_data, 0);
			if(focus)
			{
				event_data.move_camera.tick = *current_tick + target_tick;
				event_data.move_camera.x = x - t3f_current_view->virtual_width / 2;
				if(use_focus_x)
				{
					event_data.move_camera.x += focus_x;
				}
				event_data.move_camera.y = y - t3f_current_view->virtual_height / 2;
				if(use_focus_y)
				{
					event_data.move_camera.y += focus_y;
				}
				event_data.move_camera.z = z;
				t3f_cinema_add_event(cp, id, *current_tick, T3F_CINEMA_EVENT_MOVE_CAMERA, event_data, flags);
				t3f_cinema_add_event(cp, id, *current_tick + target_tick, T3F_CINEMA_EVENT_STOP_CAMERA, event_data, 0);
			}
			break;
		}
		case T3F_CINEMA_EVENT_ACCEL_ENTITY:
		{
			event_data.accel_entity.name = name;
			event_data.accel_entity.entity = e;
			event_data.accel_entity.tick = *current_tick + target_tick;
			event_data.accel_entity.uvx = use_vx;
			event_data.accel_entity.uvy = use_vy;
			event_data.accel_entity.uvz = use_vz;
			event_data.accel_entity.uva = use_va;
			event_data.accel_entity.vx = vx;
			event_data.accel_entity.vy = vy;
			event_data.accel_entity.vz = vz;
			event_data.accel_entity.va = va;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_MOVE_CAMERA:
		{
			event_data.move_camera.tick = *current_tick + target_tick;
			event_data.move_camera.x = x;
			event_data.move_camera.y = y;
			event_data.move_camera.z = z;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			t3f_cinema_add_event(cp, id, *current_tick + target_tick, T3F_CINEMA_EVENT_STOP_CAMERA, event_data, 0);
			break;
		}
		case T3F_CINEMA_EVENT_FADE_ENTITY:
		{
			event_data.fade_entity.name = name;
			event_data.fade_entity.entity = e;
			event_data.fade_entity.tick = *current_tick + target_tick;
			event_data.fade_entity.color = al_map_rgba_f(cr, cg, cb, ca);
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			t3f_cinema_add_event(cp, id, *current_tick + target_tick, T3F_CINEMA_EVENT_STOP_FADE, event_data, 0);
			break;
		}
		case T3F_CINEMA_EVENT_COLOR_ENTITY:
		{
			event_data.color_entity.name = name;
			event_data.color_entity.entity = e;
			event_data.color_entity.color = al_map_rgba_f(cr, cg, cb, ca);
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_PLAY_MUSIC:
		{
			event_data.play_music.filename = efn;
			event_data.play_music.fade_ticks = s;
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_PLAY_SOUND:
		{
			if(efn)
			{
				event_data.play_sound.filename = efn;
				event_data.play_sound.pan = s;
				t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			}
			break;
		}
		case T3F_CINEMA_EVENT_NONE:
		{
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			break;
		}
		case T3F_CINEMA_EVENT_END:
		{
			t3f_cinema_add_event(cp, id, *current_tick, t, event_data, flags);
			cp->length = *current_tick;
			break;
		}
	}
}

T3F_CINEMA * t3f_load_cinema_script(const char * fn, float graphics_scale, int flags)
{
	T3F_CINEMA * cp = NULL;
	const char * val;
	char buf[256] = {0};
	int i;
	unsigned long current_tick = 0;

	cp = t3f_create_cinema();
	if(!cp)
	{
		goto fail;
	}
	cp->script = al_load_config_file(fn);
	if(!cp->script)
	{
		goto fail;
	}
	strcpy(cp->script_path, fn);
	cp->path = al_create_path(fn);
	if(!cp->path)
	{
		goto fail;
	}
	cp->graphics_scale = graphics_scale;
	cp->flags = flags;
	cp->entities = 0;

	/* load settings */
	val = al_get_config_value(cp->script, "Settings", "integer_rendering");
	if(val && !strcasecmp(val, "true"))
	{
		cp->flags |= T3F_CINEMA_FLAG_INTEGER_RENDERING;
	}
	val = al_get_config_value(cp->script, "Settings", "constrain_camera");
	if(val && !strcasecmp(val, "true"))
	{
		cp->flags |= T3F_CINEMA_FLAG_CONSTRAIN_CAMERA;
	}
	val = al_get_config_value(cp->script, "Settings", "developer_mode");
	if(val && !strcasecmp(val, "true"))
	{
		cp->flags |= T3F_CINEMA_FLAG_DEVELOPER_MODE;
	}
	cp->camera.width = t3f_current_view->virtual_width;
	cp->camera.height = t3f_current_view->virtual_height;
	val = al_get_config_value(cp->script, "Settings", "camera_width");
	if(val)
	{
		cp->camera.width = atoi(val);
	}
	val = al_get_config_value(cp->script, "Settings", "camera_height");
	if(val)
	{
		cp->camera.height = atoi(val);
	}

	/* load events */
	for(i = 0; i < T3F_CINEMA_MAX_SCRIPT_EVENTS; i++)
	{
		sprintf(buf, "event %d", i);

		val = al_get_config_value(cp->script, buf, "tick");
		if(val)
		{
			_t3f_load_cinema_event(cp, i, buf, i, &current_tick);
		}
	}

	/* start cinema at first frame */
	cp->tick = 0;

	/* set up first frame so we can render immediately, we'll clear it before
	 * actually processing the first tick */
	cp->first_frame = true;
	for(i = 0; i < cp->events; i++)
	{
		if(cp->event[i]->tick == cp->tick)
		{
			t3f_process_cinema_event(cp, i);
		}
	}
	cp->first_frame = false;
	cp->scene_start_event = 0;

	return cp;

	fail:
	{
		t3f_destroy_cinema(cp);
		return NULL;
	}
}

bool t3f_save_cinema(T3F_CINEMA * cp, const char * fn)
{
	return true;
}

void t3f_set_cinema_view_callbacks(T3F_CINEMA * cp, void (*logic_callback)(void * data), void (*render_callback)(void * data), void * data)
{
	cp->view_logic_callback = logic_callback;
	cp->view_callback = render_callback;
	cp->view_callback_data = data;
}

void t3f_set_cinema_event_callback(T3F_CINEMA * cp, bool (*callback)(T3F_CINEMA_EVENT * event, void * data), void * data)
{
	cp->event_callback = callback;
	cp->event_callback_data = data;
}

static void t3f_cinema_remove_entity(T3F_CINEMA * cp, int entity)
{
	int i;

	if(entity < cp->entities)
	{
		for(i = entity; i < cp->entities - 1; i++)
		{
			memcpy(&cp->entity[i], &cp->entity[i + 1], sizeof(T3F_CINEMA_ENTITY));
		}
		cp->entities--;
	}
}

static int _get_entity_id(T3F_CINEMA * cp, const char * name, int id)
{
	int i;

	if(name)
	{
		for(i = 0; i < cp->entities; i++)
		{
			if(cp->entity[i].name)
			{
				if(!strcmp(name, cp->entity[i].name))
				{
					return i;
				}
			}
		}
		if(i == cp->entities)
		{
			printf("Error: No match for entity name %s!\n", name);
		}
	}
	return id;
}

void t3f_process_cinema_event(T3F_CINEMA * cp, int event)
{
	int e, i;
	float d, r, g, b, a, dr, dg, db, da;
	unsigned long t;
	const char * val = NULL;
	char buf[256] = {0};
	char buf2[256] = {0};
	char buf3[1024];

	if(cp->event_callback && !cp->first_frame)
	{
		if(cp->event_callback(cp->event[event], cp->event_callback_data))
		{
			return;
		}
	}
	switch(cp->event[event]->type)
	{
		case T3F_CINEMA_EVENT_CLEAR:
		{
			cp->entities = 0;
			break;
		}
		case T3F_CINEMA_EVENT_LOAD_SCENE:
		{
			al_stop_timer(t3f_timer);
			/* destroy old scene bitmaps */
			for(i = 0; i < T3F_CINEMA_MAX_BITMAPS; i++)
			{
				if(cp->bitmap[i])
				{
					t3f_destroy_animation(cp->bitmap[i]);
					cp->bitmap[i] = NULL;
				}
			}

			/* load bitmaps for this scene */
			sprintf(buf, "scene %d", cp->event[event]->data.load_scene.scene);
			i = 0;
			for(i = 0; i < T3F_CINEMA_MAX_BITMAPS; i++)
			{
				sprintf(buf2, "bitmap %d", i);
				val = al_get_config_value(cp->script, buf, buf2);
				if(val)
				{
					al_set_path_filename(cp->path, val);
					cp->bitmap[i] = t3f_load_animation(al_path_cstr(cp->path, '/'), 0, false);
					if(!cp->bitmap[i])
					{
						printf("Failed to load %s!\n", al_path_cstr(cp->path, '/'));
						sprintf(buf3, "%s#Size %1.1f", val, cp->graphics_scale);
						cp->bitmap[i] = t3f_load_animation(buf3, 0, false);
						if(!cp->bitmap[i])
						{
							printf("Failed to load %s!\n", buf3);
						}
					}
				}
			}
			cp->entities = 0;
			cp->camera.x = 0.0;
			cp->camera.y = 0.0;
			cp->camera.z = 0.0;
			cp->scene = cp->event[event]->data.load_scene.scene;
			cp->scene_start_event = event;
			al_start_timer(t3f_timer);
			break;
		}
		case T3F_CINEMA_EVENT_ADD_ENTITY:
		{
			cp->entity[cp->entities].name = cp->event[event]->data.add_entity.name;
			cp->entity[cp->entities].bitmap = cp->event[event]->data.add_entity.bitmap;
			cp->entity[cp->entities].x = cp->event[event]->data.add_entity.x;
			cp->entity[cp->entities].y = cp->event[event]->data.add_entity.y;
			cp->entity[cp->entities].z = cp->event[event]->data.add_entity.z;
			cp->entity[cp->entities].a = cp->event[event]->data.add_entity.a;
			cp->entity[cp->entities].s = cp->event[event]->data.add_entity.s;
			cp->entity[cp->entities].vx = 0.0;
			cp->entity[cp->entities].vy = 0.0;
			cp->entity[cp->entities].vz = 0.0;
			cp->entity[cp->entities].va = 0.0;
			al_unmap_rgba_f(cp->event[event]->data.add_entity.color, &cp->entity[cp->entities].cr, &cp->entity[cp->entities].cg, &cp->entity[cp->entities].cb, &cp->entity[cp->entities].ca);
			cp->entity[cp->entities].cvr = 0.0;
			cp->entity[cp->entities].cvg = 0.0;
			cp->entity[cp->entities].cvb = 0.0;
			cp->entity[cp->entities].cva = 0.0;
			cp->entity[cp->entities].color = cp->event[event]->data.add_entity.color;
			cp->entity[cp->entities].tick = cp->event[event]->data.add_entity.tick;
			cp->entity[cp->entities].by = cp->entity[cp->entities].y;
			cp->entity[cp->entities].jy = 0.0;
			cp->entity[cp->entities].gy = 0.0;
			cp->entities++;
			break;
		}
		case T3F_CINEMA_EVENT_CHANGE_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.change_entity.name, cp->event[event]->data.change_entity.entity);
			cp->entity[e].bitmap = cp->event[event]->data.change_entity.bitmap;
			break;
		}
		case T3F_CINEMA_EVENT_REMOVE_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.remove_entity.name, cp->event[event]->data.remove_entity.entity);
			t3f_cinema_remove_entity(cp, e);
			break;
		}
		case T3F_CINEMA_EVENT_MOVE_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.move_entity.name, cp->event[event]->data.move_entity.entity);
			t = cp->event[event]->data.move_entity.tick;
			if(cp->flags & T3F_CINEMA_FLAG_DEVELOPER_MODE)
			{
				sprintf(buf, "event %lu", cp->event[event]->id);
				sprintf(buf2, "%d", (int)cp->bitmap[cp->entity[e].bitmap]->data->frame[0]->width / 2);
				al_set_config_value(cp->script, buf, "focus_x", buf2);
				sprintf(buf2, "%d", (int)cp->bitmap[cp->entity[e].bitmap]->data->frame[0]->height / 2);
				al_set_config_value(cp->script, buf, "focus_y", buf2);
			}

			/* if target tick is the current tick, move entity immediately */
			if(t - cp->event[event]->tick <= 0)
			{
				if(cp->event[event]->data.move_entity.ux)
				{
					cp->entity[e].x = cp->event[event]->data.move_entity.x;
				}
				if(cp->event[event]->data.move_entity.uy)
				{
					cp->entity[e].y = cp->event[event]->data.move_entity.y;
				}
				if(cp->event[event]->data.move_entity.uz)
				{
					cp->entity[e].z = cp->event[event]->data.move_entity.z;
				}
				cp->entity[e].a = cp->event[event]->data.move_entity.a;
			}
			else
			{
				if(cp->event[event]->data.move_entity.ux)
				{
					d = (cp->event[event]->data.move_entity.x - cp->entity[e].x) / (float)(t - cp->event[event]->tick);
					cp->entity[e].vx = d;
				}
				if(cp->event[event]->data.move_entity.uy)
				{
					d = (cp->event[event]->data.move_entity.y - cp->entity[e].y) / (float)(t - cp->event[event]->tick);
					cp->entity[e].vy = d;
				}
				if(cp->event[event]->data.move_entity.uz)
				{
					d = (cp->event[event]->data.move_entity.z - cp->entity[e].z) / (float)(t - cp->event[event]->tick);
					cp->entity[e].vz = d;
				}
				d = (cp->event[event]->data.move_entity.a - cp->entity[e].a) / (float)(t - cp->event[event]->tick);
				cp->entity[e].va = d;
				cp->entity[e].by = cp->entity[e].y;
				cp->entity[e].gy = cp->event[event]->data.move_entity.gy;
				cp->entity[e].jy = cp->event[event]->data.move_entity.jy;
			}
			break;
		}
		case T3F_CINEMA_EVENT_ACCEL_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.move_entity.name, cp->event[event]->data.move_entity.entity);

			if(cp->event[event]->data.accel_entity.uvx)
			{
				cp->entity[e].vx = cp->event[event]->data.accel_entity.vx;
			}
			if(cp->event[event]->data.accel_entity.uvy)
			{
				cp->entity[e].vy = cp->event[event]->data.accel_entity.vy;
			}
			if(cp->event[event]->data.accel_entity.uvz)
			{
				cp->entity[e].vz = cp->event[event]->data.accel_entity.vz;
			}
			if(cp->event[event]->data.accel_entity.uva)
			{
				cp->entity[e].va = cp->event[event]->data.accel_entity.va;
			}
			break;
		}
		case T3F_CINEMA_EVENT_MOVE_CAMERA:
		{
			int target_x, target_y;
			t = cp->event[event]->data.move_camera.tick;

			target_x = cp->event[event]->data.move_camera.x;
			target_y = cp->event[event]->data.move_camera.y;
			if(cp->flags & T3F_CINEMA_FLAG_CONSTRAIN_CAMERA)
			{
				if(target_x < 0)
				{
					target_x = 0;
				}
				else if(target_x + t3f_current_view->virtual_width > cp->camera.width)
				{
					target_x = cp->camera.width - t3f_current_view->virtual_width;
				}
				if(target_y < 0)
				{
					target_y = 0;
				}
				else if(target_y + t3f_current_view->virtual_height > cp->camera.height)
				{
					target_y = cp->camera.height - t3f_current_view->virtual_height;
				}
			}
			/* if target tick is the current tick, move camera immediately */
			if(t - cp->event[event]->tick <= 0)
			{
				cp->camera.x = target_x;
				cp->camera.y = target_y;
				cp->camera.z = cp->event[event]->data.move_camera.z;
			}
			else
			{
				d = (target_x - cp->camera.x) / (float)(t - cp->event[event]->tick);
				cp->camera.vx = d;
				d = (target_y - cp->camera.y) / (float)(t - cp->event[event]->tick);
				cp->camera.vy = d;
				d = (cp->event[event]->data.move_camera.z - cp->camera.z) / (float)(t - cp->event[event]->tick);
				cp->camera.vz = d;
			}
			break;
		}
		case T3F_CINEMA_EVENT_FADE_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.fade_entity.name, cp->event[event]->data.fade_entity.entity);
			t = cp->event[event]->data.fade_entity.tick;
			if(e >= 0)
			{
				al_unmap_rgba_f(cp->entity[e].color, &r, &g, &b, &a);
				al_unmap_rgba_f(cp->event[event]->data.fade_entity.color, &dr, &dg, &db, &da);
				d = (dr - r) / (float)(t - cp->event[event]->tick);
				cp->entity[e].cvr = d;
				d = (dg - g) / (float)(t - cp->event[event]->tick);
				cp->entity[e].cvg = d;
				d = (db - b) / (float)(t - cp->event[event]->tick);
				cp->entity[e].cvb = d;
				d = (da - a) / (float)(t - cp->event[event]->tick);
				cp->entity[e].cva = d;
			}
			else
			{
				for(i = 0; i < cp->entities; i++)
				{
					al_unmap_rgba_f(cp->entity[i].color, &r, &g, &b, &a);
					al_unmap_rgba_f(cp->event[event]->data.fade_entity.color, &dr, &dg, &db, &da);
					d = (dr - r) / (float)(t - cp->event[event]->tick);
					cp->entity[i].cvr = d;
					d = (dg - g) / (float)(t - cp->event[event]->tick);
					cp->entity[i].cvg = d;
					d = (db - b) / (float)(t - cp->event[event]->tick);
					cp->entity[i].cvb = d;
					d = (da - a) / (float)(t - cp->event[event]->tick);
					cp->entity[i].cva = d;
				}
			}
			break;
		}
		case T3F_CINEMA_EVENT_COLOR_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.color_entity.name, cp->event[event]->data.color_entity.entity);
			cp->entity[e].color = cp->event[event]->data.color_entity.color;
			al_unmap_rgba_f(cp->entity[e].color, &cp->entity[e].cr, &cp->entity[e].cg, &cp->entity[e].cb, &cp->entity[e].ca);
			break;
		}
		case T3F_CINEMA_EVENT_STOP_ENTITY:
		{
			e = _get_entity_id(cp, cp->event[event]->data.stop_entity.name, cp->event[event]->data.stop_entity.entity);
			cp->entity[e].vx = 0.0;
			cp->entity[e].vy = 0.0;
			cp->entity[e].vz = 0.0;
			cp->entity[e].va = 0.0;
			cp->entity[e].gy = 0.0;
			break;
		}
		case T3F_CINEMA_EVENT_STOP_FADE:
		{
			e = _get_entity_id(cp, cp->event[event]->data.color_entity.name, cp->event[event]->data.color_entity.entity);
			if(e >= 0)
			{
				cp->entity[cp->event[event]->data.stop_entity.entity].cvr = 0.0;
				cp->entity[cp->event[event]->data.stop_entity.entity].cvg = 0.0;
				cp->entity[cp->event[event]->data.stop_entity.entity].cvb = 0.0;
				cp->entity[cp->event[event]->data.stop_entity.entity].cva = 0.0;
			}
			else
			{
				for(i = 0; i < cp->entities; i++)
				{
					cp->entity[i].cvr = 0.0;
					cp->entity[i].cvg = 0.0;
					cp->entity[i].cvb = 0.0;
					cp->entity[i].cva = 0.0;
				}
			}
			break;
		}
		case T3F_CINEMA_EVENT_STOP_CAMERA:
		{
			cp->camera.vx = 0.0;
			cp->camera.vy = 0.0;
			cp->camera.vz = 0.0;
			break;
		}
		case T3F_CINEMA_EVENT_PLAY_MUSIC:
		{
			if(!cp->first_frame)
			{
				if(cp->event[event]->data.play_music.filename)
				{
					t3f_play_music(cp->event[event]->data.play_music.filename);
				}
			}
			break;
		}
		case T3F_CINEMA_EVENT_PLAY_SOUND:
		{
			if(!cp->first_frame)
			{
				t3f_stream_sample(cp->event[event]->data.play_sound.filename, 1.0, cp->event[event]->data.play_sound.pan, 1.0);
			}
			break;
		}
	}
}

static bool _t3f_detect_cinema_wait(T3F_CINEMA * cp)
{
	int i;

	for(i = cp->scene_start_event; i < cp->events; i++)
	{
		if(cp->event[i]->tick == cp->tick)
		{
			if(cp->event[i]->flags & T3F_CINEMA_EVENT_FLAG_WAIT)
			{
				return true;
			}
		}
	}
	return false;
}

static bool _t3f_process_cinema_tick(T3F_CINEMA * cp)
{
	int i;
	bool ret = false;

	/* clear entities on first tick */
	if(cp->tick == 0)
	{
		for(i = 0; i < cp->entities; i++)
		{
			t3f_cinema_remove_entity(cp, 0);
		}
	}

	if(!(cp->flags & T3F_CINEMA_FLAG_DEVELOPER_MODE))
	{
		if(_t3f_detect_cinema_wait(cp) && !cp->wait_skipped)
		{
			cp->state = T3F_CINEMA_STATE_WAIT;
			return true;
		}
	}
	cp->wait_skipped = false;
	for(i = cp->scene_start_event; i < cp->events; i++)
	{
		if(cp->event[i]->tick == cp->tick)
		{
			t3f_process_cinema_event(cp, i);
			if(cp->event[i]->flags & T3F_CINEMA_EVENT_FLAG_BREAK)
			{
				ret = true;
			}
		}
	}
	cp->tick++;

	/* handle entity movement */
	for(i = 0; i < cp->entities; i++)
	{
		cp->entity[i].x += cp->entity[i].vx;
		cp->entity[i].y += cp->entity[i].vy;
		cp->entity[i].z += cp->entity[i].vz;
		cp->entity[i].a += cp->entity[i].va;
		cp->entity[i].cr += cp->entity[i].cvr;
		cp->entity[i].cg += cp->entity[i].cvg;
		cp->entity[i].cb += cp->entity[i].cvb;
		cp->entity[i].ca += cp->entity[i].cva;
		cp->entity[i].color = al_map_rgba_f(cp->entity[i].cr, cp->entity[i].cg, cp->entity[i].cb, cp->entity[i].ca);

		/* process gravity */
		if(cp->entity[i].gy > 0.0)
		{
			cp->entity[i].vy += cp->entity[i].gy;
			if(cp->entity[i].y >= cp->entity[i].by)
			{
				cp->entity[i].y = cp->entity[i].by;
				cp->entity[i].vy = cp->entity[i].jy;
			}
		}
		cp->entity[i].tick++;

		/* process the logic for the view if this is a view entity */
		if(cp->entity[i].bitmap < 0 && cp->view_logic_callback)
		{
			cp->view_logic_callback(cp->view_callback_data);
		}
	}

	/* handle camera movement */
	cp->camera.x += cp->camera.vx;
	cp->camera.y += cp->camera.vy;
	cp->camera.z += cp->camera.vz;
	cp->camera.render_x = cp->camera.x;
	cp->camera.render_y = cp->camera.y;
	if(cp->flags & T3F_CINEMA_FLAG_INTEGER_RENDERING)
	{
		cp->camera.render_x = (int)cp->camera.render_x;
		cp->camera.render_y = (int)cp->camera.render_y;
	}

	if(cp->tick > cp->length)
	{
		ret = true;
	}
	return ret;
}

bool t3f_process_cinema(T3F_CINEMA * cp)
{
	bool ret = false;

	switch(cp->state)
	{
		case T3F_CINEMA_STATE_PLAY:
		{
			ret = _t3f_process_cinema_tick(cp);
			break;
		}
	}

	return ret;
}

void t3f_render_cinema(T3F_CINEMA * cp)
{
	int i;
	float cx, cy;
	float ox, oy;
	float w, h;

	/* render entities */
	t3f_set_clipping_rectangle(-cp->camera.render_x, -cp->camera.render_y, cp->camera.width, cp->camera.height);
	for(i = 0; i < cp->entities; i++)
	{
		/* use the view callback in this case, we want to be able to render the
		 * game into this view while the cinema is still running */
		if(cp->entity[i].bitmap < 0)
		{
			if(cp->view_callback)
			{
				ox = t3f_project_x(cp->entity[i].x - cp->camera.x, cp->entity[i].z - cp->camera.z);
				oy = t3f_project_y(cp->entity[i].y - cp->camera.y, cp->entity[i].z - cp->camera.z);
				w = t3f_project_x(cp->entity[i].x - cp->camera.x + t3f_virtual_display_width, cp->entity[i].z - cp->camera.z) - ox;
				h = t3f_project_y(cp->entity[i].y - cp->camera.y + t3f_virtual_display_height, cp->entity[i].z - cp->camera.z) - oy;
				t3f_adjust_view(cp->view, ox, oy, w * cp->entity[i].s, h * cp->entity[i].s, cp->view->vp_x, cp->view->vp_y, T3F_FORCE_ASPECT);
				t3f_select_view(cp->view);
				cp->view_callback(cp->view_callback_data);
				t3f_select_view(NULL);
				al_hold_bitmap_drawing(false);
			}
		}
		else if(cp->bitmap[cp->entity[i].bitmap])
		{
			cx = cp->bitmap[cp->entity[i].bitmap]->data->frame[0]->width / 2;
			cy = cp->bitmap[cp->entity[i].bitmap]->data->frame[0]->height / 2;
			t3f_draw_rotated_scaled_animation(cp->bitmap[cp->entity[i].bitmap], cp->entity[i].color, cp->entity[i].tick, cx, cy, cp->entity[i].x - cp->camera.render_x + cx * cp->entity[i].s, cp->entity[i].y - cp->camera.render_y + cy * cp->entity[i].s, cp->entity[i].z - cp->camera.z, cp->entity[i].a, cp->entity[i].s, (cp->flags & T3F_CINEMA_FLAG_INTEGER_RENDERING) ? T3F_DRAW_INTEGER_SNAP : 0);
		}
	}
	t3f_set_clipping_rectangle(0, 0, 0, 0);
}

static int _t3f_cinema_click_count = 0;

void t3f_block_cinema_skip(void)
{
	_t3f_cinema_click_count = 10;
}

void t3f_handle_cinema_skip(T3F_CINEMA * cp)
{
	if(cp->state == T3F_CINEMA_STATE_WAIT)
	{
		cp->state = T3F_CINEMA_STATE_PLAY;
	}
	else
	{
		while(!t3f_process_cinema(cp))
		{
		}
	}
	cp->wait_skipped = true;
}
