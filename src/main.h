#include "buffer.h"
#include "dynamic_array.h"
#include "request.h"
#include "map.h"

#include <poll.h>

#ifndef MAIN_H
#define MAIN_H
typedef struct
{
    int fd;

    bool want_read;
    bool want_write;
    bool want_close;

    Buffer *incoming;
    Buffer *outgoing;
} Conn;

#define buf_append(buff, data, size) \
    do{ \
        for (int i = 0; i < (size); ++i) \
        { \
            buff_push((buff), ((char *)(data))[i]); \
        } \
    } while(0); \


#define buf_consume(arr, n) \
do{ \
    for (int i = 0; i < n; ++i) \
    { \
        buff_pop_front((arr)); \
    } \
} while(0); \


#define REPLY(c, ...) \
    do{  \
        char msg[512]; \
        memset(msg, 0, sizeof(msg)); \
        sprintf(msg,  __VA_ARGS__); \
        reply(c, msg); \
    } while(0); \

Conn* new_connection(struct pollfd*);

#endif



