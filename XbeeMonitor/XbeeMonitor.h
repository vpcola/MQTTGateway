#ifndef _XBEE_MONITOR_H_
#define _XBEE_MONITOR_H_

#include <stdint.h>

typedef struct 
{
    uint64_t radioID;
    int sprinkler_pin;    
} RadioControlData, *pRadioControlData;

int initXbeeMonitor();
uint64_t getXbeeId();
void runXbeeMonitor();
void postRadioControl(RadioControlData &);

#endif
