#ifndef __INPUT_H
#define __INPUT_H

#include "joystick_hid.h"

void joystick_input_init();
void joystick_input_read(int id, joystickState *state);

#endif
