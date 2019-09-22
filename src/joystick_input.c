// =============================================================================
// Button input
// =============================================================================
#include <zephyr.h>
#include <device.h>
#include <gpio.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(input);

#include "joystick_input.h"
#include "joystick_hid.h"

#define USE_DEBUG_JOYSTICK 0

#define INPUT_JOYSTICK_UP    0
#define INPUT_JOYSTICK_DOWN  1
#define INPUT_JOYSTICK_LEFT  2
#define INPUT_JOYSTICK_RIGHT 3
#define INPUT_JOYSTICK_A     4
#define INPUT_JOYSTICK_B     5
#define INPUT_JOYSTICK_C     6
#define INPUT_JOYSTICK_D     7

typedef struct button {
  u32_t pin;
  u32_t key;
  const char *name;
} button;

#define INPUT_DRV_NAME "GPIOC"

struct button buttons[] = {
  { pin: 4, key: INPUT_JOYSTICK_LEFT, name: "Left"  },
  { pin: 2, key: INPUT_JOYSTICK_UP, name: "Up" },
  { pin: 6, key: INPUT_JOYSTICK_DOWN, name: "Down" },
  { pin: 8, key: INPUT_JOYSTICK_RIGHT, name: "Right" },
  { pin: 3, key: INPUT_JOYSTICK_A, name: "A" },
  { pin: 5, key: INPUT_JOYSTICK_B, name: "B" },
  { pin: 7, key: INPUT_JOYSTICK_C, name: "C" },
  { pin: 9, key: INPUT_JOYSTICK_D, name: "D" },
};
static int numButtons = sizeof(buttons) / sizeof(button);

struct device *inputDev;

// id ignored for now - I only have one joystick anyway!
void joystick_input_read(int id, joystickState *state) {
  state->x=0;
  state->y=0;
  for(int i=0; i<numButtons; i++) {
    button *b = &buttons[i];
    if(true) {//pins & BIT(b->pin)) {
      u32_t val;
      gpio_pin_read(inputDev, b->pin, &val);
      switch(b->key) {
        case INPUT_JOYSTICK_A: state->b1 = val==0 ? 1 : 0; break;
        case INPUT_JOYSTICK_B: state->b2 = val==0 ? 1 : 0; break;
        case INPUT_JOYSTICK_C: state->b3 = val==0 ? 1 : 0; break;
        case INPUT_JOYSTICK_D: state->b4 = val==0 ? 1 : 0; break;
        case INPUT_JOYSTICK_UP: if(!val) state->y=127; break;
        case INPUT_JOYSTICK_DOWN: if(!val) state->y=-127; break;
        case INPUT_JOYSTICK_LEFT: if(!val) state->x=-127; break;
        case INPUT_JOYSTICK_RIGHT: if(!val) state->x=127; break;
      }
    //  LOG_INF("Button %d %s => %d", i, b->name, val);
    }
  }
}

#if USE_DEBUG_JOYSTICK
static int debugJoystickId=0;
static void joystick_debug_update_state(joystickState *state) {
  static s8_t x=0, y=0;
  static u8_t b=0;

  state->x=x++;
  state->y=y--;
  state->buttons = b++;
}

static void joystick_debug_emit(struct k_timer *timer) {
  ARG_UNUSED(timer);
  joystickState *state=joystick_hid_get_state(debugJoystickId);
  joystick_debug_update_state(state);
  joystick_post_update(debugJoystickId, state);
  debugJoystickId+=1;
  if(debugJoystickId==NUM_JOYSTICKS) debugJoystickId=0;
}

K_TIMER_DEFINE(joystick_debug_timer, joystick_debug_emit, NULL);
#endif

void joystick_input_init() {
#if USE_DEBUG_JOYSTICK
  k_timer_start(&joystick_debug_timer, K_MSEC(40), K_MSEC(40));
#else
  int ret;
  inputDev = device_get_binding(INPUT_DRV_NAME);
  if(!inputDev) {
    LOG_ERR("Cannot find input device %s", INPUT_DRV_NAME);
    return;
  }

  // Configure the input pins as inpts
  u32_t flags = GPIO_DIR_IN | GPIO_PUD_PULL_UP | GPIO_INT_DEBOUNCE;

  for(int i=0; i<numButtons; i++) {
    button *b = &buttons[i];
    ret = gpio_pin_configure(inputDev, b->pin,flags);
    if(ret) {
      LOG_ERR("Error configuring %s pin %d", INPUT_DRV_NAME, b->pin);
      return;
    }
  }
#endif
}
