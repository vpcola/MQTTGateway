/* mbed main.cpp to test adafruit 2.8" TFT LCD shiled w Touchscreen
 * Copyright (c) 2014 Motoo Tanaka @ Design Methodology Lab
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
 /* *
  * @note This program is derived from the SeeeStudioTFTv2 program.
  * @note Although both program share same ILI9341 TFT driver,
  * @note the touch sensor was not same with the Display I purchased from Akizuki.
  * @note http://akizukidenshi.com/catalog/g/gM-07747/
  * @note The touch sensor on the display is STMPE610,
  * @note so I hacked the minimum spi driver for it (polling mode only).
  */
/**
 * @note To make this work with FRDM-K64F
 * @note PTA0 must be disconnected from the swd clk by cutting J11.
 * @note But to re-active SWD you need to put jumper header to J11
 * @note so that it can be re-connected by a jumper.
 */

#include "mbed.h"
#include "rtos.h"
#include "XbeeMonitor.h"
#include "easy-connect.h"
#include "NTPClient.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "DownloadFile.h"
#include "MQTTSManager.h"
#include "WeatherInfo.h"
#include "RadioCfg.h"
#include "config.h"

#include <string>
#include <sstream>

#if defined(ENABLE_LOGGING)
#include "DigiLoggerMbedSerial.h"
using namespace DigiLog;
#endif

SDBlockDevice sd(PTE3, PTE1, PTE2, PTE4);
FATFileSystem fs("sd");
Serial pc(USBTX, USBRX, 115200);

Thread xbeeThd;


static const char * radioconfigfile = "/sd/radioconfig.json";

/* List of trusted root CA certificates
 * currently only "letsencrypt", the CA for mbedhacks.com
 *
 * To add more than one root, just concatenate them.
 *
 * TODO: Move this certificate file onto the SD card.
 */
