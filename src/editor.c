#include "editor.h"
#include "base.h"
#include "platform.h"

#include "errno.h"
#include <stdio.h>
#include <string.h>


#define VALIDATE_CURSOR()                   \
    do {                                    \
        ENSURE(ed->cursor <= ed->data.len); \
    } while (0)


static usize
ed_current_line(Editor const * ed)
{
    VALIDATE_CURSOR();
    ENSURE(ed->lines.len >= 1);

    for (iterate(i, ed->lines.len)) {
        if (ed->lines.ptr[i].begin <= ed->cursor && ed->cursor <= ed->lines.ptr[i].end) {
            return i;
        }
    }
    return 0;
}


static usize
ed_current_column(Editor const * ed, usize cur_line)
{
    VALIDATE_CURSOR();
    ENSURE(cur_line < ed->lines.len);
    return ed->cursor - ed->lines.ptr[cur_line].begin;
}


static void ed_set_desired_col(Editor * ed)
{
    usize const line = ed_current_line(ed);
    ed->desired_col  = ed_current_column(ed, line);
}


static bool
is_cursor_on_line(Editor const * ed, Line line)
{
    return (ed->cursor >= line.begin && ed->cursor <= line.end);
}


static Pos
ed_find_cursor_pos(Editor const * ed)
{
    usize cursor_x = 0;
    usize cursor_y = 0;

    for (iterate(i, ed->lines.len)) {
        Line const line = arr_at(&ed->lines, i);

        if (is_cursor_on_line(ed, line)) {
            cursor_x = i;
            cursor_y = ed->cursor - line.begin;
            break;
        }
    }
    return (Pos) { cursor_x, cursor_y };
}


static bool
is_between(usize x, usize lo, usize hi)
{
    return (x >= lo && x < hi);
}


static bool
ed_is_selection(Editor const * ed)
{
    return ed->cursor != ed->anchor;
}


static void
move_char_back_one(char * begin, char * end)
{
    memmove(begin + 1, begin, end - begin);
}


static void
ed_compute_lines(Editor * ed)
{
    arr_clear(&ed->lines);

    usize begin = 0;

    for (iterate(i, ed->data.len)) {
        if (ed->data.ptr[i] == '\n') {
            Line const line = { .begin = begin, .end = i };
            arr_push(&ed->lines, line);
            begin = i + 1;
        }
    }

    Line final = { .begin = begin, .end = ed->data.len };

    arr_push(&ed->lines, final);
}


static void
ed_insert_char(Editor * ed, char c)
{
    arr_reserve(&ed->data, ed->data.len + 1);

    move_char_back_one(ed->data.ptr + ed->cursor, ed->data.ptr + ed->data.len);
    ed->data.len += 1;

    arr_at(&ed->data, ed->cursor) = c;
    ed->cursor += 1;

    ed->anchor = ed->cursor;
    ed_compute_lines(ed);

    ed_set_desired_col(ed);
}


static void
ed_insert_newline(Editor * ed)
{
    ed_insert_char(ed, '\n');
    VALIDATE_CURSOR();
}


static void
ed_insert_tab(Editor * ed)
{
    u32 const tab_size = 4;
    for (iterate(_, tab_size))
        ed_insert_char(ed, ' ');
    VALIDATE_CURSOR();
}


static void
ed_select_all(Editor * ed)
{
    ed->anchor = 0;
    ed->cursor = ed->data.len;
}


static void
ed_delete_selection(Editor * ed)
{
    usize const begin = Min(ed->anchor, ed->cursor);
    usize const end   = Max(ed->anchor, ed->cursor);

    if (begin == end)
        return;

    char *      dst = ed->data.ptr + begin;
    char *      src = ed->data.ptr + end;
    usize const len = ed->data.len - end;

    memmove(dst, src, len);

    ed->data.len -= (end - begin);

    ed->cursor = begin;
    ed->anchor = begin;

    ed_compute_lines(ed);

    VALIDATE_CURSOR();
}


