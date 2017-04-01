#include "WeatherInfo.h"
#include <string.h>
#include <stdlib.h>

static int jsoneq(const char * json, jsmntok_t * tok, const char * s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
            return 0;
    }
    return -1;    
}

int parseweatherinfo(const char * weather, WeatherInfo & winfo)
{
    // Parse weather information
    jsmn_parser p;
    jsmntok_t t[128];

    jsmn_init(&p);
    int r = jsmn_parse(&p, weather, strlen(weather), t, sizeof(t)/sizeof(t[0]));
    // Top level must be an object
    if ((r > 0) && (t[0].type == JSMN_OBJECT)) 
    { 
        /* Loop over all keys of the root object */
        for (int i = 1; i < r; i++)
        {
            if (jsoneq(weather, &t[i], "description") == 0)
            {
                winfo.weather = std::string((const char *)(weather + t[i+1].start), (size_t) t[i+1].end - t[i+1].start);
                i++;
            }
            else if (jsoneq(weather, &t[i], "icon") == 0)
            {
                winfo.icon = std::string((const char *)(weather + t[i+1].start), (size_t) t[i+1].end - t[i+1].start);
                i++;
            }
            else if (jsoneq(weather, &t[i], "temp") == 0)
            {
                std::string tempstr((const char *)(weather + t[i+1].start), (size_t) t[i+1].end - t[i+1].start);
                winfo.temperature = strtod(tempstr.c_str(), NULL);
                i++;
            }
            else if (jsoneq(weather, &t[i], "humidity") == 0)
            {
                std::string humidstr((const char *)(weather + t[i+1].start), (size_t) t[i+1].end - t[i+1].start);
                winfo.humidity = strtod(humidstr.c_str(), NULL);
                i++;
            }            
            else if (jsoneq(weather, &t[i], "pressure") == 0)
            {
                std::string pressurestr((const char *)(weather + t[i+1].start), (size_t) t[i+1].end - t[i+1].start);
                winfo.pressure = strtod(pressurestr.c_str(), NULL);
                i++;
            }else
            {
                // discard the rest of the tokens
            }
        }
        return 0;
    }else
        return -1;
}
