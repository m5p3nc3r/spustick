#ifndef _joystick_hid_h
#define _joystick_hid_h

#define NUM_JOYSTICKS 2

typedef struct {
  // Keep this the same form as the Descriptor
  union {
    u8_t buttons;
    struct {
      u8_t b1:1;
      u8_t b2:1;
      u8_t b3:1;
      u8_t b4:1;
      u8_t b5:1;
      u8_t b6:1;
      u8_t b7:1;
      u8_t b8:1;
    };
  };
  s8_t x;
  s8_t y;
} joystickState;

void joystick_post_update(int id, const joystickState *state);
joystickState *joystick_hid_get_state(int id);


#endif