static void
ed_backspace(Editor * ed)
{
    if (ed->cursor != ed->anchor) {
        ed_delete_selection(ed);
        return;
    }

    if (ed->cursor == 0)
        return;

    char * begin = ed->data.ptr + ed->cursor - 1;
    char * end   = ed->data.ptr + ed->data.len;

    memmove(begin, begin + 1, end - (begin + 1));

    ed->data.len -= 1;
    ed->cursor -= 1;

    ed->anchor = ed->cursor;

    ed_compute_lines(ed);

    ed_set_desired_col(ed);

    VALIDATE_CURSOR();
}


static void
ed_delete_line(Editor * ed, usize line)
{
    ENSURE(line < ed->lines.len);

    Line const line_to_delete = ed->lines.ptr[line];
    bool const has_next       = line + 1 < ed->lines.len;
    bool const has_prev       = line > 0;

    usize const begin = has_next ? line_to_delete.begin : (has_prev ? line_to_delete.begin - 1 : line_to_delete.begin);
    usize const end   = has_next ? line_to_delete.end + 1 : line_to_delete.end;

    memmove(ed->data.ptr + begin, ed->data.ptr + end, ed->data.len - end);
    ed->data.len -= (end - begin);

    ed->cursor = Min(begin, ed->data.len);
    ed->anchor = ed->cursor;

    ed_compute_lines(ed);

    VALIDATE_CURSOR();
}


static bool
is_word_delim(char c)
{
    return isspace(c) || ispunct(c);
}


static void
ed_move_char_left(Editor * ed, bool selecting)
{
    if (ed->cursor > 0) {
        ed->cursor -= 1;
        ed_set_desired_col(ed);
    }

    if (!selecting)
        ed->anchor = ed->cursor;

    VALIDATE_CURSOR();
}


static void
ed_move_word_left(Editor * ed, bool selecting)
{
    // Skip delims
    while (ed->cursor > 0 && is_word_delim(ed->data.ptr[ed->cursor - 1]))
        ed->cursor -= 1;

    // Skip word chars
    while (ed->cursor > 0 && !is_word_delim(ed->data.ptr[ed->cursor - 1]))
        ed->cursor -= 1;

    ed_set_desired_col(ed);

    if (!selecting)
        ed->anchor = ed->cursor;

    VALIDATE_CURSOR();
}


static void
ed_move_char_right(Editor * ed, bool selecting)
{
    if (ed->cursor < ed->data.len) {
        ed->cursor += 1;
        ed_set_desired_col(ed);
    }

    if (!selecting)
        ed->anchor = ed->cursor;

    VALIDATE_CURSOR();
}


static void
ed_move_word_right(Editor * ed, bool selecting)
{
    // Skip delims
    while (ed->cursor < ed->data.len && is_word_delim(ed->data.ptr[ed->cursor]))
        ed->cursor += 1;

    // Skip word chars
    while (ed->cursor < ed->data.len && !is_word_delim(ed->data.ptr[ed->cursor]))
        ed->cursor += 1;

    ed_set_desired_col(ed);

    if (!selecting)
        ed->anchor = ed->cursor;

    VALIDATE_CURSOR();
}


static void
ed_move_up(Editor * ed, bool selecting)
{
    usize const line = ed_current_line(ed);
    // usize const column = ed_current_column(ed, line);
    if (line > 0) {
        ed->cursor = ed->lines.ptr[line - 1].begin + ed->desired_col;

        if (ed->cursor > ed->lines.ptr[line - 1].end)
            ed->cursor = ed->lines.ptr[line - 1].end;
    }

    if (!selecting)
        ed->anchor = ed->cursor;

    VALIDATE_CURSOR();
}


