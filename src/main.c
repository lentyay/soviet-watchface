#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *outline_layer[4];
static Layer *main_layer;
static GBitmap *bitmap;
static GFont custom_font;
static GColor time_color;
static GColor outline_color;
int cur_plakat;
int coord_x;
int coord_y;
int get_poster(int count);

static void load_resources() {
  unsigned char RESOURCE[6] = {RESOURCE_ID_PLAKAT1, RESOURCE_ID_PLAKAT2, RESOURCE_ID_PLAKAT3, RESOURCE_ID_PLAKAT4, RESOURCE_ID_PLAKAT5, RESOURCE_ID_PLAKAT6};

  int posters_count = sizeof(RESOURCE);
  cur_plakat = get_poster(posters_count);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d %d", cur_plakat, resource_size(resource_get_handle(RESOURCE[cur_plakat])), sizeof(RESOURCE));

  bitmap = gbitmap_create_with_resource(RESOURCE[cur_plakat]);
  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OCTOBER_34));

  #ifdef PBL_BW
    time_color = GColorBlack;
    outline_color = GColorWhite;
  #elif PBL_COLOR
    // here is colors palette: //developer.pebble.com/docs/c/Graphics/Graphics_Types/Color_Definitions/
    static uint8_t time_colors[6] = {(uint8_t)0b11110000 /* Red */, (uint8_t)0b11110000 /* Red */, (uint8_t)0b11110000 /* Red */, (uint8_t)0b11111111 /* White */, (uint8_t)0b11111111 /* White */, (uint8_t)0b11100000 /* Dark Candy Apple Red */};
    time_color = (GColor)time_colors[cur_plakat];
    static uint8_t outline_colors[6] = {(uint8_t)0b11111111 /* White */, (uint8_t)0b11111110 /* Pastel Yellow */, (uint8_t)0b11111111 /* White */, (uint8_t)0b11110000 /* Red */, (uint8_t)0b11100100 /* Windsor Tan */, (uint8_t)0b11111110 /* Pastel Yellow */};
    outline_color = (GColor)outline_colors[cur_plakat];
  #endif  

  #ifdef PBL_ROUND
    coord_x = 25;
    coord_y = 128;
  #elif PBL_RECT
    coord_x = 5;
    coord_y = 129;
  #endif  
}

static void destroy_resources() {
  gbitmap_destroy(bitmap);
  fonts_unload_custom_font(custom_font);
}

int get_poster(int count) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Take 'poster_id' as last seconds digit
  int poster_id = (int)(tick_time->tm_sec % 10);
  
  // If 'poster_id' is higher than 'count' then pick 'poster_id' randomly
  if (poster_id > count - 1) { 
    poster_id = rand() % count;
  };
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d", poster_id, count);
  return(poster_id);
};

static void draw_picture(GContext* ctx, GBitmap **sources, GRect bounces, int number) {
    GPoint origin = bounces.origin;
    bounces.origin = GPoint(bounces.size.w*number, 0);
    GBitmap* temp = gbitmap_create_as_sub_bitmap(*sources, bounces);
    bounces.origin = origin;
    graphics_draw_bitmap_in_rect(ctx, temp, bounces);
    gbitmap_destroy(temp);
}

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

  // Kludge to handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  text_layer_set_text(text_layer, time_text);
  for (int i=0; i<4; ++i) {
    text_layer_set_text(outline_layer[i], time_text);
  }
}

static void layer_update(Layer *layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  draw_picture(ctx, &bitmap, GRect(0, 0, bounds.size.w, bounds.size.h), 0);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  load_resources();

  main_layer = layer_create(bounds);
  layer_set_update_proc(main_layer, layer_update);
  layer_add_child(window_layer, main_layer);

  text_layer = text_layer_create(GRect(coord_x, coord_y, 130, 50));
  text_layer_set_text_color(text_layer, time_color);

  GRect outline_layer_pos[4] = {GRect(coord_x + 2, coord_y + 2, 130, 50), GRect(coord_x + 2, coord_y - 2, 130, 50), GRect(coord_x - 2, coord_y + 2, 130, 50), GRect(coord_x - 2, coord_y - 2, 130, 50)};
  for (int i=0; i<4; ++i) {
    outline_layer[i] = text_layer_create(outline_layer_pos[i]);
    text_layer_set_text_color(outline_layer[i], outline_color);
  }

  for (int i=0; i<4; ++i) {
    text_layer_set_background_color(outline_layer[i], GColorClear);
    #ifdef PBL_ROUND
      text_layer_set_text_alignment(outline_layer[i], GTextAlignmentCenter);
    #elif PBL_RECT
      text_layer_set_text_alignment(outline_layer[i], GTextAlignmentLeft);
    #endif  
    text_layer_set_font(outline_layer[i], custom_font);
    layer_add_child(window_layer, text_layer_get_layer(outline_layer[i]));
  }    
    
  text_layer_set_background_color(text_layer, GColorClear);
  #ifdef PBL_ROUND
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  #elif PBL_RECT
    text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  #endif  
  text_layer_set_font(text_layer, custom_font);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
}

static void window_unload(Window *window) {
  destroy_resources();
  text_layer_destroy(text_layer);

  for (int i=0; i<4; ++i) {
    text_layer_destroy(outline_layer[i]);
  }

  layer_destroy(main_layer);
}


static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit() {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}