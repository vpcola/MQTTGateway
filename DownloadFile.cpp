#include "DownloadFile.h"
#include "https_request.h"

void dump_response(HttpResponse* res) 
{
    mbedtls_printf("\r\nStatus: %d - %s\r\n", res->get_status_code(), res->get_status_message().c_str());

    mbedtls_printf("Headers:\r\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        mbedtls_printf("\t%s: %s\r\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    mbedtls_printf("\nBody (%d bytes):\r\n\r\n%s\r\n", res->get_body_length(), res->get_body_as_string().c_str());
}

static unsigned int base64enc_len(const char *str) 
{
   return (((strlen(str)-1)/3)+1)<<2;
}

static void base64enc(const char *input, unsigned int length, char *output) 
{
   static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   unsigned int c, c1, c2, c3;
   for(unsigned int i = 0, j = 0; i<length; i+=3,j+=4) {
     c1 = ((((unsigned char)*((unsigned char *)&input[i]))));
     c2 = (length>i+1)?((((unsigned char)*((unsigned char *)&input[i+1])))):0;
     c3 = (length>i+2)?((((unsigned char)*((unsigned char *)&input[i+2])))):0;

     c = ((c1 & 0xFC) >> 2);
     output[j+0] = base64[c];
     c = ((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4);
     output[j+1] = base64[c];
     c = ((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6);
     output[j+2] = (length>i+1)?base64[c]:'=';
     c = (c3 & 0x3F);
     output[j+3] = (length>i+2)?base64[c]:'=';
   }
   output[(((length-1)/3)+1)<<2] = '\0';
}

static string encode(const string& str)
{
    char* out = new char[ base64enc_len(str.c_str()) ];
    base64enc(str.c_str(), str.length(), out);
    string res(out);
    delete[] out;
    return res;
}

void DownloadFile::basic_auth(const char * user, const char * password)
{
    authstr = user;
    authstr += ":";
    authstr += password;
    printf("Auth Str : %s\r\n", authstr.c_str());

    std::string base64str = encode(authstr);
    printf("Base64 conversion : %s\r\n", base64str.c_str());
    
    authstr = "Basic " + base64str;
    printf("Authorization: %s\r\n", authstr.c_str()); 
}
        
HttpResponse* DownloadFile::get_file(const char * url)
{
    if (url == NULL) 
        return NULL;

    if(get_req != NULL) 
        delete get_req;
        
    HttpResponse* get_res;
    Callback<void(const char *at, size_t length)> aBodyCallback = NULL;
    
    if (fp != NULL)
        aBodyCallback = mbed::callback(this, &DownloadFile::body_callback);
    
    if (useSSL)
    {                
        get_req_ssl = new HttpsRequest(network, 
                pem, HTTP_GET, 
                url,
                aBodyCallback);
                
        if (!authstr.empty())
            get_req_ssl->set_header("Authorization", authstr.c_str());
            
        get_req_ssl->set_debug(true);
        
        get_res = get_req_ssl->send();
        
    }
    else
    {
        get_req = new HttpRequest(network, 
                HTTP_GET, 
                url,
                aBodyCallback);
    
        if (!authstr.empty())
            get_req->set_header("Authorization", authstr.c_str());
            
        get_res = get_req->send();        
    }
    
    if (!get_res) {
            printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
            return NULL;
    }
    
    //dump_response(get_res);

    return get_res;    
}

std::string DownloadFile::get_file_content()
{
    size_t numread;
    
    if (fp == NULL) 
        return "";
        
    // plain old c ..
    // Determine file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);

    char* dummy = new char[(sizeof(char) * size) + 1];

    rewind(fp);
    numread = fread(dummy, sizeof(char), size, fp);
    // Make sure its NULL terminanted
    dummy[numread] = 0;

    // create a return string
    std::string retstr = std::string((const char *) dummy);

    delete[] dummy;

    return retstr;
}

void DownloadFile::body_callback(const char* data, size_t data_len)
{
    // do something with the data
    if (fp != NULL) {
        size_written += fwrite((const void *) data, sizeof(char), data_len, fp);
    }
}
