#ifndef _MQTTS_MANAGER_H_
#define _MQTTS_MANAGER_H_


#include "MQTTThreadedClient.h"
#include "Sensor.h"


int mqttsInit(NetworkInterface * net, const char * pem);
void postMQTTUpdate(SensorData &msg);
int runMQTTS();

#endif
