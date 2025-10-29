#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t byte;

#define KB(x) x * 1024ull
#define MB(x) KB(x) * 1024ull
#define GB(x) MB(x) * 1024ull

typedef struct Arena {
   byte*  buffer;
   size_t cap;
   size_t offset;
   size_t prev_offset;
} Arena;

inline Arena arena_new(byte* buffer, size_t buffer_size)
{
   return (Arena) { .buffer = buffer, .cap = buffer_size };
}

inline static size_t align_forward(size_t ptr)
{
   static const size_t alignment = sizeof(void*);
   size_t const        modulo    = ptr & (alignment - 1);
   return ptr + alignment - modulo;
}

inline void* arena_alloc(Arena* arena, size_t size)
{
   if (arena->offset + size > arena->cap) {
      return NULL;
   }
   void* ptr          = arena->buffer + arena->offset + size;
   arena->prev_offset = arena->offset;
   arena->offset      = align_forward(arena->offset + size);
   return ptr;
}

inline void arena_pop(Arena* arena)
{
   if (arena->prev_offset == 0) {
      return;
   }
   size_t const length = arena->offset - arena->prev_offset;
   arena->offset       = arena->prev_offset;
   memset(arena->buffer + arena->offset, 0, length);
}

inline void arena_reset(Arena* arena)
{
   memset(arena->buffer, 0, arena->offset);
   arena->offset = 0;
}
