#include "map.h"

static char grave_value = {0};

// Maybe this should not be in heap ??
void init_map(Map *m)
{
    m->len = 0;
    m->cap = 0;
    m->entries = NULL;
}

void free_map(Map *m) {
    for (size_t i = 0; i < m->cap; ++i)
    {
        Entry *e = &m->entries[i];
        if (e->value == NULL || is_grave(e->value))
            continue;

        free(e->value);
    }
    free(m->entries);
}

// x
// [x,y,@,x,-]
//
// If the hash is the same as entry[i] -> return entry[i]
// If the value is NULL and there is grave return grave
//  , otherwise return NULL
// If the value is Grave and set grave to current and go next,

static Entry *find_entry(Entry *entries, int32 hash, size_t cap)
{
    if(cap == 0) return NULL;

    int32 idx = hash % cap;
    // int32 idx = 1;
    Entry *grave = NULL;
    for (;;)
    {
        Entry *e = &entries[idx];
        if (e->value != NULL)
        {
            if (is_grave(e->value))
            {
                if (grave == NULL)
                    grave = e;
            }
            else if (e->hash == hash)
                return e;
        }
        else
        {
            if (grave != NULL)
                return grave;
            return e;
        }

        idx = (idx + 1) % cap;
    }
}

static void adjust_cap(Map *m, size_t new_cap)
{
    Entry *new_entries = calloc(new_cap, sizeof(Entry));
    Entry *old_entries = m->entries;
    for (size_t i = 0; i < new_cap; ++i)
    {
        Entry *e = &new_entries[i];
        e->value = NULL;
        e->hash = 0;
    }

    for (size_t i = 0; i < m->cap; ++i)
    {
        Entry *old_entry = &old_entries[i];
        if (old_entry == NULL)
            continue;

        Entry *new_entry = find_entry(new_entries, old_entry->hash, new_cap);
        new_entry->hash = old_entry->hash;
        new_entry->value = old_entry->value;
    }
    free(old_entries);
    m->entries = new_entries;
}

void map_set(Map *m, Key k, Value v)
{
    if (m->len + 1 > (m->cap * CAP_FACTOR))
    {
        size_t new_cap = GROW_CAPACITY(m->cap);
        adjust_cap(m, new_cap);
        m->cap = new_cap;
    }
    // Hash the key
    int32 hash = fnv_32a_str(k, strlen(k));
    Entry *e = find_entry(m->entries, hash, m->cap);
    if (e->value == NULL)
        m->len++;

    e->hash = hash;

    const size_t nbytes = sizeof(char) * strlen(v);
    char *x = calloc(1, nbytes);
    memcpy(x, v, nbytes);
    e->value = x;
}

bool map_get(Map *m, Key k, Value *v)
{
    int32 hash = fnv_32a_str(k, strlen(k));
    Entry *e = find_entry(m->entries, hash, m->cap);
    if (e == NULL || e->value == NULL || is_grave(e->value))
        return false;

    *v = e->value;
    return true;
}

bool map_delete(Map *m, char *k)
{
    int32 hash = fnv_32a_str(k, strlen(k));
    Entry *e = find_entry(m->entries, hash, m->cap);
    if (e == NULL || e->value == NULL || is_grave(e->value))
        return false;

    free(e->value);
    set_grave(e->value);
    return true;
}

void print_map(Map *m)
{
    printf("{\n");
    for (size_t i = 0; i < m->cap; ++i)
    {
        Entry *e = &m->entries[i];
        if (e->value == NULL || is_grave(e->value))
            continue;

        printf("%du => ", e->hash);
        printf("%s,\n", e->value);
    }
    printf("}\n");
}
