#pragma once

#include "common.h"


/* ----------------------------------------- */

typedef struct Stack {
   double* data;
   uint    len;
   uint    cap;
} Stack;

void stack_push(Stack* stack, double x);
bool stack_top(const Stack* stack, double* out);
bool stack_pop(Stack* stack, double* out);
bool stack_pop_2(Stack* stack, double* y_out, double* x_out);


/* ----------------------------------------- */

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


/*--------------------------------------------*/

typedef struct GC_String {
   char* data;
   uint  len;
   uint  cap;
} GC_String;

void gc_string_init(GC_String* str);
void gc_string_ensure(GC_String* str, uint add);
void gc_string_append(GC_String* str, const char* append, uint append_len);
void gc_string_appendf(GC_String* str, const char* fmt, double x);