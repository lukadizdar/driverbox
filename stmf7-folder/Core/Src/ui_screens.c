#include "ui_screens.h"
#include "gamepad.h"
#include <stdbool.h>

static lv_obj_t *scr_home;
static lv_obj_t *scr_ets2;
static lv_obj_t *scr_ac;
static lv_obj_t *scr_beamng;

/* --- Navigation callbacks --- */
static void go_home(lv_event_t *e)   { (void)e; lv_screen_load(scr_home); }
static void go_ets2(lv_event_t *e)   { (void)e; lv_screen_load(scr_ets2); }
static void go_ac(lv_event_t *e)     { (void)e; lv_screen_load(scr_ac); }
static void go_beamng(lv_event_t *e) { (void)e; lv_screen_load(scr_beamng); }

/* --- Colors --- */
#define COL_GRAY      0x424242
#define COL_RED       0xC62828
#define COL_GREEN_BR  0x43A047
#define COL_GREEN_DK  0x1B5E20
#define COL_BLUE      0x1565C0

/* --- ETS2 button state --- */
static uint8_t hazard_on;
static uint8_t trailer_on;
static uint8_t lights_state;   /* 0=off, 1=parking, 2=brights */
static uint8_t wipers_state;   /* 0=off, 1=auto, 2=spd1, 3=spd2, 4=spd3 */

static lv_obj_t *btn_hazard_ref;
static lv_obj_t *btn_trailer_ref;
static lv_obj_t *btn_lights_ref;
static lv_obj_t *btn_wipers_ref;
static lv_obj_t *lbl_lights_ref;
static lv_obj_t *lbl_wipers_ref;

static lv_timer_t *hazard_blink_timer;
static bool hazard_blink_phase;

/* --- HID press/release: held while finger is down --- */
static void hid_pressed_cb(lv_event_t *e)
{
  uint8_t bit = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
  gamepad_set_button(bit, 1);
  gamepad_send_report();
}

static void hid_released_cb(lv_event_t *e)
{
  uint8_t bit = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
  gamepad_set_button(bit, 0);
  gamepad_send_report();
}

/* --- Hazard: toggle with red/green blink --- */
static void hazard_blink_cb(lv_timer_t *t)
{
  (void)t;
  hazard_blink_phase = !hazard_blink_phase;
  uint32_t col = hazard_blink_phase ? COL_RED : COL_GREEN_BR;
  lv_obj_set_style_bg_color(btn_hazard_ref, lv_color_hex(col), 0);
}

static void hazard_click_cb(lv_event_t *e)
{
  (void)e;
  hazard_on = !hazard_on;

  if (hazard_on) {
    hazard_blink_phase = false;
    lv_obj_set_style_bg_color(btn_hazard_ref, lv_color_hex(COL_RED), 0);
    hazard_blink_timer = lv_timer_create(hazard_blink_cb, 500, NULL);
  } else {
    if (hazard_blink_timer) {
      lv_timer_delete(hazard_blink_timer);
      hazard_blink_timer = NULL;
    }
    lv_obj_set_style_bg_color(btn_hazard_ref, lv_color_hex(COL_GRAY), 0);
  }
}

/* --- Trailer: toggle on/off --- */
static void trailer_click_cb(lv_event_t *e)
{
  (void)e;
  trailer_on = !trailer_on;
  lv_obj_set_style_bg_color(btn_trailer_ref,
      lv_color_hex(trailer_on ? COL_BLUE : COL_GRAY), 0);
}

/* --- Lights: 3-state cycle --- */
static void lights_click_cb(lv_event_t *e)
{
  (void)e;
  lights_state = (lights_state + 1) % 3;

  static const uint32_t colors[] = { COL_GRAY, COL_GREEN_BR, COL_GREEN_DK };
  static const char *labels[] = { "Lights", "Parking", "Brights" };

  lv_obj_set_style_bg_color(btn_lights_ref, lv_color_hex(colors[lights_state]), 0);
  lv_label_set_text(lbl_lights_ref, labels[lights_state]);
}

