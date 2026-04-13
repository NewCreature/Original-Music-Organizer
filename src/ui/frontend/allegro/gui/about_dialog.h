#ifndef OMO_ABOUT_DIALOG_H
#define OMO_ABOUT_DIALOG_H

#include "ui.h"

bool omo_open_about_dialog(OMO_UI * uip, void * data);
void omo_close_about_dialog(OMO_UI * uip, void * data);
void omo_about_dialog_logic(void * data);

#endif
