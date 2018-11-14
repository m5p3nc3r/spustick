// =============================================================================
// USB HID
// =============================================================================

#include <zephyr.h>
#include <init.h>
#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#include "joystick_hid.h"
#include "joystick_input.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(joystick_hid);
#define USE_DEBUG_JOYSTICK 0

#define INPUT_DATA          (0U << 0)
#define INPUT_CONSTANT      (1U << 0)
#define INPUT_ARRAY         (0U << 1)
#define INPUT_VARIABLE      (1U << 1)
#define INPUT_ABSOLUTE      (0U << 2)
#define INPUT_RELATIVE      (1U << 2)

#define JOYSTICK_REPORT(id)                   \
  HID_GI_USAGE_PAGE, USAGE_GEN_DESKTOP,       \
  HID_LI_USAGE, USAGE_GEN_DESKTOP_JOYSTICK,   \
  HID_MI_COLLECTION, COLLECTION_APPLICATION,  \
    HID_MI_COLLECTION, COLLECTION_PHYSICAL,   \
      HID_GI_REPORT_ID, id,                   \
                                              \
      HID_GI_USAGE_PAGE, USAGE_GEN_BUTTON,    \
      HID_LI_USAGE_MIN(1), 0x01,              \
      HID_LI_USAGE_MAX(1), 0x08,              \
      HID_GI_LOGICAL_MIN(1), 0x00,            \
      HID_GI_LOGICAL_MAX(1), 0x01,            \
      HID_GI_REPORT_SIZE, 1,                  \
      HID_GI_REPORT_COUNT, 8,                 \
      HID_MI_INPUT, INPUT_DATA | INPUT_VARIABLE | INPUT_ABSOLUTE, \
                                              \
      HID_GI_USAGE_PAGE, USAGE_GEN_DESKTOP,   \
      HID_LI_USAGE, USAGE_GEN_DESKTOP_X,      \
      HID_LI_USAGE, USAGE_GEN_DESKTOP_Y,      \
      HID_GI_LOGICAL_MIN(1), -127,            \
      HID_GI_LOGICAL_MAX(1), 127,             \
      HID_GI_REPORT_SIZE, 8,                  \
      HID_GI_REPORT_COUNT, 2,                 \
      HID_MI_INPUT, INPUT_DATA | INPUT_VARIABLE | INPUT_ABSOLUTE, \
    HID_MI_COLLECTION_END,                    \
  HID_MI_COLLECTION_END

static const u8_t hid_report[] = {
  JOYSTICK_REPORT(1), // Report ID 0 is invalid
#if NUM_JOYSTICKS>1
  JOYSTICK_REPORT(2)
#endif
};



void hid_worker_handler(struct k_work *work);
void hid_timer_handler(struct k_timer *timer);
K_WORK_DEFINE(hid_worker, hid_worker_handler);
K_TIMER_DEFINE(hid_timer, hid_timer_handler, NULL);


static int debug_cb(struct usb_setup_packet *setup, s32_t *len, u8_t **data)
{
	LOG_INF("Debug callback");

	return -ENOTSUP;
}

static int set_idle_cb(struct usb_setup_packet *setup, s32_t *len, u8_t **data)
{
  // This sets the time between packets - for a joystick, this should be 0
  LOG_INF("Set idle callback");

  k_timer_start(&hid_timer, K_MSEC(40), K_MSEC(40));

	return 0;
}

static int get_report_cb(struct usb_setup_packet *setup, s32_t *len, u8_t **data)
{
	LOG_INF("Get report callback");

  // Should never be called for a joystick, we are continually sending reports

	return 0;
}

static bool connected=false;

static void status_cb(enum usb_dc_status_code status, const u8_t *param)
{
  #define NOT_READY(STATE) LOG_INF("Status " #STATE " %d", param ? *param : -1); connected=false;
  #define READY(STATE) LOG_INF("Status " #STATE " %d", param ? *param : -1); connected=true;
	switch (status) {
    case USB_DC_ERROR: NOT_READY(USB_DC_ERROR); break;
    case USB_DC_RESET: NOT_READY(USB_DC_RESET); break;
    case USB_DC_CONNECTED: NOT_READY(USB_DC_CONNECTED); break;
    case USB_DC_CONFIGURED: READY(USB_DC_CONFIGURED); break;
    case USB_DC_DISCONNECTED: NOT_READY(USB_DC_DISCONNECTED); break;
    case USB_DC_SUSPEND: NOT_READY(USB_DC_SUSPEND); break;
    case USB_DC_RESUME: READY(USB_DC_RESUME); break;
    case USB_DC_UNKNOWN:
	  default:
		  LOG_INF("Unknonw status %d", status);
		  break;
	}
}


static const struct hid_ops ops = {
  .get_report = get_report_cb,
  .get_idle = debug_cb,
  .get_protocol = debug_cb,
  .set_report = debug_cb,
  .set_idle = set_idle_cb,
  .set_protocol = debug_cb,
  .status_cb = status_cb,
};

typedef struct {
  u8_t id;
  joystickState state;
} joystickReport;

static struct {
  joystickReport joystick[NUM_JOYSTICKS];
} hidReports;

#if USE_DEBUG_JOYSTICK
static void joystick_debug(joystickState *state) {
  static s8_t x=0, y=0;
  static u8_t b=0;

  state->x=x++;
  state->y=y--;
  state->buttons = b++;
}
#endif

void hid_worker_handler(struct k_work *work) {
  // Iterate over the joysticks and transmit the state
  for(int joystickId=0; joystickId<NUM_JOYSTICKS; joystickId++) {
    joystickReport *report = &hidReports.joystick[joystickId];
#if USE_DEBUG_JOYSTICK
    joystick_debug(&report->state);
#else
    joystick_input_read(joystickId, &report->state);
#endif
  // Transmit the state
    if(connected) {
      hid_int_ep_write((char *)report, sizeof(joystickReport), NULL);
    }
  }
}

void hid_timer_handler(struct k_timer *timer) {
  // Can't guarantee that the HID processing won't block, so use a worker
  k_work_submit(&hid_worker);
}

static int joystick_hid_init(struct device *dev)
{
  joystick_input_init();

	usb_hid_register_device(hid_report, sizeof(hid_report), &ops);
  // Initialise report ids
  for(int j=0; j<NUM_JOYSTICKS; j++) {
    hidReports.joystick[j].id = j+1;
  }

	return usb_hid_init();
}

SYS_INIT(joystick_hid_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
