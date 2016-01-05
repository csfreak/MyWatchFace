#ifndef _sm_watchapp_h
#define _sm_watchapp_h

#define STRING_LENGTH 255
#define NUM_WEATHER_IMAGES	9

char sm_string_buffer[STRING_LENGTH];
char sm_weather_cond_str[STRING_LENGTH], sm_weather_temp_str[5];
int sm_weather_img, sm_batteryPercent, sm_batteryPblPercent;

char sm_calendar_date_str[STRING_LENGTH], sm_calendar_text_str[STRING_LENGTH];
char sm_music_artist_str1[STRING_LENGTH], sm_music_title_str1[STRING_LENGTH];

AppTimer *timerUpdateCalendar;
AppTimer *timerUpdateWeather;
AppTimer *timerUpdateMusic;


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


void sm_updateWeather(void *data);
void sm_updateCalendar(void *data);
void sm_updateMusic(void *data);
void sm_updateStock(void *data);
void sm_updateBattery(void *data);
void sm_open_app_message();
void sm_close_app_message();

#endif
