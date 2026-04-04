#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void gamepad_set_button(uint8_t bit_index, uint8_t active);
void gamepad_send_report(void);

#ifdef __cplusplus
}
#endif

#endif /* __GAMEPAD_H__ */
