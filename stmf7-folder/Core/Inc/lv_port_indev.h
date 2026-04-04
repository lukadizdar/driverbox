#ifndef __LV_PORT_INDEV_H__
#define __LV_PORT_INDEV_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile int16_t touch_raw_x;
extern volatile int16_t touch_raw_y;

void lv_port_indev_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __LV_PORT_INDEV_H__ */
