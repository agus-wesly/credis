#include "dynamic_array.h"

void *arr_allocate(size_t nmemb, size_t size)
{
    char *ret = (void *)calloc(nmemb, size + sizeof(Header));
    return (void *)(ret + sizeof(Header));
}

