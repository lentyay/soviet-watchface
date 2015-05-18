#include "pebble.h"

static Window *s_main_window;
static TextLayer *text_layer;

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  text_layer_set_text(text_layer, time_text);
}

static void init() {
  s_main_window = window_create();
  window_stack_push(s_main_window, true);

  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);

  GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OCTOBER_40));

/*  

  text_layer_set_font(text_layer, custom_font);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer = text_layer_create(bounds);
  //text_layer_set_text(text_layer, "  Hello,\n  World!");
*/
  window_set_background_color(s_main_window, GColorBlack);
  text_layer = text_layer_create(GRect(0, 95, 144, 64));

  #ifdef PBL_PLATFORM_APLITE
    text_layer_set_text_color(text_layer, GColorWhite);
  #elif PBL_PLATFORM_BASALT
    text_layer_set_text_color(text_layer, GColorRed);
  #endif
  
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, custom_font);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);  
  
}

static void deinit() {
  text_layer_destroy(text_layer);
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}