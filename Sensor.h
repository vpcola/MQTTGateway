#ifndef _SENSOR_H_
#define _SENSOR_H_


#include <stdio.h>
#include <stdint.h>
#include <string>
#include "Utils.h"

// Keep SensorData a POD type
typedef struct _SensorData
{
    uint64_t deviceaddr;
       
    uint16_t humidity;
    uint16_t temperature;
    uint16_t luminance;
    // Relay value (negative logic)
    // true = off
    // false = on
    bool  sprinkler;

    
    void debug()
    {
        printf("Channel id (lo): [%lX]\r\n", UINT64_HI32(deviceaddr));
        printf("Channel id (hi): [%lX]\r\n", UINT64_LO32(deviceaddr));
        printf("Humidity : [%d]\r\n", humidity);
        printf("Temperature : [%d]\r\n", temperature);
        printf("Luminance : [%d]\r\n", luminance);
    }    
} SensorData;

typedef struct _SensorInfo
{
    int id;
    std::string name;
} SensorInfo;

   
#endif