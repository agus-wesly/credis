#include "common.h"
#ifndef REQUEST_H
#define REQUEST_H
typedef struct
{
    int nstrings;
    int readed;
    int8 *head;
} Request;

Request *new_request(int8 *buffer, int nbytes);
char *read_next(Request *r);
void free_request(Request *r);

#endif
