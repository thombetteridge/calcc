
#pragma once

#include "common.h"
#include "lexer.h"
#include "tstring.h"

typedef struct Stack {
   f64* data;
   u32  len;
   u32  cap;
} Stack;

void stack_push(Stack* stack, f64 x);
bool stack_top(const Stack* stack, f64* out);
bool stack_pop(Stack* stack, f64* out);
bool stack_pop_2(Stack* stack, f64* y_out, f64* x_out);

typedef void (*Keyword)(Stack*);

typedef struct {
   const char* key;
   Keyword     func;
} Keyword_Table_Entry;

typedef struct {
   Keyword_Table_Entry* entries;
   uint                 len;
   uint                 cap;
} Keyword_Table;

void    keywords_table_init(Keyword_Table* t);
void    keywords_table_add(Keyword_Table* t, const char* key, Keyword func);
Keyword keywords_table_get(Keyword_Table* t, const char* key, size_t key_length);

typedef struct {
   String      key;
   Token_Array tokens;
} User_Words_Table_Entry;

typedef struct {
   User_Words_Table_Entry* entries;
   uint                    len;
   uint                    cap;
} User_Words_Table;

void         user_words_table_init(User_Words_Table* t);
void         user_words_table_add(User_Words_Table* t, String key, Token_Array arr);
Token_Array* user_words_table_get(User_Words_Table* t, String* key);

typedef struct {
   String key;
   double variable;
} Variable_Table_Entry;

typedef struct {
   Variable_Table_Entry* entries;
   uint                  len;
   uint                  cap;
} Variable_Table;

void    variable_table_init(Variable_Table* t);
void    variable_table_add(Variable_Table* t, String key, double variable);
double* variable_table_get(Variable_Table* t, String* key);
