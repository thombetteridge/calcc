#pragma once

#include "gc/gc.h"

#include <assert.h>
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

#define NEW(T, n) ((T*)GC_malloc(sizeof(T) * (n)))
#define NEW_ATOMIC(T, n) ((T*)GC_malloc(sizeof(T) * (n)))
#define RENEW(T, ptr, n) ((T*)GC_realloc((ptr), sizeof(T) * (n)))

#define ARRAY_DEF(Type) \
   typedef struct {     \
      Type* data;       \
      uint  len;        \
      uint  cap;        \
   } T##_Array;

#define ARRAY_INIT(Type, array) \
   array.len  = 0;              \
   array.cap  = 8;              \
   array.data = NEW(Type, array.cap)

#define ARRAY_PUSH(Type, array, value)                 \
   if (array.len == array.cap) {                       \
      array.cap *= 2;                                  \
      array.data = RENEW(Type, array.data, array.cap); \
   }                                                   \
   array.data[array.len] = value;                      \
   array.len++;

#define ARRAY_POP(Type, array) \
   = array[array.len - 1];     \
   --array.len;