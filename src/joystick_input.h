#ifndef __INPUT_H
#define __INPUT_H

#include "joystick_hid.h"

#define NUM_JOYSTICKS 1

void joystick_input_init();
void joystick_input_read(int id, joystickState *state);

#endif
