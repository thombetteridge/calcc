#pragma once

#include "common.h"
#include "lexer.h"

#include "gcstring.h"

#include <stddef.h>

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

void    gc_table_init(GC_Table* t);
void    gc_table_add(GC_Table* t, const char* key, GC_Func func);
GC_Func gc_table_find(GC_Table* t, const char* key);

/* ----------------------------------------- */

typedef struct {
   Token* data;
   uint   len;
   uint   cap;
} Token_Array;

void token_array_init(Token_Array* v);
void token_array_push(Token_Array* v, Token x);
void token_array_pop(Token_Array* v);

typedef struct {
   GC_String   key;
   Token_Array tokens;
} GC_TableEntry_User_Words;

typedef struct {
   GC_TableEntry_User_Words* entries;
   unsigned                  len;
   unsigned                  cap;
} GC_Table_User_Words;

void gc_user_table_init(GC_Table_User_Words* t);
void gc_user_table_add(GC_Table_User_Words* t, GC_String key, Token_Array arr);
bool gc_user_table_find(GC_Table_User_Words* t, GC_String* key, Token_Array** result);

_Static_assert(sizeof(uint) == 4, "uint must be 32-bit"); // adjust if needed

// Adjust names/order to your real definition:
_Static_assert(offsetof(Token_Array, data) == 0, "Token_Array.data offset mismatch");
_Static_assert(offsetof(Token_Array, len) > 0, "Token_Array.len offset mismatch");
_Static_assert(offsetof(Token_Array, cap) > 0, "Token_Array.cap offset mismatch");
_Static_assert(sizeof(Token_Array) >= sizeof(void*) + 2 * sizeof(uint),
    "Token_Array size too small");

_Static_assert(offsetof(GC_TableEntry_User_Words, key) == 0, "entry.key offset mismatch");
_Static_assert(offsetof(GC_TableEntry_User_Words, tokens) > 0, "entry.tokens offset mismatch");
