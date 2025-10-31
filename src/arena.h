#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t byte;

typedef struct Arena {
   byte*  buffer;
   size_t cap;
   size_t offset;
   size_t prev_offset;
} Arena;

Arena arena_new(byte* buffer, size_t buffer_size);

void* arena_alloc(Arena* a, size_t size);

void arena_pop(Arena* arena);

void arena_reset(Arena* arena);