static const char SSL_CA_PEM[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIFETCCA/mgAwIBAgISA2ktlb1Y6ap4GCH7dg3wS37XMA0GCSqGSIb3DQEBCwUA\n"
    "MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD\n"
    "ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0xNzAzMDkwMTQ4MDBaFw0x\n"
    "NzA2MDcwMTQ4MDBaMBgxFjAUBgNVBAMTDW1iZWRoYWNrcy5jb20wggEiMA0GCSqG\n"
    "SIb3DQEBAQUAA4IBDwAwggEKAoIBAQC4ppYHlH8lfB7lkWOjMSnOJGaLtCBfz57I\n"
    "VVOd1Rngsz7nE5fg3joa7lkazRY1ZqtuC2UloS+4LYoQZX4Z887dhdug/TPA4J1A\n"
    "GppA4xVCb2kUFODMjZ2r4pMLp+MjFFMBaHrL4cgx/n4aJUB+N9Z+HW0p2Yr5TsOQ\n"
    "ghIOPkNxFr2q6klm49+BMUbO98hAwFwsIISLf6IbHM93gx1ltqkvb55N87ZM1hYH\n"
    "fkq+J+YqjleiLaqRN2MVlNMNfy9MDbqM5uCyGiWGtq8eiQLaWpZkxnA2MC5zPsO/\n"
    "fzEWiVjn2uazlXZ5xZwiK22KMxVasqWMitvETtmPOl9mocRbLQdxAgMBAAGjggIh\n"
    "MIICHTAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUF\n"
    "BwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFCsgG+z1BTjrN3K+/tF0C4k818Yv\n"
    "MB8GA1UdIwQYMBaAFKhKamMEfd265tE5t6ZFZe/zqOyhMHAGCCsGAQUFBwEBBGQw\n"
    "YjAvBggrBgEFBQcwAYYjaHR0cDovL29jc3AuaW50LXgzLmxldHNlbmNyeXB0Lm9y\n"
    "Zy8wLwYIKwYBBQUHMAKGI2h0dHA6Ly9jZXJ0LmludC14My5sZXRzZW5jcnlwdC5v\n"
    "cmcvMCsGA1UdEQQkMCKCDW1iZWRoYWNrcy5jb22CEXd3dy5tYmVkaGFja3MuY29t\n"
    "MIH+BgNVHSAEgfYwgfMwCAYGZ4EMAQIBMIHmBgsrBgEEAYLfEwEBATCB1jAmBggr\n"
    "BgEFBQcCARYaaHR0cDovL2Nwcy5sZXRzZW5jcnlwdC5vcmcwgasGCCsGAQUFBwIC\n"
    "MIGeDIGbVGhpcyBDZXJ0aWZpY2F0ZSBtYXkgb25seSBiZSByZWxpZWQgdXBvbiBi\n"
    "eSBSZWx5aW5nIFBhcnRpZXMgYW5kIG9ubHkgaW4gYWNjb3JkYW5jZSB3aXRoIHRo\n"
    "ZSBDZXJ0aWZpY2F0ZSBQb2xpY3kgZm91bmQgYXQgaHR0cHM6Ly9sZXRzZW5jcnlw\n"
    "dC5vcmcvcmVwb3NpdG9yeS8wDQYJKoZIhvcNAQELBQADggEBABFH6YcvHh8foHeg\n"
    "NM7iR9HnYRqa5gSERcCtq6jm8PcTsAbsdQ/BNpIHK7AZSg2kk17kj+JFeyMuNJWq\n"
    "lmabV0dtzdC8ejp1d7hGb/HjuQ400th/QRayvyrDVzQPfCNyJ0C82Q2DFjeUgnqv\n"
    "oJMcV6i4ICW0boI7GUf7oeHCmrUEHKffAbeFvx3c85c39IHJEFa59UWj1linU/Tr\n"
    "g9i5AaSKB95d706u1XRA7WLV/Hu7yunhxEjlj33bfdifBb/ZLBd0LtrXPwtXi6E8\n"
    "r6obp+B+Ce89G7WEhdT9BX0ck1KTK+yP7uAC7tvvsiejxXOoCtVyBAumBJS7mRuv\n"
    "I5hmKgE=\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----";

const char * openweathermap_id = "1bfddd34faa0e0216769d688b0b0c743";


#define BASIC_AUTH_USER         "tinong"
#define BASIC_AUTH_PASSWORD     "tatay"

template <class T>
std::string to_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

void UpdateWeatherInfo(NetworkInterface* network)
{
    printf("Downloading weather information ...\r\n");
    DownloadFile * wInfo = new DownloadFile(network);
    if (wInfo)
    {
        std::string url = "http://api.openweathermap.org/data/2.5/weather?q=Singapore,sg&appid="; 
        url += openweathermap_id;
        printf("Getting weather from [%s]\r\n", url.c_str());
        HttpResponse* result = wInfo->get_file(url.c_str());
        if (result != NULL)
        {

            std::string content = result->get_body_as_string();
         
            printf("\r\n========= Weather Info =============\r\n");
            printf("%s\r\n", content.c_str());
            printf("==============================\r\n");
            
            const char * weather = content.c_str();
            WeatherInfo info;
            if ( parseweatherinfo(weather, info) == 0)
            {
                printf("Weather : %s\r\n", info.weather.c_str());
                printf("Weather Icon : %s\r\n", info.icon.c_str());
                printf("Current Temperature : %.2f\r\n", info.temperature);
                printf("Current Pressure : %.2f\r\n", info.pressure);
                printf("Current Humidity : %.2f\r\n", info.humidity);
        
                // TODO: process the information and send updates
                // to GUI                
                
            }
        }
            
        delete wInfo;
    }
}
    
int main()
{
    uint64_t radioID;
    int lasthour = 0;
    RadioCfg cfg;
    
    // Mount the SD card first!
    printf("Mounting the filesystem on \"/sd\". ");
    fs.mount(&sd);
    
#if defined(ENABLE_LOGGING)
    new DigiLoggerMbedSerial(&pc, LogLevelInfo);
#endif      
    pc.printf("Initializing XbeeMonitor ...\r\n");
    initXbeeMonitor();
    // TODO: get XBee radio id 
    // and request configuration settings
    // from the web.
    radioID = getXbeeId();
    cfg.radioID = radioID;
    pc.printf("Radio ID : %llu\r\n", radioID);
        
    pc.printf("Connecting to ethernet ...\r\n");
    // Get the network interface via 
    // easy connect
    NetworkInterface* network = easy_connect(true);
    if (!network) {
        return 1;
    }

    pc.printf("IP Address : %s\r\n", network->get_ip_address());
    pc.printf("MAC Address : %s\r\n", network->get_mac_address());
    
    time_t ctTime;
    ctTime = time(NULL);

    printf("Getting coordinator info from the internet!...\r\n");
    DownloadFile * df = new DownloadFile(network, radioconfigfile, SSL_CA_PEM );
    if (df)
    {
        std::string url = "https://www.mbedhacks.com/Garden/getconfig.php?id=";
        url += to_string(radioID);
        printf("Getting config from [%s]\r\n", url.c_str());
        df->basic_auth(BASIC_AUTH_USER, BASIC_AUTH_PASSWORD);
        HttpResponse* result = df->get_file(url.c_str());
        if (result != NULL)
        {
            std::string content = df->get_file_content();
            printf("\r\n========= %s =============\r\n", radioconfigfile);
            printf("%s\r\n", content.c_str());
            printf("==============================\r\n");
            
            // TODO: pass the downloaded file to the RadioCfg
            if (parseradioconfig(content.c_str(), cfg) == 0)
            {
                cfg.debug();
            }
        }
        delete df;
    }

    NTPClient ntp(network, cfg.utc_offset);
    pc.printf("Initial System Time is: %s\r\n", ctime(&ctTime));
    pc.printf("Trying to update time...\r\n");
    if (ntp.setTime("0.pool.ntp.org") == 0) {
        
        pc.printf("Set time successfully\r\n");
        pc.printf("Updated time is : %s\r\n", ctime(&ctTime));
        
        pc.printf("Starting the Xbee manager thread ...\r\n");
        xbeeThd.start(runXbeeMonitor);
        pc.printf("Starting MQTTS subscriber thread ...\r\n");
        // Initialize MQTT
        mqttsInit(network, SSL_CA_PEM);
        if ( runMQTTS() != 0 )
            pc.printf("Failed launching MQTT thread ...\r\n");
        
        while(true) {
            time_t currentTime;
            struct tm *localTime;            
            
            time( &currentTime );                   // Get the current time
            localTime = localtime( &currentTime );  // Convert the current time to local time

            // TODO: Update the weather information every hour ...            
            if (lasthour != localTime->tm_hour )
            //if (lasthour != localTime->tm_min )            
            {
                printf("An hour has passed!!! ...\r\n");
                UpdateWeatherInfo(network);
                // Update ...
                //lasthour = localTime->tm_min;
                lasthour = localTime->tm_hour;                
            }

            Thread::wait(10000);
        }
    }
    else
    {
        pc.printf("Failed to setup time from NTP  ...\r\n");
        while(true)
        {
            Thread::wait(100);
        }
    }
}
