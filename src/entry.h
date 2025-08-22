#include "common.h"
#include "sorted_set.h"
#include "map_chain.h" 

#ifndef ENTRY_H
#define ENTRY_H

typedef enum {
    ENTRY_STRING,
    ENTRY_SET
} EntryType;

// ["string", {}]
typedef struct
{
    EntryType type;

    HNode node;
    char *key;
    union {
        SortedSet *set;
        char *str;
    };
} Entry;

#define IS_ENTRY_STRING(entry) (entry->type ==  ENTRY_STRING)
#define IS_ENTRY_SET(entry) (entry->type ==  ENTRY_SET)

#endif // ENTRY_H
