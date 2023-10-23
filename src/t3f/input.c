#include "t3f.h"
#include "input.h"

static bool _t3f_input_initialized = false;
ALLEGRO_JOYSTICK_STATE _input_state_fudging_helper[T3F_MAX_JOYSTICKS];

static void _reset_input_state_fudging(void)
{
  int i;

  #ifdef ALLEGRO_MACOSX
    for(i = 0; i < T3F_MAX_JOYSTICKS; i++)
    {
      _input_state_fudging_helper[i].stick[4].axis[0] = 100.0;
      _input_state_fudging_helper[i].stick[4].axis[1] = 100.0;
      _input_state_fudging_helper[i].stick[2].axis[0] = 100.0;
      _input_state_fudging_helper[i].stick[3].axis[0] = 100.0;
    }
  #else
    #ifdef ALLEGRO_UNIX
      for(i = 0; i < T3F_MAX_JOYSTICKS; i++)
      {
        _input_state_fudging_helper[i].stick[1].axis[0] = 100.0;
        _input_state_fudging_helper[i].stick[2].axis[1] = 100.0;
      }
    #endif
  #endif
}

bool t3f_initialize_input(int flags)
{
  if(!_t3f_input_initialized)
  {
    _reset_input_state_fudging();
    _t3f_input_initialized = true;
  }
  return true;
}

void t3f_deinitialize_input(void)
{
  if(_t3f_input_initialized)
  {
    _t3f_input_initialized = false;
  }
}

T3F_INPUT_HANDLER * t3f_create_input_handler(int type)
{
  T3F_INPUT_HANDLER * input_handler = NULL;

  input_handler = malloc(sizeof(T3F_INPUT_HANDLER));
  if(!input_handler)
  {
    goto fail;
  }
  memset(input_handler, 0, sizeof(T3F_INPUT_HANDLER));

  input_handler->type = type;
  switch(input_handler->type)
  {
    case T3F_INPUT_HANDLER_TYPE_GAMEPAD:
    {
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Analog Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Left
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Right
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Up
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // D-Pad Down
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Left Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS); // Right Trigger
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // A
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // B
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // X
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Y
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // L3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // R3
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Select
      t3f_add_input_handler_element(input_handler, T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON); // Start
      break;
    }
  }
  return input_handler;

  fail:
  {
    if(input_handler)
    {
      free(input_handler);
    }
    return NULL;
  }
}

void t3f_destroy_input_handler(T3F_INPUT_HANDLER * input_handler)
{
  free(input_handler);
}

bool t3f_add_input_handler_element(T3F_INPUT_HANDLER * input_handler, int type)
{
  T3F_INPUT_HANDLER_ELEMENT * element = NULL;
  int i;

  element = malloc(sizeof(T3F_INPUT_HANDLER_ELEMENT) * (input_handler->elements + 1));
  if(!element)
  {
    goto fail;
  }
  for(i = 0; i < input_handler->elements; i++)
  {
    memcpy(&element[i], &input_handler->element[i], sizeof(T3F_INPUT_HANDLER_ELEMENT));
  }
  memset(&element[i], 0, sizeof(T3F_INPUT_HANDLER_ELEMENT));
  element[i].type = type;
  free(input_handler->element);
  input_handler->element = element;
  input_handler->elements++;

  return true;

  fail:
  {
    if(element)
    {
      free(element);
    }
    return false;
  }
}

void t3f_bind_input_handler_element(T3F_INPUT_HANDLER * input_handler, int element, int device_type, int device_number, int device_element)
{
  input_handler->element[element].device_type = device_type;
  input_handler->element[element].device_number = device_number;
  input_handler->element[element].device_element = device_element;
}

bool t3f_map_input_for_xbox_controller(T3F_INPUT_HANDLER * input_handler, int joystick)
{
  if(input_handler->type != T3F_INPUT_HANDLER_TYPE_GAMEPAD)
  {
    return false;
  }

  /* Left Analog X */
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].dead_zone = 0.15;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].threshold = 0.5;

  /* Left Analog Y */
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].dead_zone = 0.15;
  input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].threshold = 0.5;

  /* Right Analog X */
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].dead_zone = 0.15;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].threshold = 0.5;

  /* Right Analog Y */
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].dead_zone = 0.15;
  input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].threshold = 0.5;

  /* D-Pad Left */
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_LEFT].threshold = 0.1;

  /* D-Pad Right */
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].threshold = 0.1;

  /* D-Pad Up */
  input_handler->element[T3F_GAMEPAD_DPAD_UP].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_UP].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_UP].threshold = 0.1;

  /* D-Pad Down */
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_DPAD_DOWN].threshold = 0.1;

  /* Left Trigger */
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].threshold = 0.5;
//  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].scale = 0.5;
//  input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].offset = 1.0;

  /* Right Trigger */
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].dead_zone = 0.0;
  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].threshold = 0.5;
