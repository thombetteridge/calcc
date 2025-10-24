#pragma once

#include "gcstack.h"

// simple string->function lookup table
typedef void (*GC_Func)(Stack*);

typedef struct {
    const char* key;
    GC_Func     func;
} GC_TableEntry;

typedef struct {
    GC_TableEntry* entries;
    unsigned       len;
    unsigned       cap;
} GC_Table;

void gc_table_init(GC_Table* t);
void gc_table_add(GC_Table* t, const char* key, GC_Func func);
GC_Func gc_table_find(GC_Table* t, const char* key);
