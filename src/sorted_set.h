#include "common.h"
#include "tree.h"
#include "map_chain.h"

#ifndef SORTED_SET_H
#define SORTED_SET_H

typedef struct {
    AVLNode *by_score;
    HMap by_name;
} SortedSet;

typedef struct {
    AVLNode tree_node;
    HNode h_node;

    float score;
    size_t length;
    char key[];
} SEntry;

typedef struct {
    AVLNode node;
    int value;
} TEntry;

SortedSet * new_sorted_set();
int zset_add(SortedSet *s, float score, char *key, size_t length);
bool zset_rem(SortedSet *s, char *key, size_t length);
SEntry *zset_lookup_map(SortedSet *s, char *key, size_t length);
void add_tree_entry(AVLNode **root, int value);

#endif
