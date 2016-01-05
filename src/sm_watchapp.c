#include <pebble.h>
#include "globals.h"
#include "sm_watchapp.h"
#include "sm_callbacks.h"

char sm_string_buffer[STRING_LENGTH];
char sm_weather_cond_str[STRING_LENGTH], sm_weather_temp_str[5];
int sm_weather_img, sm_batteryPercent, sm_batteryPblPercent;

char sm_calendar_date_str[STRING_LENGTH], sm_calendar_text_str[STRING_LENGTH];
char sm_music_artist_str1[STRING_LENGTH], sm_music_title_str1[STRING_LENGTH];

AppTimer *timerUpdateCalendar = NULL;
AppTimer *timerUpdateWeather = NULL;
AppTimer *timerUpdateMusic = NULL;


const int WEATHER_IMG_IDS[] = {	
  RESOURCE_ID_IMAGE_SUN,
  RESOURCE_ID_IMAGE_RAIN,
  RESOURCE_ID_IMAGE_CLOUD,
  RESOURCE_ID_IMAGE_SUN_CLOUD,
  RESOURCE_ID_IMAGE_FOG,
  RESOURCE_ID_IMAGE_WIND,
  RESOURCE_ID_IMAGE_SNOW,
  RESOURCE_ID_IMAGE_THUNDER,
  RESOURCE_ID_IMAGE_DISCONNECT
};


static uint32_t s_sequence_number = 0xFFFFFFFE;

AppMessageResult sm_message_out_get(DictionaryIterator **iter_out) {
    AppMessageResult result = app_message_outbox_begin(iter_out);
    if(result != APP_MSG_OK) return result;
    dict_write_int32(*iter_out, SM_SEQUENCE_NUMBER_KEY, ++s_sequence_number);
    if(s_sequence_number == 0xFFFFFFFF) {
        s_sequence_number = 1;
    }
    return APP_MSG_OK;
}

void sendCommand(int key) {
	DictionaryIterator* iterout;
	sm_message_out_get(&iterout);
    if(!iterout) return;
	
	dict_write_int8(iterout, key, -1);
	app_message_outbox_send();
}

void sendCommandInt(int key, int param) {
	DictionaryIterator* iterout;
	sm_message_out_get(&iterout);
    if(!iterout) return;
	
	dict_write_int8(iterout, key, param);
	app_message_outbox_send();
}

void sm_updateWeather(void *data) {
	sendCommand(SM_STATUS_UPD_WEATHER_KEY);	
}

void sm_updateCalendar(void *data) {
	sendCommand(SM_STATUS_UPD_CAL_KEY);	
}

void sm_updateMusic(void *data) {
	sendCommand(SM_SONG_LENGTH_KEY);	
}


void sm_rcv(DictionaryIterator *received, void *context) {
	// Got a message callback
	Tuple *t;
	static bool doWeather = false, doStocks = false, doMusic = false, doBatt = false, doCal = false;

	t=dict_find(received, SM_WEATHER_COND_KEY); 
	if (t!=NULL) {
		memcpy(sm_weather_cond_str, t->value->cstring, strlen(t->value->cstring));
        sm_weather_cond_str[strlen(t->value->cstring)] = '\0';
        doWeather = true;
	}

	t=dict_find(received, SM_WEATHER_TEMP_KEY); 
	if (t!=NULL) {
		memcpy(sm_weather_temp_str, t->value->cstring, strlen(t->value->cstring));
        sm_weather_temp_str[strlen(t->value->cstring)] = '\0';
		doWeather = true;
	}

	t=dict_find(received, SM_WEATHER_ICON_KEY); 
	if (t!=NULL) {
		sm_weather_img = t->value->uint8;
		doWeather = true;	  	
	}

	t=dict_find(received, SM_COUNT_BATTERY_KEY); 
	if (t!=NULL) {
		sm_batteryPercent = t->value->uint8;
		doBatt = true;	
	}

	t=dict_find(received, SM_STATUS_CAL_TIME_KEY); 
	if (t!=NULL) {
		memcpy(sm_calendar_date_str, t->value->cstring, strlen(t->value->cstring));
        sm_calendar_date_str[strlen(t->value->cstring)] = '\0';	
        doCal = true;			
	}

	t=dict_find(received, SM_STATUS_CAL_TEXT_KEY); 
	if (t!=NULL) {
		memcpy(sm_calendar_text_str, t->value->cstring, strlen(t->value->cstring));
        sm_calendar_text_str[strlen(t->value->cstring)] = '\0';	
        doCal = true;
    }


	t=dict_find(received, SM_STATUS_MUS_ARTIST_KEY); 
	if (t!=NULL) {
		memcpy(sm_music_artist_str1, t->value->cstring, strlen(t->value->cstring));
        sm_music_artist_str1[strlen(t->value->cstring)] = '\0';
        doMusic = true;
	}

	t=dict_find(received, SM_STATUS_MUS_TITLE_KEY); 
	if (t!=NULL) {
		memcpy(sm_music_title_str1, t->value->cstring, strlen(t->value->cstring));
		sm_music_title_str1[strlen(t->value->cstring)] = '\0';
		doMusic = true;
	}


	t=dict_find(received, SM_STATUS_UPD_WEATHER_KEY); 
	if (t!=NULL) {
		int interval = t->value->int32 * 1000;

		if (timerUpdateWeather != NULL)
			app_timer_cancel(timerUpdateWeather);
		APP_LOG(APP_LOG_LEVEL_INFO, "Recieved Music Interval, %d" , interval);

		//timerUpdateWeather = app_timer_register(interval , sm_updateWeather, NULL);
	}

	t=dict_find(received, SM_STATUS_UPD_CAL_KEY); 
	if (t!=NULL) {
		int interval = t->value->int32 * 1000;

		if (timerUpdateCalendar != NULL)
			app_timer_cancel(timerUpdateCalendar);
		APP_LOG(APP_LOG_LEVEL_INFO, "Recieved Calendar Interval, %d" , interval);
		//timerUpdateCalendar = app_timer_register(interval , sm_updateCalendar, NULL);
	}

	t=dict_find(received, SM_SONG_LENGTH_KEY); 
	if (t!=NULL) {
		int interval = t->value->int32 * 1000;

		if (timerUpdateMusic != NULL)
			app_timer_cancel(timerUpdateMusic);
		APP_LOG(APP_LOG_LEVEL_INFO, "Recieved Music Interval, %d" , interval);
		//timerUpdateMusic = app_timer_register(interval , sm_updateMusic, NULL);

	}
	
	//Process Callbacks
	if (doCal) sm_calendar_callback();
	if (doWeather) sm_weather_callback();
	if (doBatt) sm_battery_callback();
	if (doMusic) sm_music_callback();
	if (doStocks) sm_stocks_callback();


}

void sm_open_app_message() {
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum() );
	app_message_register_inbox_received(sm_rcv);
}

void sm_close_app_message() {
  app_message_deregister_callbacks();
  //Add Cancel Timers Here
}
