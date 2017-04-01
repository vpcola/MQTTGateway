#ifndef _WEATHER_INFO_H_
#define _WEATHER_INFO_H_

#include "jsmn.h"
#include <string>

typedef struct {
    
    std::string weather;
    std::string icon;
    
    float temperature;
    float pressure;
    float humidity;    
} WeatherInfo, *pWeatherInfo;

int parseweatherinfo(const char * jsonstring, WeatherInfo &);

#endif