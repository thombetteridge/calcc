#include "gcstructures.h"

#include "common.h"

#include "gc/gc.h"
#include "gcstring.h"

#include <stdio.h>
#include <string.h>

void stack_push(Stack* stack, double x)
{
   if (stack->cap == 0) {
      stack->cap  = 8;
      stack->data = NEW(double, stack->cap);
   }

   if (stack->len == stack->cap) {
      stack->cap *= 2;
      stack->data = RENEW(double, stack->data, stack->cap);
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

void gc_string_init(GC_String* str)
{
   str->cap     = 32;
   str->len     = 0;
   str->data    = NEW_ATOMIC(char, str->cap);
   str->data[0] = '\0';
}

void gc_string_ensure(GC_String* str, uint add)
{
   if (str->len + add + 1 <= str->cap) {
      return;
   }
   while (str->len + add + 1 > str->cap) {
      str->cap *= 2;
   }
   str->data = RENEW(char, str->data, str->cap);
}

void gc_string_append(GC_String* str, const char* append, uint append_len)
{
   gc_string_ensure(str, append_len);
   memcpy(str->data + str->len, append, append_len);
   str->len += append_len;
   str->data[str->len] = '\0';
}

void gc_string_join(GC_String* dest, const GC_String* src)
{
   gc_string_ensure(dest, src->len);
   memcpy(dest->data + dest->len, src->data, src->len);
   dest->len += src->len;
   dest->data[dest->len] = '\0';
}

void gc_string_appendf(GC_String* str, const char* fmt, double x)
{
   char buf[64];
   int  n = snprintf(buf, sizeof(buf), fmt, x);
   if (n < 0) {
      return;
   }
   if ((size_t)n >= sizeof(buf)) {
      n = (int)(sizeof(buf) - 1);
   }
   gc_string_append(str, buf, (uint)n);
}

int gc_string_compare(GC_String* a, GC_String* b)
{
   if (a->len != b->len) {
      return -1;
   }

   return strncmp(a->data, b->data, a->len);
}

void gc_table_init(GC_Table* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = NEW(GC_TableEntry, t->cap);
}

static void gc_table_grow(GC_Table* t)
{
   t->cap *= 2;
   t->entries = RENEW(GC_TableEntry, t->entries, t->cap);
}

void gc_table_add(GC_Table* t, const char* key, GC_Func func)
{
   if (t->len >= t->cap) {
      gc_table_grow(t);
   }
   t->entries[t->len].key  = key;
   t->entries[t->len].func = func;
   t->len++;
}

GC_Func gc_table_find(GC_Table* t, const char* key)
{
   for (uint i = 0; i < t->len; i++) {
      if (strcmp(t->entries[i].key, key) == 0) {
         return t->entries[i].func;
      }
   }
   return NULL;
}

void gc_user_table_init(GC_Table_User_Words* t)
{
   t->len     = 0;
   t->cap     = 8;
   t->entries = NEW(GC_TableEntry_User_Words, t->cap);
}

static void gc_user_table_grow(GC_Table_User_Words* t)
{
   t->cap *= 2;
   t->entries = RENEW(GC_TableEntry_User_Words, t->entries, t->cap);
}

GC_String gc_string_clone(const GC_String* s)
{
   GC_String out;
   gc_string_init(&out);
   gc_string_append(&out, s->data, s->len);
   return out;
}

static Token_Array token_array_clone_flat(const Token_Array* a)
{
   Token_Array out;
   out.len  = a->len;
   out.cap  = a->len ? a->len : 1;
   out.data = NEW(Token, out.cap);
   memcpy(out.data, a->data, a->len * sizeof(Token));
   for (uint i = 0; i < out.len; ++i) {
      out.data[i].literal = gc_string_clone(&a->data[i].literal);
   }
   return out;
}

void gc_user_table_add(GC_Table_User_Words* t, GC_String key, Token_Array arr)
{
   // 1) try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (gc_string_compare(&t->entries[i].key, &key) == 0) {
         t->entries[i].tokens = token_array_clone_flat(&arr);
         return;
      }
   }

   // 2) append new entry
   if (t->len >= t->cap) {
      gc_user_table_grow(t);
   }
   t->entries[t->len].key    = gc_string_clone(&key);
   t->entries[t->len].tokens = token_array_clone_flat(&arr);
   t->len++;
}

bool gc_user_table_find(GC_Table_User_Words* t, GC_String* key, Token_Array** result)
{
   for (uint i = 0; i < t->len; i++) {
      if (gc_string_compare(&t->entries[i].key, key) == 0) {
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
   t->entries = NEW(Variable_TableEntry, t->cap);
}

static void variable_table_grow(Variable_Table* t)
{
   t->cap *= 2;
   t->entries = RENEW(Variable_TableEntry, t->entries, t->cap);
}

void variable_table_add(Variable_Table* t, GC_String key, double variable)
{
   // 1) try to overwrite existing entry
   for (uint i = 0; i < t->len; ++i) {
      if (gc_string_compare(&t->entries[i].key, &key) == 0) {
         t->entries[i].variable = variable;
         return;
      }
   }

   // 2) append new entry
   if (t->len >= t->cap) {
      variable_table_grow(t);
   }
   t->entries[t->len].key    = gc_string_clone(&key);
   t->entries[t->len].variable = variable;
   t->len++;
}

bool variable_table_find(Variable_Table* t, GC_String* key, double* variable)
{
   for (uint i = 0; i < t->len; i++) {
      if (gc_string_compare(&t->entries[i].key, key) == 0) {
         *variable = t->entries[i].variable;
         return true;
      }
   }
   *variable = 0;
   return false;
}