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
   uint           len;
   uint           cap;
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
   uint                      len;
   uint                      cap;
} GC_Table_User_Words;

void gc_user_table_init(GC_Table_User_Words* t);
void gc_user_table_add(GC_Table_User_Words* t, GC_String key, Token_Array arr);
bool gc_user_table_find(GC_Table_User_Words* t, GC_String* key, Token_Array** result);

typedef struct {
   GC_String key;
   double    variable;
} Variable_TableEntry;

typedef struct {
   Variable_TableEntry* entries;
   uint                 len;
   uint                 cap;
} Variable_Table;

void variable_table_init(Variable_Table* t);
void variable_table_add(Variable_Table* t, GC_String key, double variable);
bool variable_table_find(Variable_Table* t, GC_String* key, double* variable);
