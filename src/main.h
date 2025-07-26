#include "buffer.h"
#include "dynamic_array.h"
#include <poll.h>

#ifndef MAIN_H
#define MAIN_H
typedef struct
{
    int fd;

    bool want_read;
    bool want_write;
    bool want_close;

    int8 *incoming;
    int8 *outgoing;
} Conn;

#define buf_append(buff, data, size) \
    do{ \
        for (int i = 0; i < (size); ++i) \
        { \
            arr_push((buff), ((char *)(data))[i]); \
        } \
    } while(0); \


#define buf_consume(arr, n) \
do{ \
    for (int i = 0; i < n; ++i) \
    { \
        pop_front((arr)); \
    } \
} while(0); \

Conn* new_connection(struct pollfd*);

#endif