//  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].scale = 0.5;
//  input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].offset = 1.0;

  /* A */
  input_handler->element[T3F_GAMEPAD_A].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* B */
  input_handler->element[T3F_GAMEPAD_B].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* X */
  input_handler->element[T3F_GAMEPAD_X].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* Y */
  input_handler->element[T3F_GAMEPAD_Y].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* L */
  input_handler->element[T3F_GAMEPAD_L].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* R */
  input_handler->element[T3F_GAMEPAD_R].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* L3 */
  input_handler->element[T3F_GAMEPAD_L3].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* R3 */
  input_handler->element[T3F_GAMEPAD_R3].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* Start */
  input_handler->element[T3F_GAMEPAD_START].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  /* Select */
  input_handler->element[T3F_GAMEPAD_SELECT].device_type = T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK;

  #ifdef ALLEGRO_MACOSX

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_element = 2;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_element = 3;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_element = 4;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].device_element = 8;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].device_element = 9;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].device_element = 10;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].device_element = 11;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].device_element = 12;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].device_element = 13;

    /* L3 */
    input_handler->element[T3F_GAMEPAD_L3].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L3].device_element = 16;

    /* R3 */
    input_handler->element[T3F_GAMEPAD_R3].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R3].device_element = 17;

    /* Start */
    input_handler->element[T3F_GAMEPAD_START].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_START].device_element = 14;

    /* Select */
    input_handler->element[T3F_GAMEPAD_SELECT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_SELECT].device_element = 15;

    return true;

  #endif

  #ifdef ALLEGRO_WINDOWS

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_element = 2;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_element = 3;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element = 17;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element = 16;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element = 19;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element = 18;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_element = 4;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].device_element = 6;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].device_element = 7;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].device_element = 8;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].device_element = 9;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].device_element = 11;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].device_element = 10;

    /* L3 */
    input_handler->element[T3F_GAMEPAD_L3].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L3].device_element = 13;

    /* R3 */
    input_handler->element[T3F_GAMEPAD_R3].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R3].device_element = 12;

    /* Start */
    input_handler->element[T3F_GAMEPAD_START].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_START].device_element = 14;

    /* Select */
    input_handler->element[T3F_GAMEPAD_SELECT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_SELECT].device_element = 15;

    return true;

  #endif

  #ifdef ALLEGRO_UNIX

    /* Left Analog X */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_X].device_element = 0;

    /* Left Analog Y */
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_ANALOG_Y].device_element = 1;

    /* Right Analog X */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_X].device_element = 3;

    /* Right Analog Y */
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_ANALOG_Y].device_element = 4;

    /* D-Pad Left */
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_LEFT].device_element_dir = -1.0;

    /* D-Pad Right */
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element = 6;
    input_handler->element[T3F_GAMEPAD_DPAD_RIGHT].device_element_dir = 1.0;

    /* D-Pad Up */
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_UP].device_element_dir = -1.0;

    /* D-Pad Down */
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element = 7;
    input_handler->element[T3F_GAMEPAD_DPAD_DOWN].device_element_dir = 1.0;

    /* Left Trigger */
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_LEFT_TRIGGER].device_element = 2;

    /* Right Trigger */
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_RIGHT_TRIGGER].device_element = 5;

    /* A */
    input_handler->element[T3F_GAMEPAD_A].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_A].device_element = 8;

    /* B */
    input_handler->element[T3F_GAMEPAD_B].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_B].device_element = 9;

    /* X */
    input_handler->element[T3F_GAMEPAD_X].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_X].device_element = 10;

    /* Y */
    input_handler->element[T3F_GAMEPAD_Y].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_Y].device_element = 11;

    /* L */
    input_handler->element[T3F_GAMEPAD_L].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_L].device_element = 12;

    /* R */
    input_handler->element[T3F_GAMEPAD_R].device_number = joystick;
    input_handler->element[T3F_GAMEPAD_R].device_element = 13;

    /* wired version */
    if(al_get_joystick_num_buttons(t3f_joystick[joystick]) > 10)
    {
      /* L3 */
      input_handler->element[T3F_GAMEPAD_L3].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_L3].device_element = 17;

      /* R3 */
      input_handler->element[T3F_GAMEPAD_R3].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_R3].device_element = 18;

      /* Start */
      input_handler->element[T3F_GAMEPAD_START].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_START].device_element = 15;

      /* Select */
      input_handler->element[T3F_GAMEPAD_SELECT].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_SELECT].device_element = 14;
    }

    /* wireless version */
    else
    {
      /* L3 */
      input_handler->element[T3F_GAMEPAD_L3].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_L3].device_element = 16;

      /* R3 */
      input_handler->element[T3F_GAMEPAD_R3].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_R3].device_element = 17;

      /* Start */
      input_handler->element[T3F_GAMEPAD_START].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_START].device_element = 15;

      /* Select */
      input_handler->element[T3F_GAMEPAD_SELECT].device_number = joystick;
      input_handler->element[T3F_GAMEPAD_SELECT].device_element = 14;
    }

    return true;

  #endif

}

