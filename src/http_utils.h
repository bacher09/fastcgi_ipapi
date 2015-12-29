#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#include <stdbool.h>

typedef struct {
    int key_len;
    int value_len;
    int max_key;
    int max_value;
    char *key;
    char *value;
} query_param;

_Bool iterate_qs(query_param*, int*, const char*);
#endif
