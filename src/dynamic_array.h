#include "common.h"
#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

typedef struct
{
    int len;
    int cap;
} Header;



#define GROW_ARRAY(ptr, type, cap) \
    (type *)realloc((ptr), (cap))

void *arr_allocate(size_t, size_t);

#define arr_header(a) ((Header *)((char *)a - sizeof(Header)))
#define arr_cap(a) (arr_header(a)->cap)
#define arr_len(a) (arr_header(a)->len)
#define arr_init(arr)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        if (arr == NULL)                                                                                               \
        {                                                                                                              \
            arr = arr_allocate(8, sizeof(*arr));                                                                       \
            arr_len(arr) = 0;                                                                                          \
            arr_cap(arr) = 8;                                                                                          \
        }                                                                                                              \
    } while (0);

#define arr_resize(arr, new_cap)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        arr = (void *)(realloc((void *)arr_header(arr), sizeof(Header) + new_cap * sizeof(*arr)));                     \
        arr = (void *)((char *)arr + sizeof(Header));                                                                  \
    } while (0);

#define arr_push(arr, item)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (arr)                                                                                                       \
        {                                                                                                              \
            if (arr_len(arr) + 1 > arr_cap(arr))                                                                       \
            {                                                                                                          \
                int new_cap = GROW_CAPACITY(arr_cap(arr));                                                             \
                arr_resize(arr, new_cap);                                                                              \
                arr_cap(arr) = new_cap;                                                                                \
            }                                                                                                          \
            arr[arr_len(arr)++] = item;                                                                                \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            arr_init(arr) arr[arr_len(arr)++] = item;                                                                  \
        }                                                                                                              \
    } while (0);

#define pop_front(arr)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        int len = arr_len(arr);                                                                                        \
        assert(len > 0);                                                                                               \
        for (int i = 0; i < len - 1; ++i)                                                                                  \
        {                                                                                                              \
            arr[i] = arr[i + 1];                                                                                       \
        }                                                                                                              \
        arr_len(arr) = len - 1;                                                                                        \
    } while (0);

#define arr_free(arr) free(arr_header(arr))
#endif
