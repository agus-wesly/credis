#include "map_chain.h"

// Because we use chaining this should be greater than 0
// Because each slot can hold multiple node
#define GROW_FACTOR 8

static void ht_init(HTab *ht, size_t n)
{
    // Ensure n is multiply by 2
    assert(n > 0 && n % 2 == 0);

    ht->nodes = calloc(n, sizeof(HNode *));
    ht->capacity = n;
    ht->length = 0;
}

void init_map(HMap *hm)
{
    ht_init(&hm->newer, 8);
    hm->older = (HTab){0};
    hm->move_idx = 0;
}

HNode *ht_detach(HTab *ht, HNode **from)
{
    if (from == NULL)
        return NULL;

    HNode *val = *from;
    *from = val->next;
    --ht->length;

    return val;
}

// [-,-,(1,2),-,(3)]
//
// [-,-,(1,2),-,(3)]
//
// [-,-,(1),-,-,-,-,-]

static void ht_set(HTab *ht, HNode *value)
{
    int32 idx = value->hash & (ht->capacity - 1);
    HNode *node = ht->nodes[idx];

    value->next = node;
    ht->nodes[idx] = value;
    ht->length++;
}

#define PER_HASH 10
void try_rehash(HMap *hm)
{
    if (hm->older.nodes == NULL)
        return; // Don't need to rehash

    int done_rehash = 0;
    while (done_rehash < PER_HASH && hm->older.length > 0)
    {
        HNode **old_node = &hm->older.nodes[hm->move_idx];
        if ((*old_node) == NULL)
        {
            hm->move_idx++;
            continue;
        }
        // printf("Rehashing : %d\n", done_rehash);

        HNode *detached = ht_detach(&hm->older, old_node);
        ht_set(&hm->newer, detached);

        ++done_rehash;
    }

    if (hm->older.nodes && hm->older.length == 0) // Reset
    {
        free(hm->older.nodes);
        hm->older = (HTab){0};
        hm->move_idx = 0;
    }
}

void hm_trigger_rehash(HMap *hm)
{
    hm->older = hm->newer;
    size_t new_cap = GROW_CAPACITY(hm->newer.capacity);
    ht_init(&hm->newer, new_cap);
}

void hm_set(HMap *hm, HNode *value)
{
    if (hm->newer.nodes == NULL)
        ht_init(&hm->newer, 8);

    ht_set(&hm->newer, value);

    if (hm->newer.length > (hm->newer.capacity * GROW_FACTOR))
        hm_trigger_rehash(hm);

    try_rehash(hm);
}

HNode *hm_get(HMap *hm, HNode *node, bool (*find)(HNode *, HNode *))
{
    try_rehash(hm);

    HNode **found_ptr = ht_lookup(&hm->newer, node, find);
    if (found_ptr == NULL)
    {
        found_ptr = ht_lookup(&hm->newer, node, find);
    }
    if (found_ptr == NULL)
        return NULL;

    return *found_ptr;
}

HNode **ht_lookup(HTab *ht, HNode *node, bool (*find)(HNode *, HNode *))
{
    if (ht == NULL || ht->nodes == NULL)
        return NULL;

    int32 idx = node->hash & (ht->capacity - 1);
    HNode **from = &ht->nodes[idx];
    for (HNode *curr; (curr = *from) != NULL; from = &curr->next)
    {
        if (curr->hash == node->hash && find(curr, node))
        {
            return from;
        }
    }
    return NULL;
}

HNode *hm_delete(HMap *hm, HNode *node, bool (*find)(HNode *, HNode *))
{
    try_rehash(hm);

    // Get the *pointer to the target*
    // So that we can modify the value directly in here
    // Without event touching any left or right node
    HNode **from = ht_lookup(&hm->newer, node, find);
    if (from == NULL)
    {
        from = ht_lookup(&hm->older, node, find);
    }

    if (from == NULL)
        return NULL;

    return ht_detach(&hm->newer, from);
}



// int main2()
// {
//     init_map(&map);
// 
//     if (true)
//     {
//         assert(map.older.nodes == NULL);
//         {
//             set_entry("foo", "bar");
//             Entry *entry = get_entry("foo");
//             print_entry(entry);
//         }
//         {
//             set_entry("name", "wesly");
//             Entry *entry = get_entry("name");
//             print_entry(entry);
//         }
//         delete_entry("name");
//         {
//             Entry *entry = get_entry("name");
//             print_entry(entry);
//         }
//         delete_entry("name");
//         {
//             set_entry("name", "wesly");
//             Entry *entry = get_entry("name");
//             print_entry(entry);
//         }
//         return EXIT_SUCCESS;
//     }
// 
//     char str[20];
//     for (int i = 0; i < 10000; ++i)
//     {
//         memset(str, 0, sizeof(str));
//         sprintf(str, "i-%d", i);
//         set_entry(str, str);
//     }
//     printf("Capacity : %zu, Length : %zu\n", map.newer.length, map.newer.capacity);
// 
//     for (int i = 500; i < 1000; ++i)
//     {
//         memset(str, 0, sizeof(str));
//         sprintf(str, "i-%d", i);
//         delete_entry(str);
//     }
//     printf("Capacity : %zu, Length : %zu\n", map.newer.length, map.newer.capacity);
// 
//     for (int i = 2000; i < 10000; ++i)
//     {
//         memset(str, 0, sizeof(str));
//         sprintf(str, "i-%d", i);
//         Entry *e = get_entry(str);
//         assert(e);
//         printf("%s => %s\n", str, e->value);
//     }
//     printf("\n");
//     printf("Capacity : %zu, Length : %zu\n", map.newer.length, map.newer.capacity);
// 
//     return 0;
// }