/* --- Wipers: 5-state cycle --- */
static void wipers_click_cb(lv_event_t *e)
{
  (void)e;
  wipers_state = (wipers_state + 1) % 5;

  static const uint32_t colors[] = { COL_GRAY, 0x0277BD, 0x01579B, 0x014A7F, 0x003355 };
  static const char *labels[] = { "Wipers", "Auto", "Spd 1", "Spd 2", "Spd 3" };

  lv_obj_set_style_bg_color(btn_wipers_ref, lv_color_hex(colors[wipers_state]), 0);
  lv_label_set_text(lbl_wipers_ref, labels[wipers_state]);
}

/* --- Helper: create a placeholder game screen --- */
static lv_obj_t *create_placeholder_screen(const char *title)
{
  lv_obj_t *scr = lv_obj_create(NULL);

  lv_obj_t *lbl = lv_label_create(scr);
  lv_label_set_text(lbl, title);
  lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *sub = lv_label_create(scr);
  lv_label_set_text(sub, "Coming soon...");
  lv_obj_center(sub);

  lv_obj_t *btn = lv_button_create(scr);
  lv_obj_set_size(btn, 80, 35);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_add_event_cb(btn, go_home, LV_EVENT_CLICKED, NULL);
  lv_obj_t *bl = lv_label_create(btn);
  lv_label_set_text(bl, "Back");
  lv_obj_center(bl);

  return scr;
}

/* --- Build home screen --- */
static void create_home_screen(void)
{
  scr_home = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_home, lv_color_hex(0x1A1A2E), 0);

  lv_obj_t *title = lv_label_create(scr_home);
  lv_label_set_text(title, "Select Game");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

  lv_obj_t *btn_ets2 = lv_button_create(scr_home);
  lv_obj_set_size(btn_ets2, 130, 50);
  lv_obj_align(btn_ets2, LV_ALIGN_CENTER, -150, 20);
  lv_obj_set_style_bg_color(btn_ets2, lv_color_hex(0x1565C0), 0);
  lv_obj_add_event_cb(btn_ets2, go_ets2, LV_EVENT_CLICKED, NULL);
  lv_obj_t *l1 = lv_label_create(btn_ets2);
  lv_label_set_text(l1, "ETS2");
  lv_obj_center(l1);

  lv_obj_t *btn_ac = lv_button_create(scr_home);
  lv_obj_set_size(btn_ac, 130, 50);
  lv_obj_align(btn_ac, LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_bg_color(btn_ac, lv_color_hex(0xC62828), 0);
  lv_obj_add_event_cb(btn_ac, go_ac, LV_EVENT_CLICKED, NULL);
  lv_obj_t *l2 = lv_label_create(btn_ac);
  lv_label_set_text(l2, "Assetto Corsa");
  lv_obj_center(l2);

  lv_obj_t *btn_beam = lv_button_create(scr_home);
  lv_obj_set_size(btn_beam, 130, 50);
  lv_obj_align(btn_beam, LV_ALIGN_CENTER, 150, 20);
  lv_obj_set_style_bg_color(btn_beam, lv_color_hex(0xF57F17), 0);
  lv_obj_add_event_cb(btn_beam, go_beamng, LV_EVENT_CLICKED, NULL);
  lv_obj_t *l3 = lv_label_create(btn_beam);
  lv_label_set_text(l3, "BeamNG");
  lv_obj_center(l3);
}

