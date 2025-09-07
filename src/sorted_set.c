#include "sorted_set.h"


#define container_of(ptr, type, attr) (type *)((char *)(ptr) - offsetof(type, attr));

// TODO : check this
int compare_tree(AVLNode *left, AVLNode *right) {
    TEntry *entry_left = container_of(left, TEntry, node);
    TEntry *entry_right = container_of(right, TEntry, node);

    if (entry_left->value == entry_right->value ) return 0;
    if (entry_left->value < entry_right->value ) return -1;
    return 1;
}

int ent_less_than(AVLNode *left, AVLNode *right) {
    TEntry *entry_left = container_of(left, TEntry, node);
    TEntry *entry_right = container_of(right, TEntry, node);
    return entry_left->value < entry_right->value;
}

void add_tree_entry(AVLNode **root, int value) {
    TEntry *entry = (TEntry *)malloc(sizeof(TEntry));
    init_tree_node(&entry->node);
    entry->value = value;

    // Query and check if already there
    if (search(root, &entry->node, compare_tree) == NULL) {
        search_and_insert(root, &entry->node, ent_less_than);
    }
}

void remove_tree_entry(AVLNode **root, int value) {
    TEntry entry;
    init_tree_node(&entry.node);
    entry.value = value;

    AVLNode *removed = search_and_delete(root, &entry.node, compare_tree);
    if (removed != NULL) {
        TEntry *removed_entry = container_of(removed, TEntry, node);
        free(removed_entry);
    }
}

ZSet *new_sorted_set() {
    ZSet *s = malloc(sizeof(ZSet));
    s->by_score = NULL;
    init_map(&s->by_name);
    return s;
}

bool hn_eq(HNode *a, HNode *b)
{
    ZNode *first = container_of(a, ZNode, h_node);
    ZNode *second = container_of(b, ZNode, h_node);
    if (first->length != second->length) return false;
    return 0 == memcmp(first->key, second->key, strlen(first->key));
}

int compare(AVLNode *a, AVLNode *b) {
    ZNode *first = container_of(a, ZNode, tree_node);
    ZNode *second = container_of(b, ZNode, tree_node);

    if (first->score != second->score) {
        return first->score < second->score ? -1 : 1;
    }
    // check the name 
    int res = memcmp(first->key, second->key, fmax(strlen(first->key), strlen(second->key)));
    if(res < 0) return -1;
    if(res > 0) return 1;
    return 0;
}

int less_than(AVLNode *a, AVLNode *b) {
    ZNode *first = container_of(a, ZNode, tree_node);
    ZNode *second = container_of(b, ZNode, tree_node);

    if (first->score != second->score) {
        return first->score < second->score;
    }
    // check the name 
    int res = memcmp(first->key, second->key, fmax(strlen(first->key), strlen(second->key)));
    return res < 0;
}

ZNode* znode_new(float score, char *key, size_t length) {
    ZNode *node = (ZNode *)malloc(sizeof(ZNode) + length + 1);
    node->score = score;
    node->length = length;
    memcpy(&node->key[0], key, length);
    node->key[length] = '\0';

    // Tree
    init_tree_node(&node->tree_node);
    // Map
    node->h_node.hash = fnv_32a_str(key, length);
    node->h_node.next = NULL;
    return node;
}

void znode_free(ZNode *node) {
    free(node);
}

TEntry *znode_offset(AVLNode *node, int offset) {
    AVLNode *offseted = avl_offset(node, offset);
    TEntry *ent = container_of(offseted, TEntry, node);
    return ent;
}

ZNode *zset_hm_lookup(ZSet *s, char *key, size_t length) {
    if (s->by_name.newer.length == 0) return NULL;

    ZNode* entry = znode_new(0, key, length);

    HNode *found_ptr = hm_get(&s->by_name, &entry->h_node, hn_eq);
    if (found_ptr == NULL) return NULL;
    ZNode *res = container_of(found_ptr, ZNode, h_node);
    free(entry);
    return res;
}

void zset_query(ZSet *s, float score, char *key, size_t length, int offset, int limit) {
    ZNode *target = znode_new(score, key, length);
    init_tree_node(&target->tree_node);
    
    AVLNode *curr = avl_find_ge(&s->by_score, &target->tree_node, compare);
    curr = avl_offset(curr, offset);
    for (int i = 0; i < limit; ++i) {
        if (!curr) break;
        ZNode *node = container_of(curr, ZNode, tree_node);
        // TODO : add into array here
        printf("%s, %f\n", node->key, node->score);
        curr = avl_offset(curr, +1);
    }

    znode_free(target);
}

void zset_update(ZSet *s, ZNode *entry, float new_score) {
    if (entry->score == new_score) return;
    s->by_score = avl_detach(&entry->tree_node);
    init_tree_node(&entry->tree_node);
    entry->score = new_score;
    search_and_insert(&s->by_score, &entry->tree_node, compare);
}

int zset_add(ZSet *s, float score, char *key, size_t length) {
    ZNode *entry = zset_hm_lookup(s, key, length);
    if (entry != NULL) {
        zset_update(s, entry, score);
        return 0;
    }
    ZNode *new_entry = znode_new(score, key, length);
    search_and_insert(&s->by_score, &new_entry->tree_node, compare);
    hm_set(&s->by_name, &new_entry->h_node, hn_eq);
    return 1;
}

bool zset_rem(ZSet *s, char *key, size_t length) {
    ZNode *entry = zset_hm_lookup(s, key, length);
    if (entry == NULL) {
        return false;
    }
    hm_delete(&s->by_name, &entry->h_node, hn_eq);
    AVLNode *avl_deleted = search_and_delete(&s->by_score, &entry->tree_node, less_than);
    if (avl_deleted) {
        ZNode *del = container_of(avl_deleted, ZNode, tree_node);
        znode_free(del);
    }
    return true;
}