static void
ed_move_down(Editor * ed, bool selecting)
{
    usize const line = ed_current_line(ed);
    // usize const column = ed_current_column(ed, line);
    if (line < ed->lines.len - 1) {
        ed->cursor = ed->lines.ptr[line + 1].begin + ed->desired_col;
        if (ed->cursor > ed->lines.ptr[line + 1].end) {
            ed->cursor = ed->lines.ptr[line + 1].end;
        }
    }

    if (!selecting)
        ed->anchor = ed->cursor;


    VALIDATE_CURSOR();
}


static void
ed_clipboard_paste(Editor * ed)
{
    usize        text_length;
    char const * text = p_read_clipboard(&text_length);
    if (text == NULL)
        return;
    if (ed_is_selection(ed)) {
        ed_delete_selection(ed);
    }
    // text_length includes null terminator ( i think lol )
    for (iterate(i, text_length - 1)) {
        ed_insert_char(ed, text[i]);
    }
}


static void
ed_clipboard_copy(Editor * ed)
{
    if (ed->cursor != ed->anchor) {
        usize const begin = Min(ed->cursor, ed->anchor);
        usize const end   = Max(ed->cursor, ed->anchor);
        p_write_clipboard(&ed->data.ptr[begin], (u32)(end - begin));
    }
    else {
        // no selection copy whole line
        Line cur_line = ed->lines.ptr[ed_current_line(ed)];
        p_write_clipboard(&ed->data.ptr[cur_line.begin], (u32)(cur_line.end - cur_line.begin));
    }
}


static void
ed_clipboard_cut(Editor * ed)
{
    ed_clipboard_copy(ed);

    if (ed->cursor != ed->anchor) {
        ed_delete_selection(ed);
    }
    else {
        // no selection delete line
        usize line = ed_current_line(ed);
        ed_delete_line(ed, line);
    }
}


/***********************************************************************************************************************
__/\\\\\\\\\\\\\____/\\\________/\\\__/\\\\\\\\\\\\\____/\\\______________/\\\\\\\\\\\________/\\\\\\\\\_______________
 _\/\\\/////////\\\_\/\\\_______\/\\\_\/\\\/////////\\\_\/\\\_____________\/////\\\///______/\\\////////_______________
  _\/\\\_______\/\\\_\/\\\_______\/\\\_\/\\\_______\/\\\_\/\\\_________________\/\\\_______/\\\/_______________________
   _\/\\\\\\\\\\\\\/__\/\\\_______\/\\\_\/\\\\\\\\\\\\\\__\/\\\_________________\/\\\______/\\\________________________
    _\/\\\/////////____\/\\\_______\/\\\_\/\\\/////////\\\_\/\\\_________________\/\\\_____\/\\\_______________________
     _\/\\\_____________\/\\\_______\/\\\_\/\\\_______\/\\\_\/\\\_________________\/\\\_____\//\\\_____________________
      _\/\\\_____________\//\\\______/\\\__\/\\\_______\/\\\_\/\\\_________________\/\\\______\///\\\__________________
       _\/\\\______________\///\\\\\\\\\/___\/\\\\\\\\\\\\\/__\/\\\\\\\\\\\\\\\__/\\\\\\\\\\\____\////\\\\\\\\\________
        _\///_________________\/////////_____\/////////////____\///////////////__\///////////________\/////////________
***********************************************************************************************************************/


Editor
ed_init(Allocator * a)
{
    Editor ed = { 0 };

    ed.allocator = a;

    arr_init(&ed.data, ed.allocator);
    arr_init(&ed.lines, ed.allocator);

    Line line = { 0 };

    arr_push(&ed.lines, line);

    return ed;
}


void ed_deinit(Editor * ed)
{
    arr_deinit(&ed->data);
    arr_deinit(&ed->lines);
}


