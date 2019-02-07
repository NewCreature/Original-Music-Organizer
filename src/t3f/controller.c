#include <allegro5/allegro5.h>
#include "t3f.h"
#include "controller.h"

static char t3f_binding_return_name[256] = {0};
static char t3f_controller_return_name[256] = {0};
static float t3f_controller_axis_threshold = 0.3;

T3F_CONTROLLER * t3f_create_controller(int bindings)
{
	T3F_CONTROLLER * cp;

	cp = malloc(sizeof(T3F_CONTROLLER));
	if(!cp)
	{
		return NULL;
	}
	memset(cp, 0, sizeof(T3F_CONTROLLER));
	cp->bindings = bindings;
	return cp;
}

void t3f_destroy_controller(T3F_CONTROLLER * cp)
{
	free(cp);
}

const char * t3f_get_controller_name(T3F_CONTROLLER * cp, int binding)
{
	switch(cp->binding[binding].type)
	{
		case T3F_CONTROLLER_BINDING_KEY:
		{
			sprintf(t3f_controller_return_name, "Keyboard");
			return t3f_controller_return_name;
		}
		case T3F_CONTROLLER_BINDING_MOUSE_BUTTON:
		{
			sprintf(t3f_controller_return_name, "Mouse");
			return t3f_controller_return_name;
		}
		case T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON:
		case T3F_CONTROLLER_BINDING_JOYSTICK_AXIS:
		{
			if(t3f_joystick[cp->binding[binding].joystick])
			{
				sprintf(t3f_controller_return_name, "%s (%d)", al_get_joystick_name(t3f_joystick[cp->binding[binding].joystick]), cp->binding[binding].joystick);
			}
			else
			{
				sprintf(t3f_controller_return_name, "N/A");
			}
			return t3f_controller_return_name;
		}
	}
	t3f_controller_return_name[0] = 0;
	return t3f_controller_return_name;
}

static const char * unknown_binding_string = "Unknown";

static bool string_is_empty(const char * s)
{
	int i;

	if(s)
	{
		for(i = 0; i < strlen(s); i++)
		{
			if(s[i] != ' ')
			{
				return false;
			}
		}
	}
	return true;
}

const char * t3f_get_controller_binding_name(T3F_CONTROLLER * cp, int binding)
{
	const char * stick_name;
	const char * name;
	switch(cp->binding[binding].type)
	{
		case T3F_CONTROLLER_BINDING_KEY:
		{
			sprintf(t3f_binding_return_name, "%s", al_keycode_to_name(cp->binding[binding].button));
			if(string_is_empty(t3f_binding_return_name))
			{
				return unknown_binding_string;
			}
			else
			{
				return t3f_binding_return_name;
			}
		}
		case T3F_CONTROLLER_BINDING_MOUSE_BUTTON:
		{
			sprintf(t3f_binding_return_name, "Button %d", cp->binding[binding].button);
			if(string_is_empty(t3f_binding_return_name))
			{
				return unknown_binding_string;
			}
			else
			{
				return t3f_binding_return_name;
			}
		}
		case T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON:
		{
			if(t3f_joystick[cp->binding[binding].joystick])
			{
				name = al_get_joystick_button_name(t3f_joystick[cp->binding[binding].joystick], cp->binding[binding].button);
				sprintf(t3f_binding_return_name, "%s", name ? name : "???");
			}
			else
			{
				sprintf(t3f_binding_return_name, "N/A");
			}
			if(string_is_empty(t3f_binding_return_name))
			{
				return unknown_binding_string;
			}
			else
			{
				return t3f_binding_return_name;
			}
		}
		case T3F_CONTROLLER_BINDING_JOYSTICK_AXIS:
		{
			if(t3f_joystick[cp->binding[binding].joystick])
			{
				stick_name = al_get_joystick_stick_name(t3f_joystick[cp->binding[binding].joystick], cp->binding[binding].stick);
				name = al_get_joystick_axis_name(t3f_joystick[cp->binding[binding].joystick], cp->binding[binding].stick, cp->binding[binding].axis);
				sprintf(t3f_binding_return_name, "%s %s%s%s", stick_name ? stick_name : "???", name ? name : "???", (cp->binding[binding].flags & T3F_CONTROLLER_FLAG_AXIS_NEGATIVE) ? "-" : "+", (cp->binding[binding].flags & T3F_CONTROLLER_FLAG_AXIS_INVERT) ? "(I)" : "");
			}
			else
			{
				sprintf(t3f_binding_return_name, "N/A");
			}
			if(string_is_empty(t3f_binding_return_name))
			{
				return unknown_binding_string;
			}
			else
			{
				return t3f_binding_return_name;
			}
		}
	}
	return unknown_binding_string;
}

