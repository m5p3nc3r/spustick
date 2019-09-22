#include <zephyr.h>
#include <device.h>
#include <led_strip.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ws2812_led);


/* ========================================
* Colour conversion code
* Code taken from https://stackoverflow.com/questions/1737726/how-to-perform-rgb-yuv-conversion-in-c-c
*/
 #define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

// RGB -> YCbCr
#define CRGB2Y(R, G, B) CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16)
#define CRGB2Cb(R, G, B) CLIP((36962 * (B - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)
#define CRGB2Cr(R, G, B) CLIP((46727 * (R - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)

// YCbCr -> RGB
#define CYCbCr2R(Y, Cb, Cr) CLIP( Y + ( 91881 * Cr >> 16 ) - 179 )
#define CYCbCr2G(Y, Cb, Cr) CLIP( Y - (( 22544 * Cb + 46793 * Cr ) >> 16) + 135)
#define CYCbCr2B(Y, Cb, Cr) CLIP( Y + (116129 * Cb >> 16 ) - 226 )



#define STRIP_DEV_NAME DT_INST_0_WORLDSEMI_WS2812_LABEL
#define STRIP_NUM_LEDS 2
#define DELAY_TIME K_MSEC(40)

//30

struct led_rgb strip_colors[STRIP_NUM_LEDS];

typedef struct _phase {
  u8_t val;
  s8_t step;
} phase;

void phase_init(phase *p, u8_t val, u8_t step) {
  p->val=val;
  p->step=step;
}

u8_t phase_step(phase *p) {
  s32_t val = (s32_t)p->val + p->step;
  if(val<0) {
    val = -val;
    p->step = -p->step;
  } else if(val>=255){
    val = 255 - (val - 255);
    p->step = -p->step;
  }
  p->val = (u8_t)val;
  return p->val;
}


static struct device *strip;
static phase red, green, blue;

void led_work_handler(struct k_work *work) {
  int err;
  err=led_strip_update_rgb(strip, &strip_colors[0], STRIP_NUM_LEDS);
  LOG_ERR("Led update %d",err);
}
K_WORK_DEFINE(led_work, led_work_handler);

static void led_emit(struct k_timer *timer) {
  ARG_UNUSED(timer);
  int err;

//  LOG_ERR("LED_EMIT %p", strip);

  for(int i=0; i<STRIP_NUM_LEDS; i++) {
    strip_colors[i].r=phase_step(&red);
    strip_colors[i].g=phase_step(&green);
    strip_colors[i].b=phase_step(&blue);
  }

  k_work_submit(&led_work);
}

K_TIMER_DEFINE(led_timer, led_emit, NULL);

void leds_init(void) {
	LOG_INF("Starting ws2812 code...");
  strip = device_get_binding(STRIP_DEV_NAME);
  if(!strip) {
    LOG_ERR("Unable to open led strip %s", STRIP_DEV_NAME);
    return;
  }
  LOG_ERR("Setup ok");

  phase_init(&red, 0, 1);
  phase_init(&green, 0, 2);
  phase_init(&blue, 0, 3);

  k_timer_start(&led_timer, K_MSEC(40), K_MSEC(40));
}
