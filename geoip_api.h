#ifndef GEOIP_API_H
#define GEOIP_API_H

typedef enum {
    BAD_IP = 0,
    IP_V4 = 1,
    IP_V6 = 2,
} AddrType;


void Geoip_Init();
void Geoip_Cleanup();
AddrType get_addr_family(const char*);
int lookup_country_id(const char*);
const char *country_code_by_id(int);
const char *country_code3_by_id(int);
const char *country_name_by_id(int);
#endif
