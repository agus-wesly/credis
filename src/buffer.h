#include "common.h"

#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    size_t head;
    size_t tail;
    size_t len;
    size_t cap;
    int8 *data;
} Buffer;

#define buff_data(b) (&b->data[b->head])

Buffer *new_buffer();
void buff_push(Buffer *, int8);
void buff_pop_front(Buffer *);
void display_buffer(Buffer *b);

#endif
