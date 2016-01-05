#ifndef _sm_watchapp_h
#define _sm_watchapp_h

#define STRING_LENGTH 255
#define NUM_WEATHER_IMAGES	9

extern char sm_string_buffer;
extern char sm_weather_cond_str, sm_weather_temp_str;
extern int sm_weather_img, sm_batteryPercent, sm_batteryPblPercent;

extern char sm_calendar_date_str, sm_calendar_text_str;
extern char sm_music_artist_str1, sm_music_title_str1;

extern AppTimer *timerUpdateCalendar;
extern AppTimer *timerUpdateWeather;
extern AppTimer *timerUpdateMusic;


extern const int WEATHER_IMG_IDS[];

void sm_updateWeather(void *data);
void sm_updateCalendar(void *data);
void sm_updateMusic(void *data);
void sm_updateStock(void *data);
void sm_updateBattery(void *data);
void sm_open_app_message();
void sm_close_app_message();

#endif
