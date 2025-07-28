#include "buffer.h"

Buffer *new_buffer()
{
    Buffer *b = (Buffer *)malloc(sizeof(Buffer));
    b->start = 0;
    b->end = 0;

    b->data = malloc(sizeof(int8) * 8);
    b->cap = 8;
    return b;
}
void buff_push(Buffer *b, int8 new_data)
{
    // Check if need to resize
    if (buff_len(b) + 1 > b->cap || b->end >= b->cap)
    {
        size_t len = buff_len(b);
        size_t new_cap = GROW_CAPACITY(b->cap);
        int8 *old_buff = b->data;
        int8 *new_buff = (int8 *)malloc(new_cap);
        assert(new_buff);

        memcpy(new_buff, (int8 *)&old_buff[b->start], len);
        b->start = 0;
        b->end = len;
        b->data = new_buff;
        b->cap = new_cap;

        free(old_buff);
    }
    b->data[b->end++] = new_data;
}

void buff_pop_front(Buffer *b)
{
    assert(buff_len(b) > 0);
    b->start += 1;
}

void display_buffer(Buffer *b)
{
    printf("[");
    for (size_t i = b->start; i < b->end; ++i)
    {
        printf("%d,", b->data[i]);
    }
    printf("]\n");
}

void free_buff(Buffer *b) {
    free(b->data);
    free(b);
}

// int testcase()
// {
//     Buffer *b = new_buffer();
// 
//     display_buffer(b);
//     buff_push(b, 69);
//     display_buffer(b);
//     buff_pop_front(b);
//     display_buffer(b);
// 
//     for (int i = 0; i < 2000; ++i)
//     {
//         buff_push(b, i);
//     }
//     // display_buffer(b);
//     for (int i = 0; i < 2000; ++i)
//     {
//         buff_pop_front(b);
//     }
//     // display_buffer(b);
// 
//     for (int i = 0; i < 2000; ++i)
//     {
//         buff_push(b, i);
//         buff_pop_front(b);
//     }
// 
//     for (int i = 0; i < 5; ++i)
//     {
//         buff_push(b, i);
//     }
//     display_buffer(b);
//     for (int i = 0; i < 3; ++i)
//     {
//         buff_pop_front(b);
//     }
//     display_buffer(b);
//     for (int i = 0; i < 5; ++i)
//     {
//         buff_push(b, i);
//     }
//     display_buffer(b);
//     printf("Len : %zu, Cap : %zu\n", buff_len(b), b->cap);
//     return 0;
// }