void t3f_write_controller_config(ALLEGRO_CONFIG * acp, const char * section, T3F_CONTROLLER * cp)
{
	char temp_string[1024] = {0};
	char temp_string2[1024] = {0};
	int j;

	al_set_config_value(acp, section, "Device Name", cp->device_name ? cp->device_name : "");
	for(j = 0; j < cp->bindings; j++)
	{
		sprintf(temp_string, "Binding %d Type", j);
		sprintf(temp_string2, "%d", cp->binding[j].type);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d SubType", j);
		sprintf(temp_string2, "%d", cp->binding[j].sub_type);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d Joystick", j);
		sprintf(temp_string2, "%d", cp->binding[j].joystick);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d Button", j);
		sprintf(temp_string2, "%d", cp->binding[j].button);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d Stick", j);
		sprintf(temp_string2, "%d", cp->binding[j].stick);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d Axis", j);
		sprintf(temp_string2, "%d", cp->binding[j].axis);
		al_set_config_value(acp, section, temp_string, temp_string2);
		sprintf(temp_string, "Binding %d Flags", j);
		sprintf(temp_string2, "%d", cp->binding[j].flags);
		al_set_config_value(acp, section, temp_string, temp_string2);
		if(cp->binding[j].type == T3F_CONTROLLER_BINDING_JOYSTICK_AXIS)
		{
			sprintf(temp_string, "Binding %d Min", j);
			sprintf(temp_string2, "%f", cp->binding[j].min);
			al_set_config_value(acp, section, temp_string, temp_string2);
			sprintf(temp_string, "Binding %d Mid", j);
			sprintf(temp_string2, "%f", cp->binding[j].mid);
			al_set_config_value(acp, section, temp_string, temp_string2);
			sprintf(temp_string, "Binding %d Max", j);
			sprintf(temp_string2, "%f", cp->binding[j].max);
			al_set_config_value(acp, section, temp_string, temp_string2);
		}
	}
}

static int matched[T3F_MAX_JOYSTICKS] = {0};

static void remap_controller_to_device(T3F_CONTROLLER * cp)
{
	int i;
	int matching_device = -1;

	if(cp->device_name)
	{
		for(i = 0; i < al_get_num_joysticks(); i++)
		{
			if(!strcmp(cp->device_name, al_get_joystick_name(t3f_joystick[i])) && !matched[i])
			{
				matching_device = i;
				matched[i] = 1;
				break;
			}
		}
		if(matching_device >= 0)
		{
			for(i = 0; i < cp->bindings; i++)
			{
				cp->binding[i].joystick = matching_device;
			}
		}
	}
}

bool t3f_read_controller_config(ALLEGRO_CONFIG * acp, const char * section, T3F_CONTROLLER * cp)
{
	char temp_string[1024] = {0};
	const char * item;
	int j;

	cp->device_name = al_get_config_value(acp, section, "Device Name");
	for(j = 0; j < cp->bindings; j++)
	{
		sprintf(temp_string, "Binding %d Type", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].type = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d SubType", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].sub_type = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d Joystick", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].joystick = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d Button", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].button = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d Stick", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].stick = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d Axis", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].axis = atoi(item);
		}
		else
		{
			return false;
		}
		sprintf(temp_string, "Binding %d Flags", j);
		item = al_get_config_value(acp, section, temp_string);
		if(item)
		{
			cp->binding[j].flags = atoi(item);
		}
		else
		{
			return false;
		}
		if(cp->binding[j].type == T3F_CONTROLLER_BINDING_JOYSTICK_AXIS)
		{
			sprintf(temp_string, "Binding %d Min", j);
			item = al_get_config_value(acp, section, temp_string);
			if(item)
			{
				cp->binding[j].min = atof(item);
			}
			else
			{
				return false;
			}
			sprintf(temp_string, "Binding %d Mid", j);
			item = al_get_config_value(acp, section, temp_string);
			if(item)
			{
				cp->binding[j].mid = atof(item);
			}
			else
			{
				return false;
			}
			sprintf(temp_string, "Binding %d Max", j);
			item = al_get_config_value(acp, section, temp_string);
			if(item)
			{
				cp->binding[j].max = atof(item);
			}
			else
			{
				return false;
			}
		}
	}
	if(cp->device_name)
	{
		remap_controller_to_device(cp);
	}
	return true;
}

