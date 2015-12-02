CC=gcc
LIBS=-lpthread -lfcgi -lGeoIP
CFLAGS=-Wall --std=c11 -O2 -D_POSIX_C_SOURCE=200809L
DEPS=geoip_api.h http_utils.h
TARGET=ip_api

.PHONY: all clean

all: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ip_api: ip_api.o geoip_api.o http_utils.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	@$(RM) $(wildcard *.o) ip_api
