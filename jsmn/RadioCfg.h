#ifndef _RADIO_CONFIG_H_
#define _RADIO_CONFIG_H_

#include <stdio.h>
#include <string>
#include <vector>
#include "stdint.h"
#include "jsmn.h"

typedef struct
{
    uint64_t radioID;
    std::string name;
    
    uint16_t temp_pin;
    uint16_t humid_pin;
    uint16_t lumin_pin;
    uint16_t sprinkler_pin;    
    
    void debug()
    {
        printf("Slave ID : %lld\r\n", radioID);
        printf("Name : \"%s\"\r\n", name.c_str());
        printf("Temperature Pin: %d\r\n", temp_pin);
        printf("Humidity Pin: %d\r\n", humid_pin);
        printf("Luminance Pin: %d\r\n", lumin_pin);
        printf("Sprinkler Pin: %d\r\n", sprinkler_pin);
    }
} RadioSlave, *pRadioSlave;

typedef struct 
{
    uint64_t    radioID;
    std::string name;
    std::string country;
    int utc_offset;    
    std::vector<RadioSlave> radios;
    
    inline void debug()
    {
        printf("Radio ID: %lld\r\n", radioID);
        printf("Name : \"%s\"\r\n", name.c_str());
        printf("Country : \"%s\"\r\n", country.c_str());
        printf("UTC Offset: %d\r\n", utc_offset);        
        std::vector<RadioSlave>::iterator it;
        for(it = radios.begin(); it != radios.end(); it++)
        {
            (*it).debug();
        }
    }
    
} RadioCfg, *pRadioCfg;

int parseradioconfig(const char * jsonstring, RadioCfg &);

#endif