bool ed_edit_key(Editor * ed, PKey key, KeyMod mod)
{
    if (mod.ctrl && key == PKEY_a) {
        ed_select_all(ed);
        return true;
    }

    if (mod.ctrl && key == PKEY_v) {
        ed_clipboard_paste(ed);
    }

    if (mod.ctrl && key == PKEY_c) {
        ed_clipboard_copy(ed);
    }

    if (mod.ctrl && key == PKEY_x) {
        ed_clipboard_cut(ed);
    }

    if (!mod.ctrl && !mod.alt && isprint(key)) {
        ed_insert_char(ed, p_get_last_key_char());
        return true;
    }

    if (key == PKEY_backSpace) {
        ed_backspace(ed);
        return true;
    }

    if (key == PKEY_tab) {
        ed_insert_tab(ed);
        return true;
    }

    if (key == PKEY_return) {
        ed_insert_newline(ed);
        return true;
    }

    if (key == PKEY_left) {
        if (mod.ctrl) {
            ed_move_word_left(ed, mod.shift);
            return true;
        }
        else {
            ed_move_char_left(ed, mod.shift);
            return true;
        }
    }

    if (key == PKEY_right) {
        if (mod.ctrl) {
            ed_move_word_right(ed, mod.shift);
            return true;
        }
        else {
            ed_move_char_right(ed, mod.shift);
            return true;
        }
    }

    if (key == PKEY_down) {
        ed_move_down(ed, mod.shift);
        return true;
    }

    if (key == PKEY_up) {
        ed_move_up(ed, mod.shift);
        return true;
    }

    return false;
}


void ed_mouse_click(Editor * ed, i32 x, i32 y)
{
    i32 const rel_y = y - (i32)OFFSET_Y;
    i32 const rel_x = x - (i32)OFFSET_X;

    usize const row_offset = (usize)((f32)Max(rel_y, 0) / GLYPH_HEIGHT);
    usize const col_offset = (usize)((f32)Max(rel_x, 0) / GLYPH_WIDTH);

    usize click_row = ed->scroll_offset_y + row_offset;
    click_row       = Min(click_row, ed->lines.len - 1);

    Line const  click_line = ed->lines.ptr[click_row];
    usize const click_col  = ed->scroll_offset_x + col_offset;
    usize const click_pos  = Min(click_line.begin + click_col, click_line.end);

    ed->cursor = click_pos;
    ed->anchor = ed->cursor;
}


