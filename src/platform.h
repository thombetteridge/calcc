#pragma once

#include "base.h"

#define PROGGY_RATIO (7.0f / 13.0f)
#define GLYPH_HEIGHT (13.0f)
#define GLYPH_WIDTH  (GLYPH_HEIGHT * PROGGY_RATIO)

typedef struct surface_t surface_t;
struct surface_t {
    u32 * buffer;
    i32   width;
    i32   height;

    Allocator * allocator;
};

typedef struct point_t point_t;
struct point_t {
    i32 x, y;
};

typedef enum {
    PLATFORM_MOUSE_LEFT   = 0,
    PLATFORM_MOUSE_MIDDLE = 1,
    PLATFORM_MOUSE_RIGHT  = 2,
} PMouseButton;


typedef enum {
    PKEY_keyNULL      = 0,
    PKEY_escape       = '\033',
    PKEY_backtick     = '`',
    PKEY_0            = '0',
    PKEY_1            = '1',
    PKEY_2            = '2',
    PKEY_3            = '3',
    PKEY_4            = '4',
    PKEY_5            = '5',
    PKEY_6            = '6',
    PKEY_7            = '7',
    PKEY_8            = '8',
    PKEY_9            = '9',
    PKEY_minus        = '-',
    PKEY_equals       = '=',
    PKEY_backSpace    = '\b',
    PKEY_tab          = '\t',
    PKEY_space        = ' ',
    PKEY_a            = 'a',
    PKEY_b            = 'b',
    PKEY_c            = 'c',
    PKEY_d            = 'd',
    PKEY_e            = 'e',
    PKEY_f            = 'f',
    PKEY_g            = 'g',
    PKEY_h            = 'h',
    PKEY_i            = 'i',
    PKEY_j            = 'j',
    PKEY_k            = 'k',
    PKEY_l            = 'l',
    PKEY_m            = 'm',
    PKEY_n            = 'n',
    PKEY_o            = 'o',
    PKEY_p            = 'p',
    PKEY_q            = 'q',
    PKEY_r            = 'r',
    PKEY_s            = 's',
    PKEY_t            = 't',
    PKEY_u            = 'u',
    PKEY_v            = 'v',
    PKEY_w            = 'w',
    PKEY_x            = 'x',
    PKEY_y            = 'y',
    PKEY_z            = 'z',
    PKEY_period       = '.',
    PKEY_comma        = ',',
    PKEY_slash        = '/',
    PKEY_bracket      = '[',
    PKEY_closeBracket = ']',
    PKEY_semicolon    = ';',
    PKEY_apostrophe   = '\'',
    PKEY_backSlash    = '\\',
    PKEY_return       = '\n',
    PKEY_enter        = PKEY_return,
    PKEY_delete       = '\177', /* 127 */
    PKEY_F1,
    PKEY_F2,
    PKEY_F3,
    PKEY_F4,
    PKEY_F5,
    PKEY_F6,
    PKEY_F7,
    PKEY_F8,
    PKEY_F9,
    PKEY_F10,
    PKEY_F11,
    PKEY_F12,
    PKEY_F13,
    PKEY_F14,
    PKEY_F15,
    PKEY_F16,
    PKEY_F17,
    PKEY_F18,
    PKEY_F19,
    PKEY_F20,
    PKEY_F21,
    PKEY_F22,
    PKEY_F23,
    PKEY_F24,
    PKEY_F25,
    PKEY_capsLock,
    PKEY_shiftL,
    PKEY_controlL,
    PKEY_altL,
    PKEY_superL,
    PKEY_shiftR,
    PKEY_controlR,
    PKEY_altR,
    PKEY_superR,
    PKEY_up,
    PKEY_down,
    PKEY_left,
    PKEY_right,
    PKEY_insert,
    PKEY_menu,
    PKEY_end,
    PKEY_home,
    PKEY_pageUp,
    PKEY_pageDown,
    PKEY_numLock,
    PKEY_kpSlash,
    PKEY_kpMultiply,
    PKEY_kpPlus,
    PKEY_kpMinus,
    PKEY_kpEqual,
    PKEY_kp1,
    PKEY_kp2,
    PKEY_kp3,
    PKEY_kp4,
    PKEY_kp5,
    PKEY_kp6,
    PKEY_kp7,
    PKEY_kp8,
    PKEY_kp9,
    PKEY_kp0,
    PKEY_kpPeriod,
    PKEY_kpReturn,
    PKEY_scrollLock,
    PKEY_printScreen,
    PKEY_pause,
    PKEY_world1,
    PKEY_world2,
    PKEY_keyLast = 256 /* padding for alignment ~(175 by default) */
} PKey;


