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
      HID_LI_USAGE_MAX(1), NUM_BUTTONS,              \
      HID_GI_LOGICAL_MIN(1), 0x00,            \
      HID_GI_LOGICAL_MAX(1), 0x01,            \
      HID_GI_REPORT_SIZE, 1,                  \
      HID_GI_REPORT_COUNT, NUM_BUTTONS,                 \
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
  JOYSTICK_REPORT(2),
#endif
};



void hid_worker_handler(struct k_work *work);
//void hid_timer_handler(struct k_timer *timer);
K_WORK_DEFINE(hid_worker, hid_worker_handler);
//K_TIMER_DEFINE(hid_timer, hid_timer_handler, NULL);

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
  .status_cb = status_cb,
};

typedef struct {
  u8_t id;
  joystickState state;
} joystickReport;

typedef struct {
  struct k_work work;
  joystickReport report;
} joystickWorker;
static joystickWorker joystickWorkers[NUM_JOYSTICKS];

// Not sure this is the right way to do this, but hacking this in here for now
static struct device *hid_dev;

void hid_worker_handler(struct k_work *work) {
  joystickWorker *worker=CONTAINER_OF(work, joystickWorker, work);

  hid_int_ep_write(hid_dev, (char *)&worker->report, sizeof(joystickReport), NULL);
}

void joystick_post_update(int id, const joystickState *state) {
  if(id<NUM_JOYSTICKS) {
    joystickWorker *worker=&joystickWorkers[id];
    // Copy over the state
    memcpy(&worker->report.state, state, sizeof(joystickState));
    // Post the work
    if(connected) {
      k_work_submit(&worker->work);
    }
  }
}

joystickState *joystick_hid_get_state(int id) {
  joystickState *ret=id<NUM_JOYSTICKS ? &joystickWorkers[id].report.state : NULL;
  return ret;
}

static int joystick_hid_init(struct device *dev)
{
	ARG_UNUSED(dev);

  // Initialise the joystick input interfaces
  joystick_input_init();

	// Initialise the HID output interfaces
	hid_dev = device_get_binding("HID_0");
	if(hid_dev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return -ENODEV;
	}

	usb_hid_register_device(hid_dev, hid_report, sizeof(hid_report), &ops);
  // Initialise report ids
  for(int j=0; j<NUM_JOYSTICKS; j++) {
    joystickWorkers[j].report.id = j+1;
    k_work_init(&joystickWorkers[j].work, hid_worker_handler);
  }

	return usb_hid_init(hid_dev);
}

SYS_INIT(joystick_hid_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
