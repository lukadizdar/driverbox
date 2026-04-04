#include "lv_port_indev.h"
#include "lvgl.h"
#include "i2c.h"

/* FT5336 I2C address: 0x70 is the 8-bit write address (0x38 << 1) */
#define FT5336_ADDR        0x70

#define FT5336_TD_STATUS   0x02
#define FT5336_P1_XH       0x03

/* Last raw coordinates for debug display */
volatile int16_t touch_raw_x = -1;
volatile int16_t touch_raw_y = -1;

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
  (void)indev;

  uint8_t touch_count = 0;

  if (HAL_I2C_Mem_Read(&hi2c3, FT5336_ADDR, FT5336_TD_STATUS, I2C_MEMADD_SIZE_8BIT,
                        &touch_count, 1, 10) != HAL_OK)
  {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  touch_count &= 0x0F;

  if (touch_count > 0)
  {
    uint8_t buf[4];

    if (HAL_I2C_Mem_Read(&hi2c3, FT5336_ADDR, FT5336_P1_XH, I2C_MEMADD_SIZE_8BIT,
                          buf, 4, 10) != HAL_OK)
    {
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }

    uint16_t raw_x = ((buf[0] & 0x0F) << 8) | buf[1];
    uint16_t raw_y = ((buf[2] & 0x0F) << 8) | buf[3];

    touch_raw_x = raw_x;
    touch_raw_y = raw_y;

    data->point.x = raw_y;
    data->point.y = raw_x;
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void lv_port_indev_init(void)
{
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_read_cb);
}
