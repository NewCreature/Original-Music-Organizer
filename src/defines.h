#ifndef OMO_DEFINES_H
#define OMO_DEFINES_H

#define OMO_MAX_MENUS    32
#define OMO_MENU_MAIN     0
#define OMO_MENU_FILE     1
#define OMO_MENU_PLAYBACK 2

#ifdef ALLEGRO_MACOSX
    #define OMO_KEY_CTRL (t3f_key[ALLEGRO_KEY_COMMAND])
#else
    #define OMO_KEY_CTRL (t3f_key[ALLEGRO_KEY_LCTRL] || t3f_key[ALLEGRO_KEY_RCTRL])
#endif

#define OMO_MAX_TAG_TYPES 16

#endif
