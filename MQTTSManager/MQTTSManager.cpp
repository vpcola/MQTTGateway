#include "MQTTSManager.h"
#include "XbeeMonitor.h"
#include "Utils.h"
#include "jsmn.h"
#include <string>

using namespace MQTT;

#define MQTTS_PORT  8883

static const char * topic_update = "garden_update";
static const char * topic_listen = "garden_status";
static const char * hostname = "mqtt.mbedhacks.com";
static const char * clientID = "mbed-sample";
static const char * username = "mbedhacks";
static const char * password = "qwer123";

static MQTTThreadedClient * pmqtt = NULL;
Thread mqttThd(osPriorityNormal, DEFAULT_STACK_SIZE * 2);
RadioControlData postdata;
static char tempbuff[100];

static int jsoneq(const char * json, jsmntok_t * tok, const char * s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
            return 0;
    }
    return -1;    
}

void messageArrived(MessageData& md)
{
    int i, r;
    
    jsmn_parser p;
    jsmntok_t t[100];
        
    Message &message = md.message;
    printf("Arrived Callback 1 : qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload [%.*s]\r\n", message.payloadlen, (char*)message.payload);
    
    // handle payload
    const char * jsonstring = std::string((const char *) message.payload, message.payloadlen).c_str();
    
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonstring, strlen(jsonstring), t, sizeof(t)/sizeof(t[0]));
    
    uint64_t radio_id = 0;
    int sprinkler_pin = 1; // 0 - turn on sprinkler, 1 - off
    
    /* Top level element is an object */
    if ((r > 0) && (t[0].type == JSMN_OBJECT)) 
    { 
        /* Loop over all tokens */
        for (i = 1; i < r; i++)
        {
            if (jsoneq(jsonstring, &t[i], "radioid") == 0)
            {
                memset(tempbuff, 0, sizeof(tempbuff));                
                strncpy(tempbuff, jsonstring + t[i+1].start, t[i+1].end - t[i+1].start);
                radio_id = strtoull(&tempbuff[0], NULL, 0);
                i++;                
            }
            else if (jsoneq(jsonstring, &t[i], "sprinkler") == 0)
            {
                memset(tempbuff, 0, sizeof(tempbuff));
                strncpy(tempbuff, jsonstring + t[i+1].start, t[i+1].end - t[i+1].start);
                sprinkler_pin = strtoul(&tempbuff[0], NULL, 0);
                i++;
            }
            else
            {
                
            }
        }    
    }
    
    // TODO: Send the values to the XBeeMonitor thread
    printf("Radio ID: %llu\r\n", radio_id);
    printf("Sprinkler Pin : %d\r\n", sprinkler_pin);
    postdata.radioID = radio_id;
    postdata.sprinkler_pin = sprinkler_pin;
    postRadioControl(postdata);
}

int mqttsInit(NetworkInterface * net, const char * pem)
{
    pmqtt = new MQTTThreadedClient(net, pem);
    if (pmqtt == NULL) 
        return -1;
    
    MQTTPacket_connectData logindata = MQTTPacket_connectData_initializer;
    logindata.MQTTVersion = 3;
    logindata.clientID.cstring = (char *) clientID;
    logindata.username.cstring = (char *) username;
    logindata.password.cstring = (char *) password;
    
    pmqtt->setConnectionParameters(hostname, MQTTS_PORT, logindata);
    pmqtt->addTopicHandler(topic_listen, messageArrived);
    
    return 0;
}

void postMQTTUpdate(SensorData &msg)
{
    // Serialize data to json string ...
    if (pmqtt)
    {
        PubMessage message;
        message.qos = QOS0;
        message.id = 123;
        
        strcpy(&message.topic[0], topic_update);
        size_t numbytes = snprintf(&message.payload[0], MAX_MQTT_PAYLOAD_SIZE,
                "{\"radio\":%llu,\"status\":{\"sprinkler\":%d,\"humidity\":%d,\"temperature\":%d,\"luminance\":%d}}", 
                msg.deviceaddr,
                msg.sprinkler,
                msg.humidity,
                msg.temperature,
                msg.luminance);
        printf("[%s]\r\n", &message.payload[0]);
        message.payloadlen = numbytes;
        pmqtt->publish(message);
    }
}

int runMQTTS()
{
    if ( pmqtt && (mqttThd.start(mbed::callback(pmqtt, &MQTTThreadedClient::startListener)) != osOK ) )
        return -1;
    return 0;
}