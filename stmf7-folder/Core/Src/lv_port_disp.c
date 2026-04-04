#include "lv_port_disp.h"
#include "lvgl.h"
#include "ltdc.h"

#define DISP_HOR_RES 480
#define DISP_VER_RES 272

/* Two framebuffers in SDRAM for double-buffering */
static uint16_t fb1[DISP_HOR_RES * DISP_VER_RES] __attribute__((section(".sdram")));
static uint16_t fb2[DISP_HOR_RES * DISP_VER_RES] __attribute__((section(".sdram")));

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  (void)area;

  /* In direct mode with double-buffering, LVGL alternates between fb1 and fb2.
   * We just need to point LTDC to the buffer LVGL just finished rendering into. */
  HAL_LTDC_SetAddress(&hltdc, (uint32_t)px_map, 0);

  lv_display_flush_ready(disp);
}

void lv_port_disp_init(void)
{
  lv_display_t *disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
  lv_display_set_flush_cb(disp, flush_cb);
  lv_display_set_buffers(disp, fb1, fb2, sizeof(fb1), LV_DISPLAY_RENDER_MODE_DIRECT);
}
