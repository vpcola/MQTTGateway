#include "XbeeMonitor.h"
#include "XBeeLib.h"
#include "Utils.h"
#include "MQTTSManager.h"
#include "config.h"
#include "mbed.h"

using namespace XBeeLib;

XBee802 xbee(PTC15, PTC14, NC, NC, NC, 115200); // local xbee

static MemoryPool<RadioControlData, 4> mpool;
static Queue<RadioControlData, 4> mqueue;

void postRadioControl(RadioControlData &data)
{
    // push data to pool
    RadioControlData *message = mpool.alloc();
    // Simple copy
    *message = data;
    mqueue.put(message);    
}

// Callback functions
static void io_data_cb(const RemoteXBee802 & remote, const IOSample802 & data)
{
    RadioStatus radioStatus;
    const uint64_t remote_addr64 = remote.get_addr64();
    
    printf("\r\n============== Radio Callback ===================\r\n");
    printf("\r\nRemote Radio Address :[%08x:%08x] %llu\r\n",
          UINT64_HI32(remote_addr64),
          UINT64_LO32(remote_addr64),
          remote_addr64);

    SensorData sdata;    
   
    // Get data from the digital input pin (DIO2) Sprinkler
    DioVal dio2;
    radioStatus = data.get_dio(XBee802::DIO2_AD2, &dio2);
    if (radioStatus != Success) {
        printf("Getting data from digital pin DIO2 FAILED\r\n");
    } 


    // Get data from the humidity pin
    uint16_t humidity;
    radioStatus = data.get_adc( XBee802::DIO3_AD3, &humidity);
    if (radioStatus != Success) {
        printf("Getting data from analog pin AD3 FAILED\r\n" );
    } 
    
    // Get data from the temperature pin
    uint16_t temperature;
    radioStatus = data.get_adc( XBee802::DIO0_AD0, &temperature);
    if (radioStatus != Success) {
        printf("Temperature : (AD0 Not connected) FAILED\r\n");
    } 
    
    // Get data from the luminace pin
    uint16_t luminance;
    radioStatus = data.get_adc( XBee802::DIO1_AD1, &luminance);
    if (radioStatus != Success) {
        printf("Getting data from analog pin AD1 FAILED\r\n");
    }    
    
    sdata.deviceaddr  = remote_addr64;
    sdata.temperature = temperature;
    sdata.humidity = 1023 - humidity;
    sdata.luminance = luminance;
    // Assign the value to the sensor data
    sdata.sprinkler = dio2;
    
    sdata.debug();        
    postMQTTUpdate(sdata);
    printf("\r\n=================================================\r\n");    
}

uint64_t getXbeeId()
{
    return xbee.get_addr64();   
}

int initXbeeMonitor()
{
  
    /* Register the callback function */
    xbee.register_io_sample_cb(&io_data_cb);
    
    /* Initialize the xbee device */
    RadioStatus radioStatus = xbee.init();
    if (radioStatus != Success)
    {
        printf("Radio initialization failed!\r\n");
        return -1;
    }
    printf("XBee radios initialized ...\r\n");
    
    return 0;
}

int sendControlData(RadioControlData * data)
{
    RadioStatus radioStatus;
    const RemoteXBee802 remoteDevice = RemoteXBee802(data->radioID);
    if (data->sprinkler_pin == 0) // Turn on 
    {
        radioStatus = xbee.set_dio(remoteDevice, XBee802::DIO2_AD2, Low);
    }else
    {
        radioStatus = xbee.set_dio(remoteDevice, XBee802::DIO2_AD2, High);
    }
    
    return (radioStatus == Success);
}

void runXbeeMonitor()
{
    // Continually listen for arriving frames
    while(true)
    {
        osEvent evt = mqueue.get(100);
        if (evt.status == osEventMessage) 
        {
            // Unpack the message
            RadioControlData * message = (RadioControlData *)evt.value.p;

            // Send the packet, do not queue the call
            sendControlData(message);
            
            // Free the message from mempool  after using
            mpool.free(message);
        }
        
        xbee.process_rx_frames();
        // Thread::wait(100);
    }
}
