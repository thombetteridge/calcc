#pragma once

#include <stddef.h>

typedef struct ArenaRegion ArenaRegion;
struct ArenaRegion {
    char *        buffer;
    size_t        offset;
    size_t        capacity;
    ArenaRegion * next;
};

typedef struct Arena Arena;
struct Arena {
    ArenaRegion * head;
};

typedef struct ArenaMarker ArenaMarker;
struct ArenaMarker {
    ArenaRegion * parent;
    size_t        offset;
};


#define DEFAULT_REGION_SIZE 0x10000 // 64kb

#define arena_push(self, Type, count) (Type *)arena_alloc(self, sizeof(Type) * count)

void * arena_alloc(Arena * self, size_t size);
void   arena_clear(Arena * self);
void   arena_destroy(Arena * self);
void   arena_reserve(Arena * self, size_t cap);

ArenaMarker arena_mark(Arena * self);
void        arena_pop(Arena * self, ArenaMarker mark);