#ifndef __UI_SCREENS_H__
#define __UI_SCREENS_H__

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_create_screens(void);
lv_obj_t *ui_get_home_screen(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SCREENS_H__ */
