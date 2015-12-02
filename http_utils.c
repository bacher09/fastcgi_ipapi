#include <stdio.h>
#include <stdbool.h>
#include "http_utils.h"


static inline signed short hctoi(const char sym){
    if (sym >= '0' && sym <= '9')
        return sym - '0';
    else if (sym >= 'a' && sym <= 'f')
        return sym - 'a' + 10;
    else if (sym >= 'A' && sym <= 'F')
        return sym - 'A' + 10;
    else
        return -1;
}

static inline signed short hex2char(const char *hex){
    signed short f, s;

    if(hex == NULL || hex[0] == '\0' || hex[1] == '\0')
        return -1;

    f = hctoi(hex[0]);
    s = hctoi(hex[1]);

    if(f < 0 || s < 0)
        return -1;

    return (f << 4) | s;
}


static inline int get_bit(const unsigned char bit, const unsigned char bit_set[32]) {
    return bit_set[bit / 8] >> (bit % 8) & 1;
}

static inline void set_bit(const unsigned char bit, unsigned char bit_set[32]) {
    bit_set[bit / 8] |= 1 << (bit % 8);
}


int read_decode_until(const char *str, const char *seps, char *res,
                       int max_write, int *write_count) {
    int i, w;
    signed short dec;
#define APPEND(X) if(w < max_write) {res[w++] = (X);}
    unsigned char char_set[32] = {0};

    if(str == NULL || str[0] == '\0')
        return 0;

    /* Init bit set */
    for(i = 0; seps[i] != '\0'; i++)
        set_bit(seps[i], char_set);

    set_bit('\0', char_set);

    i = w = 0;

    while (!get_bit(str[i], char_set)){
        switch(str[i]){
            case '+':
                APPEND(' ');
                i++;
                break;
            case '%':
                dec = hex2char(&str[i + 1]);
                if(dec >= 0) {
                    APPEND((char)dec);
                    i += 3;
                } else {
                    APPEND('%');
                    i++;
                }
                break;
            default:
                APPEND(str[i]);
                i++;
                break;
        }
    }

    *write_count = w;
    return i;
#undef APPEND
}


_Bool iterate_qs(query_param *qp, int *i, const char *query_string) {
    int readed;
    qp->key_len = 0;
    qp->value_len = 0;
    if (query_string == NULL || *query_string == '\0')
        return false;

    /* If there are multiple & symbols then skip them */
    while (query_string[*i] == '&')
        (*i)++;

    if(query_string[*i] == '\0')
        return false;

    readed = read_decode_until(&query_string[*i], "=&", qp->key,
                               qp->max_key, &(qp->key_len));
    *i += readed;

    if(query_string[*i] == '=')
        (*i)++;


    readed = read_decode_until(&query_string[*i], "&", qp->value,
                               qp->max_value, &(qp->value_len));

    *i += readed;
    return true;
}
