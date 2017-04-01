#ifndef _DOWNLOAD_FILE_H_
#define _DOWNLOAD_FILE_H_

#include <stdio.h>
#include <string>
#include "mbed.h"
#include "rtos.h"
#include "NetworkInterface.h"
#include "https_request.h"
#include "http_request.h"

class DownloadFile
{
    public:
        DownloadFile(NetworkInterface* nw, const char * file = NULL, const char * capem = NULL)
            :network(nw), 
            filename(file), 
            pem(capem), 
            useSSL(capem != NULL), 
            fp(NULL), 
            size_written(0), 
            get_req_ssl(NULL),
            get_req(NULL)
        {
            if (filename)
            {
                fp = fopen(file, "w+");
                if (fp != NULL)
                    printf("File open successfull!\r\n");
            }
        }
        
        virtual ~DownloadFile()
        {
            if (fp != NULL) 
                fclose(fp);
            
            // HttpsRequest destructor also free's up
            // the HttpsResult ... so it must be consumed
            // before this class goes out of scope
            if(get_req)
                delete get_req;
            if(get_req_ssl)
                delete get_req_ssl;
        }
 
        HttpResponse* get_file(const char * url);
        
        std::string get_file_content();
        
        const char * get_filename()
        {
            return filename;
        }
            
        size_t get_written_size() {
            return size_written;
        }
        
        void basic_auth(const char * user, const char * password);

        
    protected:
        void body_callback(const char* data, size_t data_len);
        
    private:
        NetworkInterface* network;
        const char * filename;
        const char * pem; 
        bool useSSL;
        FILE * fp;
        size_t size_written;
        std::string authstr;
        
        HttpsRequest* get_req_ssl;
        HttpRequest* get_req;
};

#endif

