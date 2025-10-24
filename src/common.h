#pragma once

#include "gc/gc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

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

#define ARR_DEF(T)                                                       \
   typedef struct {                                                      \
      T*   data;                                                         \
      uint len;                                                          \
      uint cap;                                                          \
   } T##_Array;                                                          \
                                                                         \
   [[maybe_unused]] static inline void T##_Array_init(T##_Array* v)      \
   {                                                                     \
      v->len  = 0;                                                       \
      v->cap  = 8;                                                       \
      v->data = NEW(T, v->cap);                                          \
   }                                                                     \
                                                                         \
   [[maybe_unused]] static inline void T##_Array_push(T##_Array* v, T x) \
   {                                                                     \
      if (v->len == v->cap) {                                            \
         v->cap *= 2;                                                    \
         v->data = RENEW(T, v->data, v->cap);                            \
      }                                                                  \
      v->data[v->len++] = x;                                             \
   }                                                                     \
   [[maybe_unused]] static inline void T##_Array_pop(T##_Array* v)       \
   {                                                                     \
      v->len--;                                                          \
   }                                                                     \
   [[maybe_unused]] static inline T* T##_Array_at(T##_Array* v, uint i)  \
   {                                                                     \
      assert(i < v->len);                                                \
      return &v->data[i];                                                 \
   }
