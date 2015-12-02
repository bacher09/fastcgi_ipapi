CC=gcc
LIBS=-lpthread -lfcgi -lGeoIP
CFLAGS=-Wall --std=c11 -O2 -D_POSIX_C_SOURCE=200809L
DEPS=$(wildcard *.h)
C_FILES=$(wildcard *.c)
TARGET=ip_api
OBJ_FILES=$(C_FILES:%.c=%.o)

.PHONY: all clean

all: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	@$(RM) $(OBJ_FILES) $(TARGET)
