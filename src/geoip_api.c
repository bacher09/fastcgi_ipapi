#include "geoip_api.h"

#include "GeoIP.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

static GeoIP *geoip_country;
static GeoIP *geoip_country_v6;


void Geoip_Init(){

    if(!GeoIP_db_avail(GEOIP_COUNTRY_EDITION) ||
       !GeoIP_db_avail(GEOIP_COUNTRY_EDITION_V6))
        goto err;

    geoip_country = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_MEMORY_CACHE);
    if(geoip_country == NULL)
        goto err;

    geoip_country_v6 = GeoIP_open_type(GEOIP_COUNTRY_EDITION_V6, GEOIP_INDEX_CACHE);

    if(geoip_country_v6 == NULL)
        goto err;

    return;

    err:
        perror("Geoip country is not available");
        exit(2);
}


void Geoip_Cleanup() {
    GeoIP_delete(geoip_country);
    GeoIP_delete(geoip_country_v6);
}

AddrType get_addr_family(const char *address) {
    struct addrinfo hint, *res = NULL;
    AddrType result;
    int err;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;
    err = getaddrinfo(address, NULL, &hint, &res);
    if(err)
        return BAD_IP;
    else if(res->ai_family == AF_INET)
        result = IP_V4;
    else if(res->ai_family == AF_INET6)
        result = IP_V6;
    else
        result = BAD_IP;

    freeaddrinfo(res);
    return result;
}

int lookup_country_id(const char *ip_address) {
    AddrType type = get_addr_family(ip_address);

    if(type == IP_V4)
        return GeoIP_id_by_addr(geoip_country, ip_address);
    else if(type == IP_V6)
        return GeoIP_id_by_addr_v6(geoip_country_v6, ip_address);
    else
        return -1;
}

const char *country_code_by_id(int id){
    return (id > 0) ? GeoIP_code_by_id(id) : NULL;
}

const char *country_code3_by_id(int id){
    return (id > 0) ? GeoIP_code3_by_id(id) : NULL;
}

const char *country_name_by_id(int id){
    return (id > 0) ? GeoIP_country_name_by_id(geoip_country, id) : NULL;
}
