#include <pebble.h>

#define CS_BATTERY_LEVEL_KEY 0xFFFE
#define CS_BATTERY_STATUS_KEY 0xFFFD
#define CS_STOCK_TICKER_KEY 0xFFEF
#define CS_STOCK_VALUE_KEY 0xFFEE
#define CS_WEATHER_TEMP_F_KEY 0xFFDF
#define CS_WEATHER_TEMP_C_KEY 0xFFDE
#define CS_WEATHER_COND_KEY 0xFFDD
#define CS_WEATHER_HUMID_KEY 0xFFDC
#define CS_WEATHER_WIND_SPEED_KEY 0xFFDB
#define CS_WEATHER_WIND_DIR_KEY 0xFFDA
#define CS_UPDATE_BATTERY_KEY 0x0FFF
#define CS_UPDATE_STOCK_KEY 0x0FFE
#define CS_UPDATE_WEATHER_KEY 0x0FFD

static Window *s_main_window;
static TextLayer *s_time_layer, *s_weather_layer, *s_date_layer, *s_day_layer, *s_stock_value_layer, *s_stock_ticker_layer;

//static BitmapLayer *s_background_layer;
//static GBitmap *s_background_bitmap;

static BitmapLayer *s_bticon_layer, *s_baticon_layer, *s_pbaticon_layer;
static GBitmap *s_bticon_con_bitmap, *s_bticon_nc_bitmap, *s_baticon_00_bitmap, 
    *s_baticon_10_bitmap, *s_baticon_20_bitmap, *s_baticon_30_bitmap, *s_baticon_40_bitmap,
	*s_baticon_50_bitmap, *s_baticon_60_bitmap, *s_baticon_70_bitmap, *s_baticon_80_bitmap,
	*s_baticon_90_bitmap, *s_baticon_100_bitmap;

static GFont s_time_font, s_weather_font, s_other_font;

static AppTimer *weatherHandle, *stockHandle;
static char temperature_buffer[8];
static char conditions_buffer[22];
static char weather_layer_buffer[32];
static char stock_value_buffer[22];
static char stock_ticker_buffer[22];



char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void battery_handler(BatteryChargeState charge_state) {
    static char level[5];
    snprintf(level, sizeof(level), "%d", (int)charge_state.charge_percent);
    APP_LOG(APP_LOG_LEVEL_INFO, "BatteryStateChange. Level = %s", level);
    
	switch ((int)charge_state.charge_percent) {
		case 0:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_00_bitmap);
			break;
		case 10:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_10_bitmap);
			break;
		case 20:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_20_bitmap);
			break;
		case 30:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_30_bitmap);
			break;
		case 40:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_40_bitmap);
			break;
		case 50:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_50_bitmap);
			break;
		case 60:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_60_bitmap);
			break;
		case 70:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_70_bitmap);
			break;
		case 80:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_80_bitmap);
			break;
		case 90:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_90_bitmap);
			break;
		case 100:
			bitmap_layer_set_bitmap(s_baticon_layer, s_baticon_100_bitmap);
			break;
		default:
			break;
	};
}

static void phone_battery_handler(int charge_level) {
    
    APP_LOG(APP_LOG_LEVEL_INFO, "PhoneBatteryStateChange. Level = %d", charge_level);
    
	switch ((int)charge_level) {
		case 0:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_00_bitmap);
			break;
		case 10:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_10_bitmap);
			break;
		case 20:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_20_bitmap);
			break;
		case 30:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_30_bitmap);
			break;
		case 40:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_40_bitmap);
			break;
		case 50:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_50_bitmap);
			break;
		case 60:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_60_bitmap);
			break;
		case 70:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_70_bitmap);
			break;
		case 80:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_80_bitmap);
			break;
		case 90:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_90_bitmap);
			break;
		case 100:
			bitmap_layer_set_bitmap(s_pbaticon_layer, s_baticon_100_bitmap);
			break;
		default:
			break;
	};
}    

static void sendUpdate(int key) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, key, 0);
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_INFO, "Sent %d", key);
}

static void updateWeather(void *data) {
	sendUpdate(CS_UPDATE_WEATHER_KEY);
	if (weatherHandle) {
		app_timer_cancel(weatherHandle); 
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather Timer Canceled");
	}
	weatherHandle = app_timer_register(900000, updateWeather, NULL);
	APP_LOG(APP_LOG_LEVEL_INFO, "Weather Timer Set");
}