static bool binding_is_joystick(T3F_CONTROLLER * cp, int i)
{
	if(cp->binding[i].type == T3F_CONTROLLER_BINDING_JOYSTICK_AXIS || cp->binding[i].type == T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON)
	{
		return true;
	}
	return false;
}

static void t3f_find_controller_device_name(T3F_CONTROLLER * cp)
{
	int i;
	int source;

	/* only get device name for joysticks */
	if(binding_is_joystick(cp, 0))
	{
		source = cp->binding[0].joystick;
		for(i = 1; i < cp->bindings; i++)
		{
			if(!binding_is_joystick(cp, i) || cp->binding[i].joystick != source)
			{
				return;
			}
		}
		cp->device_name = al_get_joystick_name(t3f_joystick[source]);
	}
}

bool t3f_bind_controller(T3F_CONTROLLER * cp, int binding)
{
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_EVENT event;
	const char * val;
	int i, jn;

	queue = al_create_event_queue();
	if(!queue)
	{
		return false;
	}
	if(t3f_flags & T3F_USE_KEYBOARD)
	{
		al_register_event_source(queue, al_get_keyboard_event_source());
	}
	if(t3f_flags & T3F_USE_MOUSE)
	{
		al_register_event_source(queue, al_get_mouse_event_source());
	}
	if(t3f_flags & T3F_USE_JOYSTICK)
	{
		al_register_event_source(queue, al_get_joystick_event_source());
		for(i = 0; i < al_get_num_joysticks() - 1; i++)
		{
			t3f_joystick[i] = al_get_joystick(i);
			if(t3f_joystick[i])
			{
				al_get_joystick_state(t3f_joystick[i], &t3f_joystick_state[i]); // read initial state
			}
		}
	}
	val = al_get_config_value(t3f_config, "T3F", "joystick_axis_threshold");
	if(val)
	{
		t3f_controller_axis_threshold = atof(val);
	}
	while(1)
	{
		al_wait_for_event(queue, &event);
		switch(event.type)
		{
			/* key was pressed or repeated */
			case ALLEGRO_EVENT_KEY_DOWN:
			{
				if(event.keyboard.keycode != ALLEGRO_KEY_ESCAPE)
				{
					cp->binding[binding].type = T3F_CONTROLLER_BINDING_KEY;
					cp->binding[binding].button = event.keyboard.keycode;
					t3f_find_controller_device_name(cp);
					al_destroy_event_queue(queue);
					return true;
				}
				else
				{
					al_destroy_event_queue(queue);
					return false;
				}
			}
			case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
			{
				cp->binding[binding].type = T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON;
				cp->binding[binding].joystick = t3f_get_joystick_number(event.joystick.id);
				cp->binding[binding].stick = event.joystick.stick;
				cp->binding[binding].button = event.joystick.button;
				t3f_find_controller_device_name(cp);
				al_destroy_event_queue(queue);
				return true;
			}
			case ALLEGRO_EVENT_JOYSTICK_AXIS:
			{
				jn = t3f_get_joystick_number(event.joystick.id);
				if(jn >= 0)
				{
					if(event.joystick.pos < -T3F_CONTROLLER_AXIS_THRESHOLD && t3f_joystick_state[jn].stick[event.joystick.stick].axis[event.joystick.axis] >= -T3F_CONTROLLER_AXIS_THRESHOLD)
					{
						cp->binding[binding].type = T3F_CONTROLLER_BINDING_JOYSTICK_AXIS;
						cp->binding[binding].joystick = jn;
						cp->binding[binding].stick = event.joystick.stick;
						cp->binding[binding].axis = event.joystick.axis;
						cp->binding[binding].flags = T3F_CONTROLLER_FLAG_AXIS_NEGATIVE;
						t3f_find_controller_device_name(cp);
						al_destroy_event_queue(queue);
						return true;
					}
					else if(event.joystick.pos > T3F_CONTROLLER_AXIS_THRESHOLD && t3f_joystick_state[jn].stick[event.joystick.stick].axis[event.joystick.axis] <= T3F_CONTROLLER_AXIS_THRESHOLD)
					{
						cp->binding[binding].type = T3F_CONTROLLER_BINDING_JOYSTICK_AXIS;
						cp->binding[binding].joystick = jn;
						cp->binding[binding].stick = event.joystick.stick;
						cp->binding[binding].axis = event.joystick.axis;
						cp->binding[binding].flags = T3F_CONTROLLER_FLAG_AXIS_POSITIVE;
						t3f_find_controller_device_name(cp);
						al_destroy_event_queue(queue);
						return true;
					}
				}
				if(event.joystick.pos >= -T3F_CONTROLLER_AXIS_THRESHOLD && event.joystick.pos <= T3F_CONTROLLER_AXIS_THRESHOLD)
				{
//					ppos = event.joystick.pos;
				}
				if(al_get_joystick_stick_flags(event.joystick.id, event.joystick.stick) & ALLEGRO_JOYFLAG_DIGITAL)
				{
//					ppos = 0.1;
				}
				break;
			}
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			{
				cp->binding[binding].type = T3F_CONTROLLER_BINDING_MOUSE_BUTTON;
				cp->binding[binding].button = event.mouse.button;
				t3f_find_controller_device_name(cp);
				al_destroy_event_queue(queue);
				return true;
			}
		}
	}
	return false;
}