/* --- Helper: create an ETS2 stateful button --- */
static lv_obj_t *create_ets2_btn(lv_obj_t *parent, const char *text,
                                  int w, int h, lv_event_cb_t cb,
                                  uint8_t hid_bit, lv_obj_t **lbl_out)
{
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_bg_color(btn, lv_color_hex(COL_GRAY), 0);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btn, hid_pressed_cb, LV_EVENT_PRESSED,
                       (void *)(uintptr_t)hid_bit);
  lv_obj_add_event_cb(btn, hid_released_cb, LV_EVENT_RELEASED,
                       (void *)(uintptr_t)hid_bit);

  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, text);
  lv_obj_center(lbl);

  if (lbl_out) *lbl_out = lbl;
  return btn;
}

/* --- Build ETS2 dashboard --- */
static void create_ets2_screen(void)
{
  scr_ets2 = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_ets2, lv_color_hex(0x0D1B2A), 0);

  lv_obj_t *ets2_title = lv_label_create(scr_ets2);
  lv_label_set_text(ets2_title, "Euro Truck Simulator 2");
  lv_obj_set_style_text_color(ets2_title, lv_color_white(), 0);
  lv_obj_align(ets2_title, LV_ALIGN_TOP_MID, 0, 8);

  /* 2x2 grid of stateful buttons */
  btn_hazard_ref = create_ets2_btn(scr_ets2, "Hazard", 140, 55,
                                    hazard_click_cb, 0, NULL);
  lv_obj_align(btn_hazard_ref, LV_ALIGN_CENTER, -80, -30);

  btn_trailer_ref = create_ets2_btn(scr_ets2, "Trailer", 140, 55,
                                     trailer_click_cb, 1, NULL);
  lv_obj_align(btn_trailer_ref, LV_ALIGN_CENTER, 80, -30);

  btn_lights_ref = create_ets2_btn(scr_ets2, "Lights", 140, 55,
                                    lights_click_cb, 2, &lbl_lights_ref);
  lv_obj_align(btn_lights_ref, LV_ALIGN_CENTER, -80, 40);

  btn_wipers_ref = create_ets2_btn(scr_ets2, "Wipers", 140, 55,
                                    wipers_click_cb, 3, &lbl_wipers_ref);
  lv_obj_align(btn_wipers_ref, LV_ALIGN_CENTER, 80, 40);

  /* Back button */
  lv_obj_t *ets2_back = lv_button_create(scr_ets2);
  lv_obj_set_size(ets2_back, 60, 30);
  lv_obj_align(ets2_back, LV_ALIGN_TOP_LEFT, 10, 5);
  lv_obj_set_style_bg_color(ets2_back, lv_color_hex(0x37474F), 0);
  lv_obj_add_event_cb(ets2_back, go_home, LV_EVENT_CLICKED, NULL);
  lv_obj_t *ets2_back_lbl = lv_label_create(ets2_back);
  lv_label_set_text(ets2_back_lbl, LV_SYMBOL_LEFT);
  lv_obj_center(ets2_back_lbl);
}

/* === BeamNG screen === */

/* Image declarations (from Core/Src/images/) */
extern const lv_image_dsc_t img_haz1;   /* off state (white) */
extern const lv_image_dsc_t img_haz2;   /* on state (red) */

#define BEAMNG_BTN_COLS  4
#define BEAMNG_BTN_ROWS  3
#define BEAMNG_BTN_COUNT (BEAMNG_BTN_COLS * BEAMNG_BTN_ROWS)

/* Per-button state for BeamNG */
static uint8_t  beamng_toggle[BEAMNG_BTN_COUNT];
static lv_obj_t *beamng_img[BEAMNG_BTN_COUNT];
static lv_timer_t *beamng_blink_timer;
static bool beamng_blink_phase;

/* Blink all active hazard icons */
static void beamng_blink_cb(lv_timer_t *t)
{
  (void)t;
  beamng_blink_phase = !beamng_blink_phase;
  const lv_image_dsc_t *src = beamng_blink_phase ? &img_haz2 : &img_haz1;
  for (int i = 0; i < BEAMNG_BTN_COUNT; i++) {
    if (beamng_toggle[i]) {
      lv_image_set_src(beamng_img[i], src);
    }
  }
}

