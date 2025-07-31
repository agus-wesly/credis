#include "common.h"
#include "hash.h"

#ifndef MAP_H
#define MAP_H

#define CAP_FACTOR 0.75 // TODO : Adjust this


typedef char *Key;
typedef char *Value;

typedef struct {
    int32 hash;
    Value value;
} Entry;

typedef struct
{
    size_t cap;
    size_t len;
    Entry *entries;
} Map;

#define set_grave(val) (val = &grave_value)
#define is_grave(val) (val == &grave_value)

void init_map(Map *);
void map_set(Map *, Key k, Value v);
bool map_get(Map *, Key k, Value *v);
bool map_delete(Map *, char *k);
void print_map(Map *);
void free_map(Map *m);

#endif
