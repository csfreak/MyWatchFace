#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;
static TextLayer *s_time_layer, *s_weather_layer, *s_date_layer, *s_day_layer;

//static BitmapLayer *s_background_layer;
//static GBitmap *s_background_bitmap;

static BitmapLayer *s_bticon_layer;
static GBitmap *s_bticon_con_bitmap, *s_bticon_nc_bitmap;

static GFont s_time_font, s_other_font;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°F", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);  
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[12];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                                          "%H:%M:%S" : "%I:%M:%S", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
}
static void update_hourly() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  struct tm *utc_tick = gmtime(&temp);

  // Write the current date into a buffer
  static char s_date_buffer[12];
  strftime(s_date_buffer, sizeof(s_date_buffer),  "%m/%d/%Y", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);
  
  static char s_utc_buffer[2];
  strftime(s_utc_buffer, sizeof(s_utc_buffer), "%H", utc_tick);
  
  static char s_day_buffer[12];
  strftime(s_day_buffer, sizeof(s_day_buffer),  "%A", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if(tick_time->tm_min % 60 == 0) {
    update_hourly();
  }
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void bt_handler(bool connected) {
  if (connected) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is connected!");
    bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_con_bitmap);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is not connected!");
    bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_nc_bitmap);
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void battery_handler(BatteryChargeState charge_state) {
    APP_LOG(APP_LOG_LEVEL_INFO, "BatteryStateChange");
}
    

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_bticon_con_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BITMAP_BT_COLOR_CON);
  s_bticon_nc_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BITMAP_BT_COLOR_NC,RESOURCE_ID_BITMAP_BT_MONO_NC));

  // Create BitmapLayer to display the GBitmap
  s_bticon_layer = bitmap_layer_create(GRect(0, 137, 30, 30));

  // Set the bitmap onto the layer and add to the window
  if (connection_service_peek_pebble_app_connection()) {
      bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_con_bitmap);
  } else {
      bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_nc_bitmap);
  }
  
  
  bitmap_layer_set_compositing_mode(s_bticon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bticon_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, 64, bounds.size.w, 40)); 

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_28));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create temperature Layer
  s_weather_layer = text_layer_create(
      GRect(0, 0, bounds.size.w, 32));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");

  // Create second custom font, apply it and add to Window
  s_other_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_22));
  text_layer_set_font(s_weather_layer, s_other_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
   // Create date Layer
  s_date_layer = text_layer_create(
      GRect(0, 32, bounds.size.w, 32));

  // Style the text
  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "01/01/1970");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_date_layer, s_other_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
     // Create date Layer
  s_day_layer = text_layer_create(
      GRect(0, 104, bounds.size.w, 32));

  // Style the text
  text_layer_set_background_color(s_day_layer, GColorWhite);
  text_layer_set_text_color(s_day_layer, GColorBlack);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_text(s_day_layer, "Wednesday");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_day_layer, s_other_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  //gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  //bitmap_layer_destroy(s_background_layer);

  // Destroy weather elements
  fonts_unload_custom_font(s_other_font);
  connection_service_unsubscribe();
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();
  update_hourly();
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  //
  // Subscribe to get connection events
  connection_service_subscribe((ConnectionHandlers) { .pebble_app_connection_handler = bt_handler});
  // Subscribe to battery events
  battery_state_service_subscribe(battery_handler);
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}