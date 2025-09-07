#include "common.h"
#include "tree.h"
#include "map_chain.h"

#ifndef SORTED_SET_H
#define SORTED_SET_H

typedef struct {
    AVLNode *by_score;
    HMap by_name;
} ZSet;

typedef struct {
    AVLNode tree_node;
    HNode h_node;

    float score;
    size_t length;
    char key[];
} ZNode;

typedef struct {
    AVLNode node;
    int value;
} TEntry;

ZSet * new_sorted_set();
int zset_add(ZSet *s, float score, char *key, size_t length);
bool zset_rem(ZSet *s, char *key, size_t length);
ZNode *zset_hm_lookup(ZSet *s, char *key, size_t length);
void add_tree_entry(AVLNode **root, int value);
void remove_tree_entry(AVLNode **root, int value);
ZNode* zset_find_ge(ZSet *s, float score, char *key, size_t length);
void zset_query(ZSet *s, float score, char *key, size_t length, int offset, int limit);
TEntry *znode_offset(AVLNode *node, int offset);
#endif
