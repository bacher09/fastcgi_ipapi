#!/bin/sh

DAT_DIR="/usr/share/GeoIP"

GEOIP_URL="http://geolite.maxmind.com/download/geoip/database/"

GEOLITE_COUNTRY_PATH="GeoLiteCountry/"
GEOLITE_COUNTRY_FILE="GeoIP.dat.gz"

GEOLITE_COUNTRY_IPV6_PATH=""
GEOLITE_COUNTRY_IPV6_FILE="GeoIPv6.dat.gz"

GEOLITE_CITY_PATH=""
GEOLITE_CITY_FILE="GeoLiteCity.dat.gz"

GEOLITE_CITY_IPV6_PATH="GeoLiteCityv6-beta/"
GEOLITE_CITY_IPV6_FILE="GeoLiteCityv6.dat.gz"

GEOLITE_ASNUM_PATH="asnum/"
GEOLITE_ASNUM_FILE="GeoIPASNum.dat.gz"

GEOLITE_ASNUM_IPV6_PATH="asnum/"
GEOLITE_ASNUM_IPV6_FILE="GeoIPASNumv6.dat.gz"

FAILED=0

for url in \
    "$GEOIP_URL$GEOLITE_COUNTRY_PATH$GEOLITE_COUNTRY_FILE" \
    "$GEOIP_URL$GEOLITE_COUNTRY_IPV6_PATH$GEOLITE_COUNTRY_IPV6_FILE" \
    "$GEOIP_URL$GEOLITE_CITY_PATH$GEOLITE_CITY_FILE" \
    "$GEOIP_URL$GEOLITE_CITY_IPV6_PATH$GEOLITE_CITY_IPV6_FILE" \
    "$GEOIP_URL$GEOLITE_ASNUM_PATH$GEOLITE_ASNUM_FILE" \
    "$GEOIP_URL$GEOLITE_ASNUM_IPV6_PATH$GEOLITE_ASNUM_IPV6_FILE"
do
    echo "Downloading: $url"

    FILEGZ=$(basename $url)
    FILE=${FILEGZ%.gz}

    /usr/bin/wget -q -t3 -T15 "$url" -O "/tmp/${FILEGZ}"

    if [ "$?" != "0" ]
    then
        echo "Failed to download $url"
    else
        /bin/gunzip -f "/tmp/${FILEGZ}"

        if [ "$?" != "0" ]
        then
            echo "Failed to decompress $FILEGZ"
        else
            chmod 644 "/tmp/${FILE}"
            mv "/tmp/${FILE}" "${DAT_DIR}/${FILE}"
        fi
    fi

    rm -f "/tmp/$FILEGZ"
done

exit 0
