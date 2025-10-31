#pragma once
#include "structures.h"

#include "common.h"
#include "lexer.h"
#include "tstring.h"

#include <stdio.h>
#include <stdlib.h>

void stack_push(Stack* stack, Value x)
{
   if (stack->len == stack->cap) {
      fprintf(stderr, "stack overflow\n");
      return;
   }
   stack->data[stack->len] = x;
   stack->len++;
}

bool stack_top(const Stack* stack, Value* out)
{
   if (stack->len == 0) {
      return false;
   }
   *out = stack->data[stack->len - 1];
   return true;
}

bool stack_pop(Stack* stack, Value* out)
{
   if (stack->len == 0) {
      return false;
   }
   stack_top(stack, out);
   stack->len--;
   return true;
}

bool stack_pop_2(Stack* stack, Value* y_out, Value* x_out)
{
   Value x = { 0 };
   Value y = { 0 };
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

void keywords_table_init(Keyword_Table* t)
{
   t->len     = 0;
   t->cap     = 32;
   t->entries = (Keyword_Table_Entry*)malloc(t->cap * sizeof(Keyword_Table_Entry));
}

static void keywords_table_grow(Keyword_Table* t)
{
   t->cap *= 2;
   t->entries = (Keyword_Table_Entry*)realloc(t->entries, t->cap * sizeof(Keyword_Table_Entry));
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

Keyword keywords_table_get(Keyword_Table* t, const char* key, size_t key_length)
{
   for (uint i = 0; i < t->len; i++) {
      size_t len = strlen(t->entries[i].key);
      if (len != key_length) { continue; }
      if (strncmp(t->entries[i].key, key, key_length) == 0) {
         return t->entries[i].func;
      }
   }
   return NULL;
}

void user_words_table_init(User_Words_Table* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = (User_Words_Table_Entry*)malloc(t->cap * sizeof(User_Words_Table_Entry));
}

static void user_words_table_grow(User_Words_Table* t)
{
   t->cap *= 2;
   t->entries = (User_Words_Table_Entry*)realloc(t->entries, t->cap * sizeof(User_Words_Table_Entry));
}

static Token_Array token_array_clone_flat(const Token_Array* a)
{
   Token_Array out;
   out.len  = a->len;
   out.cap  = a->len ? a->len : 1;
   out.data = (Token*)malloc(out.cap * sizeof(Token));
   memcpy(out.data, a->data, a->len * sizeof(Token));
   for (uint i = 0; i < out.len; ++i) {
      out.data[i].literal = string_clone(&a->data[i].literal);
   }
   return out;
}

static void token_array_free_deep(Token_Array* arr)
{
   for (uint i = 0; i < arr->len; ++i) {
      string_free(&arr->data[i].literal);
   }
   free(arr->data);
   arr->data = NULL;
   arr->len = arr->cap = 0;
}

void user_words_table_add(User_Words_Table* t, String key, Token_Array arr)
{
   // try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (string_compare(&t->entries[i].key, &key)) {
         token_array_free_deep(&t->entries[i].tokens);
         t->entries[i].tokens = token_array_clone_flat(&arr);
         return;
      }
   }

   // append new entry
   if (t->len >= t->cap) {
      user_words_table_grow(t);
   }
   t->entries[t->len].key    = string_clone(&key);
   t->entries[t->len].tokens = token_array_clone_flat(&arr);
   t->len++;
}

Token_Array* user_words_table_get(User_Words_Table* t, String* key)
{
   for (uint i = 0; i < t->len; i++) {
      if (string_compare(&t->entries[i].key, key)) {
         return &t->entries[i].tokens;
      }
   }
   return NULL;
}

void variable_table_init(Variable_Table* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = (Variable_Table_Entry*)malloc(t->cap * sizeof(Variable_Table_Entry));
}

static void variable_table_grow(Variable_Table* t)
{
   t->cap *= 2;
   t->entries = (Variable_Table_Entry*)realloc(t->entries, t->cap * sizeof(Variable_Table_Entry));
}

void variable_table_add(Variable_Table* t, String key, Value variable)
{
   // TODO need to copy Values!!!
   // try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (string_compare(&t->entries[i].key, &key)) {
         t->entries[i].variable = variable;
         return;
      }
   }
   // append new entry
   if (t->len >= t->cap) {
      variable_table_grow(t);
   }
   t->entries[t->len].key      = string_clone(&key);
   t->entries[t->len].variable = variable;
   t->len++;
}

Value* variable_table_get(Variable_Table* t, String* key)
{
   for (uint i = 0; i < t->len; i++) {
      if (string_compare(&t->entries[i].key, key)) {
         return &t->entries[i].variable;
      }
   }
   return NULL;
}