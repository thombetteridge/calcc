
#include "base.h"
#include "calcc.h"
#include "editor.h"
#include "platform.h"

#include <stdio.h>

#include "config.h"


int main(/*I32 argc, char ** argv*/ void)
{
    Allocator allocator = default_allocator_init();

    printf("calcc2\n");



    if (!p_init("calcc2")) {
        return 1;
    }

    surface_t sur_out = surface_init(&allocator, 300, 160);
    draw_clear(&sur_out, WHITE);

    surface_t sur_in = surface_init(&allocator, 300, WINDOW_HEIGHT - sur_out.height);
    draw_clear(&sur_in, WHITE);

    Editor ed_out = ed_init(&allocator);
    Editor ed_in  = ed_init(&allocator);


    Calculator calc = calc_init(&allocator);


    // if (argc > 1) {
    //     char const * file_path = argv[1];
    //     // fprintf(stderr, "file:%s\n", file_path);
    //     bool const success = ed_open_file(&ed, file_path);
    //     if (!success)
    //         fprintf(stderr, "ERROR: Could not read file %s: %s\n", file_path, strerror(errno));
    // }

    while (p_running()) {
        p_wait_for_event(1000);

        if (!p_poll()) {
            continue;
        }

        if (p_is_mouse_pressed(PLATFORM_MOUSE_LEFT)) {
            point_t m = p_mouse_pos();

            ed_mouse_click(&ed_in, m.x, m.y);
        }

        PKey key;

        if (p_get_pressed_key(&key)) {
            bool ctrl_down  = p_is_key_down(PKEY_controlL) || p_is_key_down(PKEY_controlR) ? true : false;
            bool shift_down = p_is_key_down(PKEY_shiftL) || p_is_key_down(PKEY_shiftR) ? true : false;
            bool alt_down   = p_is_key_down(PKEY_altL) || p_is_key_down(PKEY_altR) ? true : false;

            KeyMod mod = { .shift = shift_down, .ctrl = ctrl_down, .alt = alt_down };

            ed_edit_key(&ed_in, key, mod);
        }

        // if (p_window_width() != sur.width || p_window_height() != sur.height) {
        //     printf("window.w %d, window.h %d, sur.w %d, sur.h %i\n",
        //         p_window_width(), p_window_height(), sur.width, sur.height);

        //     surface_resize(&sur, p_window_width(), p_window_height());
        // }
        //

        // arr_clear(&ed_out.data);

        // for (iterate(i, ed_in.data.len)) {
        //     arr_push(&ed_out.data, ed_in.data.ptr[i]);
        // }

        if (ed_in.dirty == true) {
            StringV result = calc_eval(&calc, (StringV) { .ptr = ed_in.data.ptr, .len = ed_in.data.len });

            // for (iterate(i, calc.tokens.len)) {
            //     fprintf(stderr, "Kind=%d, Text= %.*s\n", calc.tokens.ptr[i].kind, (I32)calc.tokens.ptr[i].text.len, calc.tokens.ptr[i].text.ptr);
            // }

            ed_clear(&ed_out);

            ed_push_text(&ed_out, result.ptr, result.len);
        }

        if (p_is_window_valid()) {
            p_clear(RGB_COLOUR(42u, 42u, 46u));

            draw_clear(&sur_out, BACKGROUND_COLOUR);
            ed_render(&ed_out, &sur_out, 3, 3, 290, 190);
            draw_rect_lines(&sur_out,
                (I32)OFFSET_X,
                (I32)OFFSET_Y,
                sur_out.width - (2 * (I32)OFFSET_X),
                sur_out.height - (2 * (I32)OFFSET_Y),
                BLACK);

            draw_clear(&sur_in, BACKGROUND_COLOUR);
            ed_render(&ed_in, &sur_in, 3, 3, 290, 190);
            draw_rect_lines(&sur_in,
                (I32)OFFSET_X,
                (I32)OFFSET_Y,
                sur_in.width - (2 * (I32)OFFSET_X),
                sur_in.height - (2 * (I32)OFFSET_Y),
                BLACK);

            push_surface(&sur_out, 0, 0);
            push_surface(&sur_in, 0, sur_out.height);

            p_present();
        }

        p_sleep(16);
    }

    // if (argc > 1) {
    //     char const * file_path = argv[1];
    //     bool const   success   = ed_write_file(&ed, file_path);

    //     if (!success)
    //         fprintf(stderr, "ERROR: Could not write file %s: %s\n", file_path, strerror(errno));
    // }


    calc_deinit(&calc);

    ed_deinit(&ed_in);
    ed_deinit(&ed_out);
    surface_deinit(&sur_in);
    surface_deinit(&sur_out);
    p_deinit();
    default_allocator_deinit(&allocator);
    return 0;
}


// SINGLE TRANSLATION UNIT

#include "base.c"
#include "calcc.c"
#include "editor.c"
#include "platform.c"
#include "arena.c"