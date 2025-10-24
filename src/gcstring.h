#pragma once
#include "common.h"


typedef struct GC_String {
   char* data;
   uint  len;
   uint  cap;
} GC_String;

void gc_string_init(GC_String* str);
void gc_string_ensure(GC_String* str, uint add);
void gc_string_append(GC_String* str, const char* append, uint append_len);
void gc_string_appendf(GC_String* str, const char* fmt, double x);