static void updateBattery(void *data) {
	sendUpdate(CS_UPDATE_BATTERY_KEY);
}

static void updateStock(void *data) {
	sendUpdate(CS_UPDATE_STOCK_KEY);
	if (stockHandle) {
		app_timer_cancel(stockHandle); 
		APP_LOG(APP_LOG_LEVEL_INFO, "Stock Timer Canceled");
	}	
	stockHandle = app_timer_register(900000, updateStock, NULL);
	APP_LOG(APP_LOG_LEVEL_INFO, "Stock Timer Set");
	
}

static void sendUpdateAll() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, CS_UPDATE_BATTERY_KEY, 0);
    dict_write_uint8(iter, CS_UPDATE_WEATHER_KEY, 0);
    dict_write_uint8(iter, CS_UPDATE_STOCK_KEY, 0);
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_INFO, "Sent ALL");
    
    if (stockHandle) {
		app_timer_cancel(stockHandle); 
		APP_LOG(APP_LOG_LEVEL_INFO, "Stock Timer Canceled");
	}	
	stockHandle = app_timer_register(900000, updateStock, NULL);
	APP_LOG(APP_LOG_LEVEL_INFO, "Stock Timer Set");
	if (weatherHandle) {
		app_timer_cancel(weatherHandle); 
		APP_LOG(APP_LOG_LEVEL_INFO, "Weather Timer Canceled");
	}
	weatherHandle = app_timer_register(900000, updateWeather, NULL);
	APP_LOG(APP_LOG_LEVEL_INFO, "Weather Timer Set");
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	//Verify that Battery state shows correctly.
	battery_handler(battery_state_service_peek());
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Something");
	Tuple *tuple = dict_read_first(iterator);
	// Store incoming information

	while (tuple) {
 		switch (tuple->key) {
    		case CS_WEATHER_TEMP_F_KEY:
    			APP_LOG(APP_LOG_LEVEL_INFO, "Received Weather Data, Temp: %s", tuple->value->cstring);
     	 		snprintf(temperature_buffer, sizeof(temperature_buffer), "%s°F", tuple->value->cstring);
     	 		snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    			text_layer_set_text(s_weather_layer, weather_layer_buffer);
      			break;
    		case CS_WEATHER_COND_KEY:
    			APP_LOG(APP_LOG_LEVEL_INFO, "Received Weather Data, Condition: %s", tuple->value->cstring);
      			snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", tuple->value->cstring);
      			snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    			text_layer_set_text(s_weather_layer, weather_layer_buffer);
      			break;
      		case CS_WEATHER_TEMP_C_KEY:
      			APP_LOG(APP_LOG_LEVEL_INFO, "Received Weather Data, Temp C: %s", tuple->value->cstring);
      			break;
      		case CS_WEATHER_HUMID_KEY:
      			APP_LOG(APP_LOG_LEVEL_INFO, "Received Weather Data, Humidity: %s", tuple->value->cstring);
      			break;	
      		case CS_BATTERY_LEVEL_KEY:
      			APP_LOG(APP_LOG_LEVEL_INFO, "Received Battery Data, Level: %d", (int)tuple->value->int32*10);
				phone_battery_handler((int)tuple->value->int32*10);
				break;
			case CS_BATTERY_STATUS_KEY:
      			APP_LOG(APP_LOG_LEVEL_INFO, "Received Battery Data, STATUS: %d", (int)tuple->value->int8);
				break;
			case CS_STOCK_VALUE_KEY:
				APP_LOG(APP_LOG_LEVEL_INFO, "Received Stock Data, Value: %s", tuple->value->cstring);
				snprintf(stock_value_buffer, sizeof(stock_value_buffer), "%s", tuple->value->cstring);
    			text_layer_set_text(s_stock_value_layer, stock_value_buffer);
    			break;
    		case CS_STOCK_TICKER_KEY:
				APP_LOG(APP_LOG_LEVEL_INFO, "Received Stock Data, Ticker: %s", tuple->value->cstring);
				snprintf(stock_ticker_buffer, sizeof(stock_ticker_buffer), "%s", tuple->value->cstring);
    			text_layer_set_text(s_stock_ticker_layer, stock_ticker_buffer);
    			break;
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Received Unknown Key %d with value %s or %d", (int)tuple->key, tuple->value->cstring, (int)tuple->value->int32);
				break;
  		}
  		tuple = dict_read_next(iterator);
	}
	

  	 
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped: %s", translate_error(reason));
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %s", translate_error(reason));
  Tuple *tuple = dict_read_first(iterator);
  // Store incoming information
  sendUpdate(tuple->key);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Get a tm structure
  time_t temp = time(NULL);  
  struct tm *utc_tick = gmtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_time_buffer[12];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                                          "%H:%M:%S" : "%I:%M:%S", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
  
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

