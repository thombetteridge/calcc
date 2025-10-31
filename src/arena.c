#include "arena.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

Arena arena_new(byte* buffer, size_t buffer_size)
{
   return (Arena) { .buffer = buffer, .cap = buffer_size };
}

static size_t align_forward(size_t ptr)
{
   static const size_t alignment = sizeof(void*);
   size_t const        modulo    = ptr & (alignment - 1);
   return modulo ? (ptr + (alignment - modulo)) : ptr;
}

void* arena_alloc(Arena* a, size_t size)
{
   if (a->offset + size > a->cap) {
      fprintf(stderr, "arena overflow\n");
      return NULL;
   }
   void* ptr      = a->buffer + a->offset;
   a->prev_offset = a->offset;
   a->offset      = align_forward(a->offset + size);
   return ptr;
}

void arena_pop(Arena* arena)
{
   if (arena->prev_offset == 0) {
      return;
   }
   size_t const length = arena->offset - arena->prev_offset;
   arena->offset       = arena->prev_offset;
   memset(arena->buffer + arena->offset, 0, length);
}

void arena_reset(Arena* arena)
{
   memset(arena->buffer, 0, arena->offset);
   arena->offset = 0;
}
