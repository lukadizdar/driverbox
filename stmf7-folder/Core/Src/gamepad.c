#include "gamepad.h"
#include "usbd_custom_hid_if.h"

static uint16_t gamepad_buttons = 0;

void gamepad_set_button(uint8_t bit_index, uint8_t active)
{
  if (active) {
    gamepad_buttons |= (1U << bit_index);
  } else {
    gamepad_buttons &= ~(1U << bit_index);
  }
}

void gamepad_send_report(void)
{
  uint8_t report[5] = {
    0x01,                            /* Report ID */
    gamepad_buttons & 0xFF,          /* Buttons low byte */
    (gamepad_buttons >> 8) & 0xFF,   /* Buttons high byte */
    0x00,                            /* X axis (centered) */
    0x00,                            /* Y axis (centered) */
  };
  USBD_CUSTOM_HID_SendReport_FS(report, sizeof(report));
}
