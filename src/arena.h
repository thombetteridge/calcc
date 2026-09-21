#pragma once

#include <stddef.h>

typedef struct ArenaRegion ArenaRegion;

typedef struct Arena Arena;
struct Arena {
    ArenaRegion * head;
};

typedef struct ArenaMarker ArenaMarker;
struct ArenaMarker {
    ArenaRegion * parent;
    size_t        offset;
};


#define DEFAULT_REGION_SIZE 0x1000

void * arena_alloc(Arena * a, size_t size);
void   arena_clear(Arena * a);
void   arena_destroy(Arena * a);
void   arena_reserve(Arena * a, size_t cap);

ArenaMarker arena_mark(Arena * a);
void        arena_pop(Arena * a, ArenaMarker mark);