// Lifecycle
bool p_init(Allocator * allocator, char const * title, i32 width, i32 height);
void p_deinit(void);
void p_request_close(void);
void p_sleep(u64 ms);

// Per-frame
bool p_running(void);          //
bool p_poll(void);             //
void p_present(void);          //
void p_wait_for_event(i32 ms); //

// Queries
i32     p_window_width(void);
i32     p_window_height(void);
bool    p_is_window_valid(void); // false when minimized
point_t p_mouse_pos(void);
point_t p_mouse_delta(void);
bool    p_is_mouse_down(PMouseButton button);
bool    p_is_mouse_pressed(PMouseButton button);
bool    p_is_key_pressed(PKey key);
bool    p_is_key_released(PKey key);
bool    p_is_key_down(PKey key);
bool    p_get_pressed_key(PKey * key);
bool    p_get_released_key(PKey * key);
char    p_get_last_key_char(void);
f32     p_mouse_scroll(void);

// Clipboard

char const * p_read_clipboard(usize * length);
void         p_write_clipboard(char const * text, u32 length);


// Render
typedef uint32_t Colour;

#define RGBA_COLOUR(r, g, b, a) ((Colour)((r) | ((g) << 8u) | ((b) << 16u) | ((a) << 24u)))
#define RGB_COLOUR(r, g, b)     RGBA_COLOUR(r, g, b, 255u)
#define WHITE                   RGB_COLOUR(255u, 255u, 255u)
#define BLACK                   RGB_COLOUR(0u, 0u, 0u)
#define GRAY                    RGB_COLOUR(200u, 200u, 200u)
#define BLUE                    RGB_COLOUR(0u, 0u, 255u)
#define RED                     RGB_COLOUR(255u, 0u, 0u)
#define GREEN                   RGB_COLOUR(0u, 255u, 0u)
#define CYAN                    RGB_COLOUR(0u, 255u, 255u)
#define MAGENTA                 RGB_COLOUR(255u, 0u, 255u)
#define YELLOW                  RGB_COLOUR(255u, 255u, 0u)

inline static Colour
toRGB(u32 r, u32 g, u32 b)
{
    return RGB_COLOUR(r, g, b);
}

inline static Colour
toRGBA(u32 r, u32 g, u32 b, u32 a)
{
    return RGBA_COLOUR(r, g, b, a);
}

surface_t surface_init(Allocator * allocator, i32 width, i32 height);
void      surface_deinit(surface_t * sur);
void      surface_resize(surface_t * sur, i32 width, i32 height);
void      push_surface(surface_t const * sur, i32 x, i32 y);
void      p_clear(Colour colour);

void put_pixel(surface_t * sur, i32 x, i32 y, Colour color);
void draw_clear(surface_t * sur, Colour color);
void draw_text(surface_t * sur, char const * str, i32 len, i32 x, i32 y, Colour colour);
void draw_rect(surface_t * sur, i32 x, i32 y, i32 w, i32 h, Colour colour);
void draw_rect_lines(surface_t * sur, i32 x, i32 y, i32 w, i32 h, Colour colour);
void draw_line(surface_t * sur, i32 start_x, i32 start_y, i32 end_x, i32 end_y, Colour colour);
void draw_circle(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, Colour colour);
void draw_circle_lines(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, Colour colour);
void draw_arc_lines(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, f32 start_angle, f32 end_angle, Colour colour);