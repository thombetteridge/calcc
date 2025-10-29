#pragma once

#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct String {
   char* data;
   uint  len;
} String;

// WARNING temporary!!
const char* string_c_str(const String* s)
{
   static char buffer[1024];
   static u32  offset = 0;

   u32 available = sizeof(buffer) - offset;
   if (s->len + 1 > available) {
      offset = 0;
   }

   char* ptr      = buffer + offset;
   u32   copy_len = s->len;
   if (copy_len >= sizeof(buffer)) {
      copy_len = sizeof(buffer) - 1;
   }

   memcpy(ptr, s->data, copy_len);
   ptr[copy_len] = '\0';

   offset += copy_len + 1;

   return ptr;
}

bool string_compare(String const* a, String const* b)
{
   if (a->len != b->len) {
      return false;
   }

   return (memcmp(a->data, b->data, a->len) == 0);
}

String string_from_c_str(const char* c_str, char* buffer)
{
   u32 length = strlen(c_str);
   memcpy(buffer, c_str, length);
   return (String) { .data = buffer, .len = length };
}

// you own it!
String string_clone(String* str)
{
   char* buffer = (char*)malloc(str->len * sizeof(char));
   memcpy(buffer, str->data, str->len);
   return (String) { .data = buffer, .len = str->len };
}

void string_free(String* str)
{
   free(str->data);
   str->len = 0;
}

typedef struct String_Builder {
   char* data;
   uint  len;
   uint cap;
} String_Builder;


void string_builder_append(String_Builder* str_builder, const char* str, u32 len){
   if (str_builder->len + len > str_builder->cap){
      sprintf(stderr, "strbuilder overflow");
      return;
   }

   memcpy(str_builder->data + len, str, len);
   str_builder->len += len;
}

void string_builder_reset(String_Builder* str){
   memset(str->data, 0, str->len);
   str->len=0;
}