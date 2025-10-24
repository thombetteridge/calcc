#include "gcstructures.h"

#include "common.h"

#include "gc/gc.h"
#include "gcstring.h"

#include <string.h>
#include <stdio.h>


void stack_push(Stack* stack, double x)
{
   if (stack->cap == 0) {
      stack->cap  = 8;
      stack->data = NEW(double,stack->cap);
   }

   if (stack->len == stack->cap) {
      stack->cap *= 2;
      stack->data = RENEW(double,stack->data, stack->cap );
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

void gc_string_join(GC_String* dest, const GC_String* src){
   gc_string_ensure(dest, src->len);
   memcpy(dest->data+dest->len, src->data, src->len);
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

static int gc_string_compare(GC_String* a, GC_String* b )
{
   return strcmp (a->data, b->data);
}


void gc_table_init(GC_Table* t)
{
    t->len = 0;
    t->cap = 8;
    t->entries = NEW(GC_TableEntry, t->cap);
}

static void gc_table_grow(GC_Table* t)
{
    t->cap *= 2;
    t->entries = RENEW(GC_TableEntry ,t->entries, t->cap);
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
    for (uint i = 0; i < t->len; i++) {
        if (strcmp(t->entries[i].key, key) == 0) {
            return t->entries[i].func;
        }
    }
    return NULL;
}

void gc_user_table_init(GC_Table_User_Words* t){
    t->len = 0;
    t->cap = 8;
    t->entries = NEW(GC_TableEntry_User_Words, t->cap);
}

static void gc_user_table_grow(GC_Table_User_Words* t)
{
    t->cap *= 2;
    t->entries = RENEW(GC_TableEntry_User_Words ,t->entries, t->cap);
}

void gc_user_table_add(GC_Table_User_Words* t, GC_String key, Token_Array arr){
    if (t->len >= t->cap) {
        gc_user_table_grow(t);
    }

    t->entries[t->len].key = key;
    t->entries[t->len].tokens = arr;
    t->len++;
}
Token_Array* gc_user_table_find(GC_Table_User_Words* t, GC_String* key){
    for (uint i = 0; i < t->len; i++) {
        if (gc_string_compare(&t->entries[i].key, key) == 0) {
            return &t->entries[i].tokens;
        }
    }
    return NULL;
}

