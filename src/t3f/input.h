/* Architecture notes:

   - Availablity of device types for element binding depends on flags passed to
     't3f_initialize()'.
   - SteamInput functionality will depend on whether or not SteamWorks was
     successfully initialized.
   - SteamWorks initialization should happen in 't3f_initialize()' so we can
     determine whether or not to initialize Allegro's joystick subsystem.
   - Input configurations should be able to be imported/exported.
     Configurations should be tied to device types. We'll create a unique id
     system to attach to input devices for now until Allegro gets its own id
     system. Proposal: name + sticks + buttons.

*/

#ifndef T3F_INPUT_H
#define T3F_INPUT_H

#define T3F_INPUT_HANDLER_TYPE_GENERIC               0
#define T3F_INPUT_HANDLER_TYPE_GAMEPAD               1
#define T3F_INPUT_HANDLER_TYPE_MOUSE                 2

/* device types */
#define T3F_INPUT_HANDLER_DEVICE_TYPE_NONE           0
#define T3F_INPUT_HANDLER_DEVICE_TYPE_KEYBOARD       1
#define T3F_INPUT_HANDLER_DEVICE_TYPE_MOUSE          2
#define T3F_INPUT_HANDLER_DEVICE_TYPE_JOYSTICK       3

#define T3F_INPUT_HANDLER_ELEMENT_TYPE_BUTTON        0
#define T3F_INPUT_HANDLER_ELEMENT_TYPE_AXIS          1
#define T3F_INPUT_HANDLER_ELEMENT_TYPE_ABSOLUTE_AXIS 2

#define T3F_INPUT_HANDLER_MAX_BINDINGS               4

#define T3F_INPUT_DEFAULT_STICK_DEAD_ZONE         0.15
#define T3F_INPUT_DEFAULT_STICK_THRESHOLD          0.5

#define T3F_GAMEPAD_LEFT_ANALOG_X                    0
#define T3F_GAMEPAD_LEFT_ANALOG_Y                    1
#define T3F_GAMEPAD_RIGHT_ANALOG_X                   2
#define T3F_GAMEPAD_RIGHT_ANALOG_Y                   3
#define T3F_GAMEPAD_DPAD_LEFT                        4
#define T3F_GAMEPAD_DPAD_RIGHT                       5
#define T3F_GAMEPAD_DPAD_UP                          6
#define T3F_GAMEPAD_DPAD_DOWN                        7
#define T3F_GAMEPAD_LEFT_TRIGGER                     8
#define T3F_GAMEPAD_RIGHT_TRIGGER                    9
#define T3F_GAMEPAD_A                               10
#define T3F_GAMEPAD_B                               11
#define T3F_GAMEPAD_X                               12
#define T3F_GAMEPAD_Y                               13
#define T3F_GAMEPAD_L                               14
#define T3F_GAMEPAD_R                               15
#define T3F_GAMEPAD_L3                              16
#define T3F_GAMEPAD_R3                              17
#define T3F_GAMEPAD_SELECT                          18
#define T3F_GAMEPAD_START                           19
#define T3F_GAMEPAD_INPUT_END                       19

#define T3F_MOUSE_X                                  0
#define T3F_MOUSE_Y                                  1
#define T3F_MOUSE_Z                                  2
#define T3F_MOUSE_W                                  3
#define T3F_MOUSE_BUTTON_1                           4
#define T3F_MOUSE_BUTTON_2                           5
#define T3F_MOUSE_BUTTON_3                           6
#define T3F_MOUSE_BUTTON_4                           7
#define T3F_MOUSE_BUTTON_5                           8
#define T3F_MOUSE_BUTTON_6                           9
#define T3F_MOUSE_BUTTON_7                          10
#define T3F_MOUSE_BUTTON_8                          11
#define T3F_MOUSE_BUTTON_9                          12
#define T3F_MOUSE_BUTTON_10                         13
#define T3F_MOUSE_BUTTON_11                         14
#define T3F_MOUSE_BUTTON_12                         15
#define T3F_MOUSE_BUTTON_13                         16
#define T3F_MOUSE_BUTTON_14                         17
#define T3F_MOUSE_BUTTON_15                         18
#define T3F_MOUSE_BUTTON_16                         19
#define T3F_MOUSE_INPUT_END                         19

typedef struct
{

  int device_type;    // source device type
  int device_number;  // source device number
  int device_element; // source device element number
  float device_element_dir; // for mapping axes to buttons
  float dead_zone;    // dead zone for analog inputs
  float threshold;    // threshold for analog input to register pressed/released
  float offset;       // offset element values by this amount
  float scale;        // scale element values by this amount

  /* joystick data cache */
  int stick_elements;
  int stick[128];
  int axis[128];

} T3F_INPUT_HANDLER_BINDING;

/* define an input element */
typedef struct
{

  int type;           // input type

  /* binding */
  T3F_INPUT_HANDLER_BINDING binding[T3F_INPUT_HANDLER_MAX_BINDINGS];

  /* state */
  bool binding_held[T3F_INPUT_HANDLER_MAX_BINDINGS];
  bool held;
  bool pressed;
  bool released;
  float val;
  bool allow_fudge; // ignore initial state on fudged elements if false

} T3F_INPUT_HANDLER_ELEMENT;

typedef struct
{

  int type;
  T3F_INPUT_HANDLER_ELEMENT * element;
  int elements;

} T3F_INPUT_HANDLER;

bool t3f_initialize_input(int flags);
void t3f_deinitialize_input(void);

T3F_INPUT_HANDLER * t3f_create_input_handler(int type);
void t3f_destroy_input_handler(T3F_INPUT_HANDLER * input_handler);
bool t3f_add_input_handler_element(T3F_INPUT_HANDLER * input_handler, int type);
void t3f_bind_input_handler_element(T3F_INPUT_HANDLER * input_handler, int element, int slot, int device_type, int device_number, int device_element);
bool t3f_map_input_for_xbox_controller(T3F_INPUT_HANDLER * input_handler, int joystick);
bool t3f_map_input_for_mouse(T3F_INPUT_HANDLER * input_handler);

void t3f_update_input_handler_state(T3F_INPUT_HANDLER * input_handler);
void t3f_clear_input_handler_state(T3F_INPUT_HANDLER * input_handler);
bool t3f_input_held(T3F_INPUT_HANDLER * input_handler, int element);
bool t3f_input_pressed(T3F_INPUT_HANDLER * input_handler, int element);
bool t3f_use_input_press(T3F_INPUT_HANDLER * input_handler, int element);
bool t3f_input_released(T3F_INPUT_HANDLER * input_handler, int element);
bool t3f_use_input_release(T3F_INPUT_HANDLER * input_handler, int element);
float t3f_get_input_val(T3F_INPUT_HANDLER * input_handler, int element);

void _t3f_input_handle_joystick_event(ALLEGRO_EVENT * event);

#endif
