#include "buffer.h"

Buffer *new_buffer()
{
    Buffer *b = (Buffer *)malloc(sizeof(Buffer));
    b->cap = 8;
    b->len = 0;
    b->head = 0;
    b->tail = 0;

    b->data = malloc(sizeof(int8) * 8);
    return b;
}
void buff_push(Buffer *b, int8 new_data)
{
    // Check if need to resize
    if (b->len + 1 > b->cap)
    {
        size_t old_cap = b->cap;
        size_t new_cap = GROW_CAPACITY(old_cap);
        int8 *new_buff = (int8 *)malloc(new_cap);
        assert(new_buff);
        int8 *old_buff = b->data;
        {
            size_t head = b->head;
            size_t new_idx, old_idx = {0};
            for (size_t i = 0; i < b->len; ++i)
            {
                new_idx = head % new_cap;
                old_idx = head % old_cap;
                new_buff[new_idx] = old_buff[old_idx];
                head += 1;
            }
            new_idx = (new_idx + 1) % new_cap;

            b->tail = new_idx;
            b->data = new_buff;
            b->cap = new_cap;

            free(old_buff);
        }
    }
    b->data[b->tail] = new_data;
    b->tail = (b->tail + 1) % b->cap;
    b->len++;
}

void buff_pop_front(Buffer *b)
{
    assert(b->len > 0);
    b->head = (b->head + 1) % b->cap;
    --b->len;
}

void display_buffer(Buffer *b)
{
    printf("[");
    for (size_t i = 0, idx = b->head; i < b->len; ++i, idx = (idx + 1) % b->cap)
    {
        printf("%d, ", b->data[idx]);
    }
    printf("]\n");
}

// static void test_case()
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
//     printf("Length :%zu\n", b->len);
//     // display_buffer(b);
// }
