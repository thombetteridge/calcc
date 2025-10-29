#pragma once
#include "structures.h"

#include "common.h"
#include "lexer.h"
#include "tstring.h"

#include <stdio.h>
#include <stdlib.h>


void stack_push(Stack* stack, double x)
{
    if (stack->len == stack->cap){
        sprintf(stderr, "stack overflow\n");
        return;
    }

   stack->data[stack->len] = x;
   stack->len++;
}

bool stack_top(const Stack* stack, double* out)
{
   if (stack->len == 0) {
      return false;
   }
   *out = stack->data[stack->len - 1];
   return true;
}

bool stack_pop(Stack* stack, double* out)
{
   if (stack->len == 0) {
      return false;
   }
   stack_top(stack, out);
   stack->len--;
   return true;
}

bool stack_pop_2(Stack* stack, double* y_out, double* x_out)
{
   double x, y;
   if (!stack_pop(stack, &x)) {
      return false;
   }
   if (!stack_pop(stack, &y)) {
      return false;
   }
   *y_out = y;
   *x_out = x;
   return true;
}

static void keywords_table_grow(Keyword_Table* t)
{
   t->cap *= 2;
   t->entries =(Keyword_Table_Entry*) realloc( t->entries, t->cap* sizeof(Keyword_Table_Entry));
}

void keywords_table_add(Keyword_Table* t, const char* key, Keyword func)
{
   if (t->len >= t->cap) {
      keywords_table_grow(t);
   }
   t->entries[t->len].key  = key;
   t->entries[t->len].func = func;
   t->len++;
}

Keyword keywords_table_find(Keyword_Table* t, const char* key)
{
   for (uint i = 0; i < t->len; i++) {
      if (strcmp(t->entries[i].key, key) == 0) {
         return t->entries[i].func;
      }
   }
   return NULL;
}

void user_table_init(User_Words_Table* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = (User_Words_Table_Entry*)malloc( t->cap * sizeof(User_Words_Table_Entry));
}

static void user_table_grow(User_Words_Table* t)
{
   t->cap *= 2;
   t->entries = (User_Words_Table_Entry*)realloc( t->entries, t->cap * sizeof(User_Words_Table_Entry));
}


static Token_Array token_array_clone_flat(const Token_Array* a)
{
   Token_Array out;
   out.len  = a->len;
   out.cap  = a->len ? a->len : 1;
   out.data = (Token*)malloc( out.cap* sizeof(Token));
   memcpy(out.data, a->data, a->len * sizeof(Token));
   for (uint i = 0; i < out.len; ++i) {
      out.data[i].literal = string_clone(&a->data[i].literal);
   }
   return out;
}

void user_table_add(User_Words_Table* t, String key, Token_Array arr)
{
   // 1) try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (string_compare(&t->entries[i].key, &key)) {
         token_array_free( &t->entries[i].tokens);
         t->entries[i].tokens = token_array_clone_flat(&arr);
         return;
      }
   }

   // 2) append new entry
   if (t->len >= t->cap) {
      user_table_grow(t);
   }
   t->entries[t->len].key    = string_clone(&key);
   t->entries[t->len].tokens = token_array_clone_flat(&arr);
   t->len++;
}

bool user_table_find(User_Words_Table* t, String* key, Token_Array** result)
{
   for (uint i = 0; i < t->len; i++) {
      if (string_compare(&t->entries[i].key, key)) {
         *result = &t->entries[i].tokens;
         return true;
      }
   }
   *result = NULL;
   return false;
}



void variable_table_init(Variable_Table* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = (Variable_Table_Entry*)malloc( t->cap*sizeof(Variable_Table_Entry));
}

static void variable_table_grow(Variable_Table* t)
{
   t->cap *= 2;
   t->entries = (Variable_Table_Entry*)realloc( t->entries, t->cap*sizeof(Variable_Table_Entry));
}

void variable_table_add(Variable_Table* t, String key, double variable)
{
   // 1) try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (string_compare(&t->entries[i].key, &key) ) {
         t->entries[i].variable = variable;
         return;
      }
   }

   // 2) append new entry
   if (t->len >= t->cap) {
      variable_table_grow(t);
   }
   t->entries[t->len].key    = string_clone(&key);
   t->entries[t->len].variable = variable;
   t->len++;
}

bool variable_table_find(Variable_Table* t, String* key, double* variable)
{
   for (uint i = 0; i < t->len; i++) {
      if (string_compare(&t->entries[i].key, key)) {
         *variable = t->entries[i].variable;
         return true;
      }
   }
   *variable = 0;
   return false;
}