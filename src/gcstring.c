#include "gcstring.h"

#include "gc/gc.h"

#include <string.h>
#include <stdio.h>

void gc_string_init(GC_String* str)
{
   str->data    = (char*)GC_malloc(32);
   str->len     = 0;
   str->cap     = 32;
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
   str->data = (char*)GC_realloc(str->data, str->cap);
}

void gc_string_append(GC_String* str, const char* append, uint append_len)
{
   gc_string_ensure(str, append_len);
   memcpy(str->data + str->len, append, append_len);
   str->len += append_len;
   str->data[str->len] = '\0';
}

void gc_string_appendf(GC_String* str, const char* fmt, double x)
{
   char buf[64];
   int  n = snprintf(buf, sizeof(buf), fmt, x);
   if (n < 0) {
      return; // ignore on error
   }
   if ((size_t)n >= sizeof(buf)) {
      n = (int)(sizeof(buf) - 1);
   }
   gc_string_append(str, buf, (uint)n);
}