static void update_input_device(int device)
{
  #ifdef ALLEGRO_MACOSX
    if(t3f_joystick_state_updated[device])
    {
      if(_input_state_fudging_helper[device].stick[4].axis[0] < 100.0 || _input_state_fudging_helper[device].stick[4].axis[1] < 100.0)
      {
        /* up */
        if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0 && t3f_joystick_state[device].stick[4].axis[1] <= -1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = 0.0;
          t3f_joystick_state[device].stick[4].axis[1] = -1.0;
        }

        /* down */
        else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0 && t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = 0.0;
          t3f_joystick_state[device].stick[4].axis[1] = 1.0;
        }

        /* left */
        else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0 && t3f_joystick_state[device].stick[4].axis[1] <= -1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = -1.0;
          t3f_joystick_state[device].stick[4].axis[1] = 0.0;
        }

        /* right */
        else if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0 && t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = 1.0;
          t3f_joystick_state[device].stick[4].axis[1] = 0.0;
        }

        /* up-right */
        else if(t3f_joystick_state[device].stick[4].axis[0] >= 1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = 1.0;
          t3f_joystick_state[device].stick[4].axis[1] = -1.0;
        }

        /* down-right */
        else if(t3f_joystick_state[device].stick[4].axis[1] >= 1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = 1.0;
          t3f_joystick_state[device].stick[4].axis[1] = 1.0;
        }

        /* down-left */
        else if(t3f_joystick_state[device].stick[4].axis[0] <= -1.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = -1.0;
          t3f_joystick_state[device].stick[4].axis[1] = 1.0;
        }

        /* up-left */
        else if(t3f_joystick_state[device].stick[4].axis[0] == 0.0 && t3f_joystick_state[device].stick[4].axis[1] == 0.0)
        {
          t3f_joystick_state[device].stick[4].axis[0] = -1.0;
          t3f_joystick_state[device].stick[4].axis[1] = -1.0;
        }

        /* idle */
        else
        {
          t3f_joystick_state[device].stick[4].axis[0] = 0.0;
          t3f_joystick_state[device].stick[4].axis[1] = 0.0;
        }
      }

      /* triggers */
      if(_input_state_fudging_helper[device].stick[2].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[2].axis[0] = (t3f_joystick_state[device].stick[2].axis[0] + 1.0) / 2.0;
      }
      if(_input_state_fudging_helper[device].stick[3].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[3].axis[0] = (t3f_joystick_state[device].stick[3].axis[0] + 1.0) / 2.0;
      }

      t3f_joystick_state_updated[device] = false;
    }
    return;
  #endif

  #ifdef ALLEGRO_UNIX
    if(t3f_joystick_state_updated[device])
    {
      /* triggers */
      if(_input_state_fudging_helper[device].stick[1].axis[0] < 100.0)
      {
        t3f_joystick_state[device].stick[1].axis[0] = (t3f_joystick_state[device].stick[1].axis[0] + 1.0) / 2.0;
      }
      if(_input_state_fudging_helper[device].stick[2].axis[1] < 100.0)
      {
        t3f_joystick_state[device].stick[2].axis[1] = (t3f_joystick_state[device].stick[2].axis[1] + 1.0) / 2.0;
      }

      t3f_joystick_state_updated[device] = false;
    }
    return;
  #endif
}

static void update_input_handler_element_state_keyboard(T3F_INPUT_HANDLER_ELEMENT * element)
{
  if(t3f_key[element->device_element])
  {
    element->held = true;
    element->released = false;
    if(!element->pressed)
    {
      element->pressed = true;
    }
    else
    {
      element->pressed = false;
    }
  }
  else
  {
    element->held = false;
    element->pressed = false;
    if(!element->released)
    {
      element->released = true;
    }
    else
    {
      element->released = false;
    }
  }
}

static void update_input_handler_element_state_mouse(T3F_INPUT_HANDLER_ELEMENT * element)
{
  int button;

  switch(element->device_element)
  {
    case 0:
    {
      element->val = t3f_mouse_x;
      break;
    }
    case 1:
    {
      element->val = t3f_mouse_y;
      break;
    }
    case 2:
    {
      element->val = t3f_mouse_z;
      break;
    }
    case 3: // placeholder for mouse_w
    {
      element->val = 0;
    }
    default:
    {
      button = element->device_element - 4;
      if(button >= 0)
      {
        if(t3f_mouse_button[button])
        {
          element->held = true;
          element->released = false;
          if(!element->pressed)
          {
            element->pressed = true;
          }
          else
          {
            element->pressed = false;
          }
        }
        else
        {
          element->held = false;
          element->pressed = false;
          if(!element->released)
          {
            element->released = true;
          }
          else
          {
            element->released = false;
          }
        }
      }
      break;
    }
  }
}

