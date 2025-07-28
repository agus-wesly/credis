#include "request.h"

void read4byte(int8 *request, int len, void *dest)
{
    int8 *end = request + len;
    assert(end >= (int8 *)(request + 4) && "Too short");
    memcpy((int *)dest, request, 4);
}

Request *new_request(int8 *buffer, int nbytes)
{
    Request *r = (Request *)malloc(sizeof(Request));
    r->readed = 0;

    int nstrings = {0};
    read4byte(buffer, nbytes, &nstrings);
    r->nstrings = nstrings;
    buffer += 4;

    r->head = buffer;

    return r;
}

void free_request(Request *r) {
    free(r);
}

char *read_next(Request *r)
{
    if (r->readed >= r->nstrings)
        return NULL;

    // read the char length
    int nchar = {0};
    read4byte(r->head, 4, &nchar);
    r->head += 4;
    // printf("The %d idx's length is %d\n", i, nchar);

    // read the char
    char *command = malloc(nchar + 1);
    memcpy(command, r->head, nchar);
    command[nchar] = '\0';

    r->head += nchar;
    ++r->readed;

    return command;
}
