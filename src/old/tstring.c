#include "tstring.h"

const char* string_c_str(const String* s)
{
   static char   buffer[1024];
   static size_t offset = 0;

   size_t available = sizeof(buffer) - offset;
   if (s->len + 1 > available) {
      offset = 0;
   }

   char*  ptr      = buffer + offset;
   size_t copy_len = s->len;
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
   size_t length = strlen(c_str);
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

void string_builder_append(String_Builder* str_builder, const char* str, size_t len)
{
   if (str_builder->len + len > str_builder->cap) {
      fprintf(stderr, "strbuilder overflow");
      return;
   }

   memcpy(str_builder->data + str_builder->len, str, len);
   str_builder->len += len;
}

void string_builder_reset(String_Builder* str)
{
   memset(str->data, 0, str->len);
   str->len = 0;
}