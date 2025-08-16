#include "common.h"
#include "hash.h"

#ifndef MAP_CHAIN_H
#define MAP_CHAIN_H

typedef struct HNode HNode;

struct HNode
{
    HNode *next;
    int32 hash;
};

typedef struct
{
    HNode **nodes;
    size_t capacity; // Must be the power of 2
    size_t length;
} HTab;

// For now just use the newer
// Handle the resize later
typedef struct
{
    HTab newer;
    HTab older;
    size_t move_idx;
} HMap;


#define container_of(ptr, type, attr) (type *)((char *)(ptr) - offsetof(type, attr));


void init_map(HMap *);
void add_node(HMap *ht, HNode *value);
void delete_node(HMap *hm, HNode *node, bool (*find)());
HNode **ht_lookup(HTab *ht, HNode *node, bool (*find)(HNode *, HNode *));
HNode *hm_get(HMap *hm, HNode *node, bool (*find)(HNode *, HNode *));
HNode *hm_delete(HMap *hm, HNode *node, bool (*find)(HNode *, HNode *));
void hm_set(HMap *hm, HNode *value, bool (*eq)(HNode *left, HNode *right));
void ht_each(HTab *ht, void(cb)(HNode *node, void *ptr), void *arg);

#endif
