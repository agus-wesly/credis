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

SortedSet *new_sorted_set() {
    SortedSet *s = malloc(sizeof(SortedSet));
    s->by_score = NULL;
    init_map(&s->by_name);
    return s;
}

bool hn_eq(HNode *a, HNode *b)
{
    SEntry *first = container_of(a, SEntry, h_node);
    SEntry *second = container_of(b, SEntry, h_node);

    return memcmp(first->key, second->key, fmin(strlen(first->key), strlen(second->key))) == 0;
}

int less_than(AVLNode *a, AVLNode *b) {
    SEntry *first = container_of(a, SEntry, tree_node);
    SEntry *second = container_of(b, SEntry, tree_node);

    if (first->score != second->score) {
        return first->score < second->score ? 1 : -1;
    }
    // check the name 
    int res = memcmp(second->key, first->key, fmax(strlen(first->key), strlen(second->key)));
    if(res < 0) return -1;
    if(res > 0) return 1;
    return 0;
}

SEntry* new_sorted_entry(float score, char *key, size_t length) {
    SEntry *entry = (SEntry *)malloc(sizeof(SEntry) + length + 1);
    entry->score = score;
    entry->length = length;
    memcpy(&entry->key[0], key, length);
    entry->key[length] = '\0';
    return entry;
}

SEntry *zset_lookup_map(SortedSet *s, char *key, size_t length) {
    if (s->by_name.newer.length == 0) return NULL;

    SEntry entry;
    entry.h_node.hash = fnv_32a_str(key, length);
    entry.h_node.next = NULL;
    
    HNode *found_ptr = hm_get(&s->by_name, &entry.h_node, hn_eq);
    if (found_ptr == NULL) return NULL;
    return container_of(found_ptr, SEntry, h_node);
}

void zset_update(SortedSet *s, SEntry *entry, float new_score) {
    if (entry->score == new_score) return;
    AVLNode *removed = search_and_delete(&s->by_score, &entry->tree_node, less_than);
    assert(removed != NULL && "Should have something to removed");

    entry = container_of(removed, SEntry, tree_node);
    entry->score = new_score;
    search_and_insert(&s->by_score, &entry->tree_node, less_than);
}

int zset_add(SortedSet *s, float score, char *key, size_t length) {
    SEntry *entry = zset_lookup_map(s, key, length);
    if (entry != NULL) {
        zset_update(s, entry, score);
        return 0;
    }
    SEntry *new_entry = new_sorted_entry(score, key, length);
    // Tree
    init_tree_node(&new_entry->tree_node);
    search_and_insert(&s->by_score, &new_entry->tree_node, less_than);
    // Map
    new_entry->h_node.hash = fnv_32a_str(key, length);
    new_entry->h_node.next = NULL;

    hm_set(&s->by_name, &new_entry->h_node, hn_eq);
    return 1;
}

bool zset_rem(SortedSet *s, char *key, size_t length) {
    // Cannot search on the hashmap
    // Because it can be more than one
    SEntry *entry = zset_lookup_map(s, key, length);
    if (entry == NULL) {
        return false;
    }
    search_and_delete(&s->by_score, &entry->tree_node, less_than);
    hm_delete(&s->by_name, &entry->h_node, hn_eq);
    return true;
}

