# ip_api

This is simple [fastcgi] web application that provides HTTP API to GeoIP
database and some additional services. It written in C.

## API Calls

| Path                             |  Result                                                             |
| -------------------------------- | ------------------------------------------------------------------- |
| /ip                              | Returns your current ip                                             |
| /user-agent                      | Returns user-agent of your web-client                               |
| /country                         | Returns 2 digit country code of your ip                             |
| /country?ip=7.7.7.7              | Returns 2 digit country code of `7.7.7.7`                           |
| /country?ip=7.7.7.7&type=code3   | Returns 3 digit country code of `7.7.7.7`                           |
| /country?type=name               | Returns your country name                                           |
| /country?type=xonotic&ip=7.7.7.7 | Returns country and requested ip, can be used in [RocketMinsta] mod |

For example type this to get your IPv4 address:

    $  curl -4 "http://ip-api-installed-domain/ip"

and for IPv6:

    $  curl -6 "http://ip-api-installed-domain/ip"

Already installed instance available via `api.vinnica.in` domain, it supports
both http and https.

## Building from source

To build it from source you should install C compiler, cmake, geoip database files,
geoip and fastcgi libraries and C headers.
For example, to do it in debian, you could type this command:

    $ apt-get install build-essential cmake libfcgi-dev libgeoip-dev geoip-database-contrib

Then you could build it via following commands:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

Also you could build it without IPv6 support, just add `-DWITH_IPV6:BOOL=OFF`
option to cmake.

    $ cmake -DWITH_IPV6:BOOL=OFF ..

[fastcgi]: https://en.wikipedia.org/wiki/FastCGI
[RocketMinsta]: http://rocketminsta.net/
