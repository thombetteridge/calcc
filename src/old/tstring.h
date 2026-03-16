#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct String {
   char*  data;
   size_t len;
} String;

// WARNING temporary!!
const char* string_c_str(const String* s);

bool string_compare(String const* a, String const* b);
String string_from_c_str(const char* c_str, char* buffer);

// you own it!
String string_clone(String* str);
void string_free(String* str);

typedef struct String_Builder {
   char*  data;
   size_t len;
   size_t cap;
} String_Builder;

void string_builder_append(String_Builder* str_builder, const char* str, size_t len);
void string_builder_reset(String_Builder* str);