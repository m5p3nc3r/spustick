#include <zephyr.h>

extern void leds_init(void);

void main(void) {
  leds_init();

  while(true) {
    k_sleep(MSEC_PER_SEC);
  }
}