static void update_input_handler_element_state_touch(T3F_INPUT_HANDLER_ELEMENT * element)
{
}

static void update_input_handler_element_joystick_cache(T3F_INPUT_HANDLER_ELEMENT * element)
{
  int i, j;

  if(element->stick_elements <= 0)
  {
    element->stick_elements = 0;
    for(i = 0; i < al_get_joystick_num_sticks(t3f_joystick[element->device_number]); i++)
    {
      for(j = 0; j < al_get_joystick_num_axes(t3f_joystick[element->device_number], i); j++)
      {
        element->stick[element->stick_elements] = i;
        element->axis[element->stick_elements] = j;
        element->stick_elements++;
      }
    }
  }
}

static void update_input_handler_element_state_joystick(T3F_INPUT_HANDLER_ELEMENT * element)
{
  float tval;
  float val;

  update_input_handler_element_joystick_cache(element);
  update_input_device(element->device_number);
  if(element->device_element < element->stick_elements)
  {
    val = (t3f_joystick_state[element->device_number].stick[element->stick[element->device_element]].axis[element->axis[element->device_element]] + element->offset) * (element->scale > 0.0 ? element->scale : 1.0);
    if(fabs(val) >= element->dead_zone)
    {
      element->val = val;
      if(element->val < 0.0)
      {
        element->val = (element->val + element->dead_zone) / (1.0 - element->dead_zone);
      }
      else
      {
        element->val = (element->val - element->dead_zone) / (1.0 - element->dead_zone);
      }
    }
    else
    {
      element->val = 0.0;
    }
    if(element->device_element_dir < 0.0)
    {
      tval = -element->val;
    }
    else if(element->device_element_dir > 0.0)
    {
      tval = element->val;
    }
    else
    {
      tval = fabs(element->val);
    }
    if(tval >= element->threshold)
    {
      if(!element->held)
      {
        element->pressed = true;
      }
      else
      {
        element->pressed = false;
      }
      element->held = true;
      element->released = false;
    }
    else
    {
      if(element->held)
      {
        element->released = true;
      }
      else
      {
        element->released = false;
      }
      element->held = false;
      element->pressed = false;
    }
  }
  else
  {
    if(t3f_joystick_state[element->device_number].button[element->device_element - element->stick_elements])
    {
      if(!element->held)
      {
        element->pressed = true;
      }
      else
      {
        element->pressed = false;
      }
      element->held = true;
      element->released = false;
    }
    else
    {
      if(element->held)
      {
        element->released = true;
      }
      else
      {
        element->released = false;
      }
      element->held = false;
      element->pressed = false;
    }
  }
}

static void update_input_handler_element_state(T3F_INPUT_HANDLER_ELEMENT * element)
{
  switch(element->device_type)
  {
    case T3F_INPUT_HANDLER_DEVICE_TYPE_KEYBOARD:
    {
      update_input_handler_element_state_keyboard(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE:
    {
      update_input_handler_element_state_mouse(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_TOUCH:
    {
      update_input_handler_element_state_touch(element);
      break;
    }
    case T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK:
    {
      update_input_handler_element_state_joystick(element);
      break;
    }
  }
}

void t3f_update_input_handler_state(T3F_INPUT_HANDLER * input_handler)
{
  int i;

  for(i = 0; i < input_handler->elements; i++)
  {
    update_input_handler_element_state(&input_handler->element[i]);
  }
}

void _t3f_input_handle_joystick_event(ALLEGRO_EVENT * event)
{
  int joy_num = -1;

  if(event->type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION)
  {
    _reset_input_state_fudging();
  }

  #ifdef ALLEGRO_MACOSX
    if(event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
    {
      if(event->joystick.stick == 4)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[4].axis[0] = 0.0;
          _input_state_fudging_helper[joy_num].stick[4].axis[1] = 0.0;
        }
      }
      else if(event->joystick.stick == 2)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[2].axis[0] = 0.0;
        }
      }
      else if(event->joystick.stick == 3)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[3].axis[0] = 0.0;
        }
      }
    }
    return;
  #endif

  #ifdef ALLEGRO_UNIX
    if(event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
    {
      if(event->joystick.stick == 1 && event->joystick.axis == 0)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[1].axis[0] = 0.0;
        }
      }
      else if(event->joystick.stick == 2 && event->joystick.axis == 1)
      {
        joy_num = t3f_get_joystick_number(event->joystick.id);
        if(joy_num >= 0)
        {
          _input_state_fudging_helper[joy_num].stick[2].axis[1] = 0.0;
        }
      }
    }
    return;
  #endif
}