/* see which buttons are held */
void t3f_read_controller(T3F_CONTROLLER * cp)
{
	int i;
	float vpos, vscale;

	for(i = 0; i < cp->bindings; i++)
	{
		switch(cp->binding[i].type)
		{
			case T3F_CONTROLLER_BINDING_KEY:
			{
				if(t3f_key[cp->binding[i].button])
				{
					cp->state[i].down = true;
				}
				else
				{
					cp->state[i].down = false;
				}
				break;
			}
			case T3F_CONTROLLER_BINDING_JOYSTICK_BUTTON:
			{
				if(t3f_joystick_state[cp->binding[i].joystick].button[cp->binding[i].button])
				{
					cp->state[i].down = true;
				}
				else
				{
					cp->state[i].down = false;
				}
				break;
			}
			case T3F_CONTROLLER_BINDING_JOYSTICK_AXIS:
			{
				bool held = false;

				/* get analog position */
				cp->state[i].pos = t3f_joystick_state[cp->binding[i].joystick].stick[cp->binding[i].stick].axis[cp->binding[i].axis];
				if(cp->binding[i].flags & T3F_CONTROLLER_FLAG_AXIS_INVERT)
				{
					cp->state[i].pos = -cp->state[i].pos;
				}

				/* correct position for controller configuration */
				if(!(cp->binding[i].flags & T3F_CONTROLLER_FLAG_AXIS_NO_ADJUST))
				{
					vpos = cp->state[i].pos - cp->binding[i].mid;
					if(vpos < 0.0)
					{
						vscale = 1.0 / fabs(cp->binding[i].min - cp->binding[i].mid);
					}
					else
					{
						vscale = 1.0 / fabs(cp->binding[i].mid - cp->binding[i].max);
					}
					vpos *= vscale;
					cp->state[i].pos = vpos;
				}

				/* if position is past threshold, consider the axis pressed */
				if((cp->binding[i].flags & T3F_CONTROLLER_FLAG_AXIS_NEGATIVE) && t3f_joystick_state[cp->binding[i].joystick].stick[cp->binding[i].stick].axis[cp->binding[i].axis] < -T3F_CONTROLLER_AXIS_THRESHOLD)
				{
					held = true;
				}
				else if((cp->binding[i].flags & T3F_CONTROLLER_FLAG_AXIS_POSITIVE) && t3f_joystick_state[cp->binding[i].joystick].stick[cp->binding[i].stick].axis[cp->binding[i].axis] > T3F_CONTROLLER_AXIS_THRESHOLD)
				{
					held = true;
				}
				if(held)
				{
					cp->state[i].down = true;
				}
				else
				{
					cp->state[i].down = false;
				}
				break;
			}
		}
	}
}

/* update pressed/released variables */
void t3f_update_controller(T3F_CONTROLLER * cp)
{
	int i;

	for(i = 0; i < cp->bindings; i++)
	{
		cp->state[i].was_held = cp->state[i].held;
		if(cp->state[i].down)
		{
			if(!cp->state[i].was_held)
			{
				cp->state[i].pressed = true;
				cp->state[i].released = false;
			}
			else
			{
				cp->state[i].pressed = false;
			}
			cp->state[i].held = true;
		}
		else
		{
			if(cp->state[i].was_held)
			{
				cp->state[i].released = true;
			}
			else
			{
				cp->state[i].released = false;
			}
			cp->state[i].pressed = false;
			cp->state[i].held = false;
		}
	}
}

void t3f_clear_controller_state(T3F_CONTROLLER * cp)
{
	int i;

	for(i = 0; i < cp->bindings; i++)
	{
		cp->state[i].down = false;
		cp->state[i].held = false;
		cp->state[i].was_held = false;
		cp->state[i].pressed = false;
		cp->state[i].released = false;
	}
}
