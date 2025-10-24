#pragma once

#include "common.h"
#include "lexer.h"

#include "gcstring.h"

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

/* ----------------------------------------- */


ARR_DEF(Token);

typedef struct {
    GC_String   key;
    Token_Array tokens;
} GC_TableEntry_User_Words;

typedef struct {
    GC_TableEntry_User_Words* entries;
    unsigned       len;
    unsigned       cap;
} GC_Table_User_Words;

void gc_user_table_init(GC_Table_User_Words* t);
void gc_user_table_add(GC_Table_User_Words* t, GC_String key, Token_Array arr);
Token_Array* gc_user_table_find(GC_Table_User_Words* t, GC_String* key);


