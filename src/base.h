#pragma once
<<<<<<< Updated upstream

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef unsigned uint;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

#define KB(x) x * 1024ull
#define MB(x) KB(x) * 1024ull
#define GB(x) MB(x) * 1024ull

typedef struct String {
   char*  data;
   size_t len;
} String;

typedef enum Token_Type {
   ILLEGAL,
   EOF_,

   NUMBER,
   WORD,

   USCORE, // _

   // OPERATORS
   ASSIGN,   // =
   PLUS,     // +
   MINUS,    // -
   ASTRIX,   // *
   SLASH,    // /
   CARET,    // ^
   PERCENT,  // %
   DOLLAR,   // $
   HASH,     // #
   BANG,     // !
   AT,       // @
   AND_,     // &
   PIPE,     // |
   TILDA,    // ~
   BTICK,    // `
   QUESTION, // ?
   DOT,      // .
   LT,       // <
   GT,       // >

   // DIAGRAPGHS
   EQ,        // ==
   NOT_EQ_,   // !=
   LT_EQ,     // <=
   GT_EQ,     // >=
   ARROW,     // ->
   FAT_ARROW, // =>

   // DELIMINATORS
   COMMA,        // ,
   COLON,        // :
   SEMICOLON,    // ;
   LPAREN,       // (
   RPAREN,       // )
   LBRACE,       // {
   RBRACE,       // }
   LBRACKET,     // ]
   RBRACKET,     // [
   DOUBLE_QUOTE, // "
   QUOTE,        // '
} Token_Type;

typedef struct Token {
   Token_Type type;
   size_t     pos;
   String     literal;
} Token;

typedef struct Token_Array {
   Token* data;
   size_t len;
   size_t cap;
} Token_Array;

typedef struct Quote {
   Token* data;
   size_t len;
} Quote;

typedef struct List {
   struct Value* data;
   size_t        len;
   size_t        cap;
} List;

typedef enum {
   V_INVALID,
   V_NUMBER,
   V_QUOTE,
   V_LIST
} Val_Tag;

typedef struct Value {
   union {
      List   list;
      Quote  quote;
      double num;
   };
   Val_Tag tag;
} Value;

typedef struct Stack {
   Value* data;
   size_t len;
   size_t cap;
} Stack;
typedef void (*Keyword)(Stack*);

typedef struct {
   const char* key;
   Keyword     func;
} Keyword_Table_Entry;

typedef struct {
   Keyword_Table_Entry* entries;
   uint                 len;
   uint                 cap;
} Keyword_Table;

typedef struct {
   String      key;
   Token_Array tokens;
} User_Words_Table_Entry;

typedef struct {
   User_Words_Table_Entry* entries;
   uint                    len;
   uint                    cap;
} User_Words_Table;

typedef struct {
   String key;
   Value  variable;
} Variable_Table_Entry;

typedef struct {
   Variable_Table_Entry* entries;
   uint                  len;
   uint                  cap;
} Variable_Table;

typedef struct String_Builder {
   char*  data;
   size_t len;
   size_t cap;
} String_Builder;

void token_array_init(Token_Array* arr);
void token_array_push(Token_Array* arr, Token x);
void token_array_pop(Token_Array* arr);
void token_array_free(Token_Array* arr);

// WARNING temporary!!
const char* string_c_str(const String* s);

bool   string_compare(String const* a, String const* b);
String string_from_c_str(const char* c_str, char* buffer);

// you own it!
String string_clone(String* str);
void   string_free(String* str);

void string_builder_append(String_Builder* str_builder, const char* str, size_t len);
void string_builder_reset(String_Builder* str);

Quote quote_clone_deep(const Quote* quote);

void stack_push(Stack* stack, Value x);
bool stack_top(const Stack* stack, Value* out);
bool stack_pop(Stack* stack, Value* out);
bool stack_pop_2(Stack* stack, Value* y_out, Value* x_out);

void    keywords_table_init(Keyword_Table* t);
void    keywords_table_add(Keyword_Table* t, const char* key, Keyword func);
Keyword keywords_table_get(Keyword_Table* t, const char* key, size_t key_length);

void         user_words_table_init(User_Words_Table* t);
void         user_words_table_add(User_Words_Table* t, String key, Token_Array arr);
Token_Array* user_words_table_get(User_Words_Table* t, String* key);

void   variable_table_init(Variable_Table* t);
void   variable_table_add(Variable_Table* t, String key, Value variable);
Value* variable_table_get(Variable_Table* t, String* key);
=======
>>>>>>> Stashed changes
