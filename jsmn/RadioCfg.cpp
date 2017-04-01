#include "RadioCfg.h"
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

int parseradioconfig(const char * jsonstring, RadioCfg & radiocfg)
{
    int i, r;
    
    jsmn_parser p;
    jsmntok_t t[128];
    
    jsmn_init(&p);
    r = jsmn_parse(&p, jsonstring, strlen(jsonstring), t, sizeof(t)/sizeof(t[0]));
    if ( r < 0 )
        return -1;
    
    /* Top level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT)
    {
        return -1;
    }
    
    /* Loop over all tokens */
    for (i = 1; i < r; i++)
    {
        if (jsoneq(jsonstring, &t[i], "error") == 0)
        {
            // Query returned an error ...
            return -1;
        }
        else if (jsoneq(jsonstring, &t[i], "id") == 0)
        {
            std::string idstr((const char *) jsonstring + t[i+1].start, 
                    (size_t) t[i+1].end - t[i+1].start);
                    
            // Store the radio id ...
            radiocfg.radioID = strtoull(idstr.c_str(), NULL, 0);
            i++;
        }
        else if (jsoneq(jsonstring, &t[i], "name") == 0)
        {
            radiocfg.name = std::string((const char *) jsonstring + t[i+1].start,
                    (size_t) t[i+1].end - t[i+1].start);
            i++;
        }
        else if (jsoneq(jsonstring, &t[i], "country") == 0)
        {
            radiocfg.country = std::string((const char *) jsonstring + t[i+1].start,
                    (size_t) t[i+1].end - t[i+1].start);
            i++;
        }
        else if (jsoneq(jsonstring, &t[i], "utc_offset") == 0)
        {
            std::string idstr((const char *) jsonstring + t[i+1].start, 
                    (size_t) t[i+1].end - t[i+1].start);
                    
            radiocfg.utc_offset = strtoul(idstr.c_str(), NULL, 0);
            i++;
        }                
        else if (jsoneq(jsonstring, &t[i], "radios") == 0)
        {
            int j;
            // Radios is an array of radio objects
            if (t[i+1].type != JSMN_ARRAY){
                continue;
            }
            
            int arrsize = t[i+1].size;
            // Move to the start of the array
            i += 2;
            for (j = 0; j < arrsize; j++)
            {
                int r = t[i + j].size;
                //jsmntok_t * g = &t[i + j];                
                //printf(" * %.*s\n", g->end - g->start, jsonstring + g->start);
                // Enumerate objects within the array
                for (int z = 0; z < r ; z++)
                {
                    RadioSlave slv;
                    if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "id") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        std::string idstr((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                        slv.radioID = strtoull(idstr.c_str(), NULL, 0);
                    }
                    else if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "name") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        slv.name = std::string((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                    }
                    if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "temp_pin") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        std::string idstr((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                        slv.temp_pin = strtoul(idstr.c_str(), NULL, 0);
                    }
                    if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "humid_pin") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        std::string idstr((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                        slv.humid_pin = strtoul(idstr.c_str(), NULL, 0);
                    }
                    if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "lumin_pin") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        std::string idstr((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                        slv.lumin_pin = strtoul(idstr.c_str(), NULL, 0);
                    }                
                    if (jsoneq(jsonstring, &t[i+j+(z*2)+1], "spinkler_pin") == 0)
                    {
                        jsmntok_t * o = &t[i+j+(z*2)+2];
                        std::string idstr((const char *)jsonstring + o->start, 
                                (size_t) o->end - o->start);
                        slv.sprinkler_pin = strtoul(idstr.c_str(), NULL, 0);
                    }                            
                    else
                    {
                        // TODO:
                    }
                    // Add the radio slave to the vector
                    radiocfg.radios.push_back(slv);
                }
                
                i += (t[i+j].size) * 2;
            }
            // Move to the end of the object
            i += 4;
        }else
        {
            
        }
    }
    
    return 0;    
}