/*static void set_bat_icon_color() {
	s_baticon_00_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_00);
	s_baticon_10_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_10);
	s_baticon_20_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_20);
	s_baticon_30_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_30);
	s_baticon_40_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_40);
	s_baticon_50_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_50);
	s_baticon_60_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_60);
	s_baticon_70_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_70);
	s_baticon_80_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_80);
	s_baticon_90_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_90);
	s_baticon_100_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_COLOR_100);
}

static void set_bat_icon_mono() { 
	s_baticon_00_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_00);
	s_baticon_10_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_10);
	s_baticon_20_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_20);
	s_baticon_30_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_30);
	s_baticon_40_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_40);
	s_baticon_50_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_50);
	s_baticon_60_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_60);
	s_baticon_70_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_70);
	s_baticon_80_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_80);
	s_baticon_90_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_90);
	s_baticon_100_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAT_MONO_100);
}*/

static void set_bat_icon() {
	s_baticon_00_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_00,RESOURCE_ID_BAT_MONO_00));
	s_baticon_10_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_10,RESOURCE_ID_BAT_MONO_10));
	s_baticon_20_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_20,RESOURCE_ID_BAT_MONO_20));
	s_baticon_30_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_30,RESOURCE_ID_BAT_MONO_30));
	s_baticon_40_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_40,RESOURCE_ID_BAT_MONO_40));
	s_baticon_50_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_50,RESOURCE_ID_BAT_MONO_50));
	s_baticon_60_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_60,RESOURCE_ID_BAT_MONO_60));
	s_baticon_70_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_70,RESOURCE_ID_BAT_MONO_70));
	s_baticon_80_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_80,RESOURCE_ID_BAT_MONO_80));
	s_baticon_90_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_90,RESOURCE_ID_BAT_MONO_90));
	s_baticon_100_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BAT_COLOR_100,RESOURCE_ID_BAT_MONO_100));
}

static void drawDateTime(Layer *root) {
  
  // Create the TextLayer with specific bounds
  	s_time_layer = text_layer_create(GRect(0, 28, layer_get_bounds(root).size.w, 45)); 

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(root, text_layer_get_layer(s_time_layer));
  
    
   // Create date Layer
  s_date_layer = text_layer_create(
      GRect(0, 100, layer_get_bounds(root).size.w, 30));

  // Style the text
  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "01/01/1970");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_date_layer, s_other_font);
  layer_add_child(root, text_layer_get_layer(s_date_layer));
  
     // Create date Layer
  s_day_layer = text_layer_create(
      GRect(0, 70, layer_get_bounds(root).size.w, 30));

  // Style the text
  text_layer_set_background_color(s_day_layer, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_text(s_day_layer, "Wednesday");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_day_layer, s_other_font);
  layer_add_child(root, text_layer_get_layer(s_day_layer));


}
static void drawWeather(Layer *root) {

// Create temperature Layer
  s_weather_layer = text_layer_create(
      GRect(0, 0, layer_get_bounds(root).size.w, 28));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(root, text_layer_get_layer(s_weather_layer));
  
}
static void drawBattery(Layer *root) {

 	//Call function to set gbitmaps from resources based on screen color depth
  	set_bat_icon();
  	//old way didnt work.
  	//PBL_IF_COLOR_ELSE(set_bat_icon_color(), set_bat_icon_mono());
  
  	// Create BitmapLayer to display the GBitmap
  	s_baticon_layer = bitmap_layer_create(GRect(layer_get_bounds(root).size.w - 20, 137, 20, 30));
    s_pbaticon_layer = bitmap_layer_create(GRect(layer_get_bounds(root).size.w - 40, 137, 20, 30));


  	//use battery handler to set state on window draw
  	battery_handler(battery_state_service_peek());
  	//Draw Bitmap Layers

  	bitmap_layer_set_compositing_mode(s_baticon_layer, GCompOpSet);
  	layer_add_child(root, bitmap_layer_get_layer(s_baticon_layer));
	bitmap_layer_set_compositing_mode(s_pbaticon_layer, GCompOpSet);
  	layer_add_child(root, bitmap_layer_get_layer(s_pbaticon_layer));
}

