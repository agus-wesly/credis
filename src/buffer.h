#include "common.h"

#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    size_t start;
    size_t end;
    size_t cap;
    int8 *data;
} Buffer;

#define buff_len(b)((b->end) - b->start)
#define buff_data(b)((int8 *)&b->data[b->start])

Buffer *new_buffer();
void buff_push(Buffer *, int8);
void buff_pop_front(Buffer *);
void display_buffer(Buffer *b);
void free_buff(Buffer *b);

#endif