void ed_render(Editor * ed, surface_t * sur, i32 x, i32 y, i32 w, i32 h)
{
    usize const scroll_pading = 3;

    Pos const   cursor_pos = ed_find_cursor_pos(ed);
    usize const cursor_row = cursor_pos.x;
    usize const cursor_col = cursor_pos.y;

    usize const visible_lines = (usize)(((f32)h - OFFSET_Y) / GLYPH_HEIGHT) - 1;

    usize const padding = Min(scroll_pading, visible_lines / 2);

    if (cursor_row < ed->scroll_offset_y + padding) {
        ed->scroll_offset_y = (usize)Max((i32)cursor_row - (i32)padding, 0);
    }

    if (cursor_row + padding + 1 > ed->scroll_offset_y + visible_lines) {
        ed->scroll_offset_y = cursor_row + padding + 1 - visible_lines;
    }

    usize const max_scroll = (ed->lines.len > visible_lines) ? ed->lines.len - visible_lines : 0;
    ed->scroll_offset_y    = Min(ed->scroll_offset_y, max_scroll);

    usize const start_line  = ed->scroll_offset_y;
    usize const finish_line = Min(start_line + visible_lines, ed->lines.len);

    usize const scroll_pading_x = 3;

    usize const visible_cols = (usize)(((f32)w - OFFSET_X) / GLYPH_WIDTH) - 1;
    usize const padding_x    = Min(scroll_pading_x, visible_cols / 2);

    if (cursor_col < ed->scroll_offset_x + padding_x) {
        ed->scroll_offset_x = (usize)Max((i32)cursor_col - (i32)padding_x, 0);
    }

    if (cursor_col + padding_x + 1 > ed->scroll_offset_x + visible_cols) {
        ed->scroll_offset_x = cursor_col + padding_x + 1 - visible_cols;
    }

    usize const start_col = ed->scroll_offset_x;

    // draw line
    draw_rect(sur,
        x,
        ((i32)cursor_row - (i32)start_line) * (i32)GLYPH_HEIGHT + (i32)OFFSET_Y + y,
        w, // was sur->width — probably should be the render region's width, not the whole surface
        (i32)GLYPH_HEIGHT,
        LINE_COLOUR);

    // draw cursor
    draw_rect(sur,
        ((i32)cursor_col - (i32)start_col) * (i32)GLYPH_WIDTH + (i32)OFFSET_X - 1 + x,
        ((i32)cursor_row - (i32)start_line) * (i32)GLYPH_HEIGHT + (i32)OFFSET_Y + y,
        1,
        (i32)GLYPH_HEIGHT,
        CURSOR_COLOUR);

    // draw selection
    usize const selection_begin = Min(ed->anchor, ed->cursor);
    usize const selection_end   = Max(ed->anchor, ed->cursor);

    if (ed_is_selection(ed)) {
        for (iterateEx(line, start_line, finish_line)) {
            usize const line_begin = ed->lines.ptr[line].begin;
            usize const line_end   = ed->lines.ptr[line].end;

            for (iterateEx(i, line_begin, line_end)) {
                if (is_between(i, selection_begin, selection_end)) {
                    usize const col = i - line_begin;
                    draw_rect(sur,
                        ((i32)col - (i32)start_col) * (i32)GLYPH_WIDTH + (i32)OFFSET_X + x,
                        (i32)(line - start_line) * (i32)GLYPH_HEIGHT + (i32)OFFSET_Y + y,
                        (i32)GLYPH_WIDTH,
                        (i32)GLYPH_HEIGHT,
                        CURSOR_COLOUR);
                }
            }
        }
    }


    // draw text
    for (
        usize line = start_line, row = 0;
        line < finish_line;
        line += 1, row += 1) //
    {
        usize const line_begin = ed->lines.ptr[line].begin;
        usize const line_end   = ed->lines.ptr[line].end;
        usize const line_len   = line_end - line_begin;
        usize const skip       = Min(start_col, line_len);
        draw_text(sur,
            ed->data.ptr + line_begin + skip,
            (i32)line_len - (i32)skip,
            (i32)OFFSET_X + x,
            (i32)row * (i32)GLYPH_HEIGHT + (i32)OFFSET_Y + y,
            TEXT_COLOUR);
    }
}


bool ed_open_file(Editor * ed, char const * file_path)
{
    bool result = true;

    ed->data.len  = 0;
    ed->lines.len = 0;

    FILE * file_ptr = fopen(file_path, "r");

    if (file_ptr == NULL) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n", file_path,
            strerror(errno));
        result = false;
        goto cleanup;
    }

    fseek(file_ptr, 0L, SEEK_END);
    isize const file_size = ftell(file_ptr);
    fseek(file_ptr, 0L, SEEK_SET);

    if (file_size < 0) {
        fprintf(stderr, "ERROR: ftell failed: %s\n", strerror(errno));
        result = false;
        goto cleanup;
    }

    arr_reserve(&ed->data, file_size);

    isize const n = fread(ed->data.ptr, sizeof(char), file_size, file_ptr);
    if (n != file_size) {
        if (ferror(file_ptr)) {
            fprintf(stderr, "ERROR: could not read file %s: %s\n", file_path,
                strerror(errno));
            result = false;
            goto cleanup;
        }
    }

    ed->data.len = n;

cleanup:
    if (result)
        ed_compute_lines(ed);
    if (file_ptr)
        fclose(file_ptr);
    return result;
}


bool ed_write_file(Editor * ed, char const * file_path)
{
    bool result = true;

    FILE * file_ptr = fopen(file_path, "w");

    if (file_ptr == NULL) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n", file_path,
            strerror(errno));
        result = false;
        goto cleanup;
    }

    fprintf(file_ptr, "%.*s", (int)ed->data.len, ed->data.ptr);

cleanup:
    fclose(file_ptr);
    return result;
}