/* Ensure blink timer runs if any button is active */
static void beamng_update_blink_timer(void)
{
  bool any_active = false;
  for (int i = 0; i < BEAMNG_BTN_COUNT; i++) {
    if (beamng_toggle[i]) { any_active = true; break; }
  }

  if (any_active && !beamng_blink_timer) {
    beamng_blink_phase = false;
    beamng_blink_timer = lv_timer_create(beamng_blink_cb, 500, NULL);
  } else if (!any_active && beamng_blink_timer) {
    lv_timer_delete(beamng_blink_timer);
    beamng_blink_timer = NULL;
  }
}

static void beamng_btn_click_cb(lv_event_t *e)
{
  uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
  beamng_toggle[idx] = !beamng_toggle[idx];

  if (beamng_toggle[idx]) {
    lv_image_set_src(beamng_img[idx], &img_haz2);
  } else {
    lv_image_set_src(beamng_img[idx], &img_haz1);
  }

  beamng_update_blink_timer();
}

static void create_beamng_screen(void)
{
  scr_beamng = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr_beamng, lv_color_hex(0x1A1A1A), 0);

  lv_obj_t *title = lv_label_create(scr_beamng);
  lv_label_set_text(title, "BeamNG Drive");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 4);

  /* Back button */
  lv_obj_t *back = lv_button_create(scr_beamng);
  lv_obj_set_size(back, 60, 28);
  lv_obj_align(back, LV_ALIGN_TOP_LEFT, 5, 2);
  lv_obj_set_style_bg_color(back, lv_color_hex(0x37474F), 0);
  lv_obj_add_event_cb(back, go_home, LV_EVENT_CLICKED, NULL);
  lv_obj_t *bl = lv_label_create(back);
  lv_label_set_text(bl, LV_SYMBOL_LEFT);
  lv_obj_center(bl);

  /*
   * 3 rows x 4 cols grid of 64x64 icon buttons
   * Usable area: ~470 x 230 (after title bar)
   * Cell: 112 x 72, icon 64x64 centered, 10px gaps
   */
  const int start_x = 18;   /* left margin */
  const int start_y = 32;   /* below title */
  const int cell_w  = 112;
  const int cell_h  = 76;

  for (int i = 0; i < BEAMNG_BTN_COUNT; i++) {
    int col = i % BEAMNG_BTN_COLS;
    int row = i / BEAMNG_BTN_COLS;
    int cx = start_x + col * cell_w + cell_w / 2;
    int cy = start_y + row * cell_h + cell_h / 2;

    /* Clickable container (transparent, sized to icon) */
    lv_obj_t *btn = lv_obj_create(scr_beamng);
    lv_obj_set_size(btn, 72, 72);
    lv_obj_set_pos(btn, cx - 36, cy - 36);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, beamng_btn_click_cb, LV_EVENT_CLICKED,
                         (void *)(uintptr_t)i);
    /* HID: held while finger is down (only button 0 for now) */
    if (i == 0) {
      lv_obj_add_event_cb(btn, hid_pressed_cb, LV_EVENT_PRESSED,
                           (void *)(uintptr_t)0);
      lv_obj_add_event_cb(btn, hid_released_cb, LV_EVENT_RELEASED,
                           (void *)(uintptr_t)0);
    }

    /* Icon image */
    lv_obj_t *img = lv_image_create(btn);
    lv_image_set_src(img, &img_haz1);
    lv_obj_center(img);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);

    beamng_img[i] = img;
    beamng_toggle[i] = 0;
  }
}

/* --- Public API --- */
void ui_create_screens(void)
{
  create_home_screen();
  create_ets2_screen();
  scr_ac = create_placeholder_screen("Assetto Corsa");
  create_beamng_screen();
}

lv_obj_t *ui_get_home_screen(void)
{
  return scr_home;
}
