#include <zephyr.h>

#include "leds.h"

void main(void) {
  leds_init();

  while(true) {
    k_sleep(MSEC_PER_SEC);
  }
}
