#include "arena.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

void arena_init(arena_t *a, byte_t *buffer, size_t buffer_size)
{
   a->len = 0;
   a->cap = buffer_size;
   a->ptr = buffer;
}

static size_t align_forward(size_t ptr, size_t align)
{
   size_t const modulo = ptr & (align - 1);
   return modulo ? (ptr + (align - modulo)) : ptr;
}

static inline void  memzero(void *ptr, size_t n)
{
   memset(ptr, 0, n);
}

void *arena_push(arena_t *a, size_t size)
{
   if (a->len + size > a->cap) {
      fprintf(stderr, "arena overflow\n");
      assert(0);
   }
   void *ptr = a->ptr + a->len;
   memzero(ptr, size);
   a->len = align_forward(a->len + size, sizeof(void*));
   return ptr;
}

size_t arena_mark(arena_t a) {
   return a.len;
}

void arena_pop(arena_t *a, size_t mark)
{
   assert(mark <= a->len);
   a->len = mark;
}

void arena_reset(arena_t *a)
{
   a->len = 0;
}
