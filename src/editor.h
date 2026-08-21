#ifndef EDITOR_H
#define EDITOR_H

#include "base.h"
#include "platform.h"


// #define BACKGROUND_COLOUR RGB_COLOUR(253, 246, 247)
// #define LINE_COLOUR       RGB_COLOUR(238, 232, 213)
// // #define TEXT_COLOUR       RGB_COLOUR(28, 50, 57)
// #define TEXT_COLOUR       RGB_COLOUR(8, 30, 37)
// #define CURSOR_COLOUR     RGB_COLOUR(42, 161, 152)

#define CURSOR_COLOUR     0x0969da
#define TEXT_COLOUR       0x1f2328
#define BACKGROUND_COLOUR 0xffffff
#define LINE_COLOUR       RGB_COLOUR(234, 238, 242)

#define OFFSET_X (GLYPH_WIDTH / 2.0f)
#define OFFSET_Y (GLYPH_HEIGHT / 2.0f)

typedef struct {
    bool shift;
    bool ctrl;
    bool alt;
} KeyMod;

typedef struct {
    usize x;
    usize y;
} Pos;

typedef struct {
    usize begin;
    usize end;
} Line;

typedef struct {
    Line * ptr;
    usize  len, cap;

    Allocator * allocator;
} Lines;

typedef struct {
    char * ptr;
    usize  len, cap;

    Allocator * allocator;
} Data;


// TODO, wire these up!
typedef enum {
    ED_NONE      = 0,
    ED_NO_CURSOR = 1 << 0,
    ED_NO_LINE   = 1 << 1,
    ED_READ_ONLY = 1 << 2,
} EditorFlags;

typedef u32 EditorFlags_t;


// TODO do we need to save the text window size & pos?
typedef struct {
    Data  data;
    Lines lines;

    usize cursor;
    usize desired_col;
    usize anchor;

    usize scroll_offset_y;
    usize scroll_offset_x;

    EditorFlags_t flags; // Doesnt do anything yet

    Allocator * allocator;

} Editor;



Editor ed_init(Allocator * a);
void   ed_deinit(Editor * ed);
void   ed_render(Editor * ed, surface_t * sur, i32 x, i32 y, i32 w, i32 h);
bool   ed_edit_key(Editor * ed, PKey key, KeyMod mod);
bool   ed_open_file(Editor * ed, char const * file_path);
bool   ed_write_file(Editor * ed, char const * file_path);
void   ed_push_text(Editor * ed, char const * text, usize text_len);
void   ed_clear(Editor * ed);

void ed_mouse_click(Editor * ed, i32 x, i32 y);


#endif