static void drawBT(Layer *root) {
 	s_bticon_con_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BITMAP_BT_COLOR_CON);
  	s_bticon_nc_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_BITMAP_BT_COLOR_NC,RESOURCE_ID_BITMAP_BT_MONO_NC));
  
    s_bticon_layer = bitmap_layer_create(GRect(0, 137, 30, 30));
    
    if (connection_service_peek_pebble_app_connection()) {
      bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_con_bitmap);
  	} else {
      bitmap_layer_set_bitmap(s_bticon_layer, s_bticon_nc_bitmap);
  	}
  	
  	bitmap_layer_set_compositing_mode(s_bticon_layer, GCompOpSet);
  	layer_add_child(root, bitmap_layer_get_layer(s_bticon_layer));
  }
  	
static void drawStock(Layer *root) {
	 s_stock_value_layer = text_layer_create(
      GRect(38, 145, 60, 20));
    text_layer_set_background_color(s_stock_value_layer, GColorClear);
  	text_layer_set_text_color(s_stock_value_layer, GColorWhite);
  	text_layer_set_text_alignment(s_stock_value_layer, GTextAlignmentCenter);
  	text_layer_set_text(s_stock_value_layer, "XX.XX");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_stock_value_layer, s_weather_font);
  layer_add_child(root, text_layer_get_layer(s_stock_value_layer));
  
  	 s_stock_ticker_layer = text_layer_create(
      GRect(38, 130, 60, 18));
    text_layer_set_background_color(s_stock_ticker_layer, GColorClear);
  	text_layer_set_text_color(s_stock_ticker_layer, GColorWhite);
  	text_layer_set_text_alignment(s_stock_ticker_layer, GTextAlignmentCenter);
  	text_layer_set_text(s_stock_ticker_layer, "VSAT");

  // Create second custom font, apply it and add to Window
  text_layer_set_font(s_stock_ticker_layer, s_weather_font);
  layer_add_child(root, text_layer_get_layer(s_stock_ticker_layer));
  
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_32));
  s_other_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_18));
 
  drawBT(window_layer);
  drawBattery(window_layer);
  drawWeather(window_layer);
  drawDateTime(window_layer);
  drawStock(window_layer);

}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_stock_value_layer);
  text_layer_destroy(s_stock_ticker_layer);
  bitmap_layer_destroy(s_bticon_layer);
  bitmap_layer_destroy(s_baticon_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_bticon_con_bitmap);
  gbitmap_destroy(s_bticon_nc_bitmap);
  gbitmap_destroy(s_baticon_00_bitmap);
  gbitmap_destroy(s_baticon_10_bitmap);
  gbitmap_destroy(s_baticon_20_bitmap);
  gbitmap_destroy(s_baticon_30_bitmap);
  gbitmap_destroy(s_baticon_40_bitmap);
  gbitmap_destroy(s_baticon_50_bitmap);
  gbitmap_destroy(s_baticon_60_bitmap);
  gbitmap_destroy(s_baticon_70_bitmap);
  gbitmap_destroy(s_baticon_80_bitmap);
  gbitmap_destroy(s_baticon_90_bitmap);
  gbitmap_destroy(s_baticon_100_bitmap);
    
  // Destroy BitmapLayer
  //bitmap_layer_destroy(s_background_layer);

  // Destroy weather elements
  fonts_unload_custom_font(s_other_font);
  fonts_unload_custom_font(s_weather_font);

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

  // Update Everything Once
  sendUpdateAll();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_timer_cancel(weatherHandle);
  app_timer_cancel(stockHandle);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
