#pragma once

#include "common.h"
#include "lexer.h"
#include <stddef.h>

// TODO! have not implenented yet!

typedef enum {
   V_NUMBER,
   V_QUOTE,
   V_LIST
} Val_Tag;

typedef struct Quote {
   Token* data;
   size_t len;
} Quote;

typedef struct List {
   struct Value* data;
   size_t        len;
   size_t        cap;
} List;

typedef struct Value {
   Val_Tag tag;
   union {
      double num;
      Quote  quote;
      List   list;
   };
} Value;