#include "arena.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


static ArenaRegion * ArenaRegion_new(size_t cap) {
    assert(cap > sizeof(ArenaRegion));
    ArenaRegion * region = (ArenaRegion *)malloc(cap);
    region->buffer       = (char *)(region + 1);
    region->offset       = 0;
    region->capacity     = cap - sizeof(ArenaRegion);
    region->next         = NULL;
    return region;
}


static size_t align_forward(size_t ptr, size_t align) {
    size_t const modulo = ptr & (align - 1);
    return modulo ? (ptr + (align - modulo)) : ptr;
}

void arena_reserve(Arena * self, size_t cap) {
    if (self->head == NULL) {
        ArenaRegion * node = ArenaRegion_new(cap);
        self->head         = node;
        return;
    }

    if (cap < self->head->capacity)
        return;

    size_t        new_cap = align_forward(cap, sizeof(void *));
    ArenaRegion * node    = ArenaRegion_new(new_cap);
    node->next            = self->head;
    self->head            = node;
}

void * arena_alloc(Arena * self, size_t bytes) {
    if (self->head == NULL) {
        ArenaRegion * node = ArenaRegion_new(DEFAULT_REGION_SIZE);
        self->head         = node;
    }

    if (self->head->offset + bytes > self->head->capacity) {
        size_t new_cap     = self->head->capacity * 2 > bytes ? self->head->capacity * 2 : bytes;
        new_cap            = align_forward(new_cap, sizeof(void *));
        ArenaRegion * node = ArenaRegion_new(new_cap);
        node->next         = self->head;
        self->head         = node;
    }

    void * ptr = self->head->buffer + self->head->offset;
    memset(ptr, 0, bytes);
    self->head->offset = align_forward(self->head->offset + bytes, sizeof(void *));
    return ptr;
}


void arena_clear(Arena * self) {
    if (self->head == NULL)
        return;

    ArenaRegion * node = self->head->next;
    while (node) {
        ArenaRegion * next = node->next;
        free(node);
        node = next;
    }

    self->head->offset = 0;
    self->head->next   = NULL;
}

void arena_destroy(Arena * self) {
    arena_clear(self);
    free(self->head);
    self->head = NULL;
}

ArenaMarker arena_mark(Arena * self) {
    if (self->head == NULL)
        return (ArenaMarker) { 0 };
    return (ArenaMarker) { .parent = self->head, .offset = self->head->offset };
}

void arena_pop(Arena * self, ArenaMarker mark) {
    if (self->head == NULL)
        return;
    if (self->head != mark.parent)
        return;

    if (mark.offset > self->head->offset)
        return;

    self->head->offset = mark.offset;
}
