#include "gctable.h"

#include "gc/gc.h"

#include <string.h>

void gc_table_init(GC_Table* t)
{
    t->len = 0;
    t->cap = 8;
    t->entries = (GC_TableEntry*)GC_malloc(sizeof(GC_TableEntry) * t->cap);
}

static void gc_table_grow(GC_Table* t)
{
    t->cap *= 2;
    t->entries = (GC_TableEntry*)GC_realloc(t->entries, sizeof(GC_TableEntry) * t->cap);
}

void gc_table_add(GC_Table* t, const char* key, GC_Func func)
{
    if (t->len >= t->cap) {
        gc_table_grow(t);
    }

    t->entries[t->len].key = key;
    t->entries[t->len].func = func;
    t->len++;
}

GC_Func gc_table_find(GC_Table* t, const char* key)
{
    for (unsigned i = 0; i < t->len; i++) {
        if (strcmp(t->entries[i].key, key) == 0) {
            return t->entries[i].func;
        }
    }
    return NULL;
}
