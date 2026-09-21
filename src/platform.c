#include "platform.h"

#include <RGFW.h>
#include <rtime.h>
#include <stb_truetype.h>

#include "base.h"
#include "proggy_font.h"

#include "config.h"


#define PI_f 3.14159265358979323846f

typedef struct Context Context;
struct Context {
    RGFW_window *  window;
    RGFW_surface * surface;
    surface_t      target;
    point_t        prev_mouse;
    point_t        current_mouse;
    I32            window_width;
    I32            window_height;
    F32            scroll;
    PKey           key_pressed;
    PKey           key_released;
    char           key_char;
    bool           window_valid;
    bool           running;
};

static U32 surface_buffer[WINDOW_WIDTH * WINDOW_HEIGHT];

static surface_t g_surface = {
    .buffer = surface_buffer,
    .height = WINDOW_HEIGHT,
    .width  = WINDOW_WIDTH,
};

static struct Context g_ctx = {
    .window_width  = WINDOW_WIDTH,
    .window_height = WINDOW_HEIGHT,
};


typedef struct {
    U8 *            atlas;
    I32             atlas_w;
    I32             atlas_h;
    stbtt_bakedchar cdata[96];
} font_t;

#define ATLAS_SZ 512

static U8 atlas_buffer[ATLAS_SZ * ATLAS_SZ];

static font_t g_font = {
    .atlas   = atlas_buffer,
    .atlas_w = ATLAS_SZ,
    .atlas_h = ATLAS_SZ,
};


static bool font_load(U8 const * data, u32 data_size, F32 pixel_height);

bool p_init(char const * title) {
    g_ctx.target = g_surface;

    g_ctx.window = RGFW_createWindow(title, 100, 100, g_ctx.window_width, g_ctx.window_height,
        RGFW_windowNoResize | RGFW_windowCenter);

    if (!g_ctx.window)
        return false;

    RGFW_window_setExitKey(g_ctx.window, RGFW_escape);

    g_ctx.current_mouse = p_mouse_pos();
    g_ctx.prev_mouse    = g_ctx.current_mouse;

    g_ctx.surface = RGFW_createSurface((U8 *)g_ctx.target.buffer, g_ctx.target.width, g_ctx.target.height, RGFW_formatRGBA8);

    if (!g_ctx.surface) {
        RGFW_window_close(g_ctx.window);
        g_ctx.window = NULL;
        return false;
    }

    if (!font_load(proggy_clean_ttf_compressed_data, proggy_clean_ttf_compressed_size, GLYPH_HEIGHT))
        return false;

    g_ctx.window_valid = true;
    g_ctx.running      = true;
    return true;
}

void p_deinit(void) {
    if (g_ctx.surface) {
        RGFW_surface_free(g_ctx.surface);
        g_ctx.surface = NULL;
    }
    if (g_ctx.window) {
        RGFW_window_close(g_ctx.window);
        g_ctx.window = NULL;
    }

    g_ctx.running = false;
}

void p_request_close(void) {
    g_ctx.running = false;
}

bool p_running(void) {
    if (!g_ctx.window)
        return false;
    if (RGFW_window_shouldClose(g_ctx.window))
        return false;
    return g_ctx.running;
}

bool p_get_pressed_key(PKey * key) {
    if (g_ctx.key_pressed) {
        *key = g_ctx.key_pressed;
        return true;
    }
    return false;
}

bool p_get_released_key(PKey * key) {
    if (g_ctx.key_released) {
        *key = g_ctx.key_released;
        return true;
    }
    return false;
}

char p_get_last_key_char(void) {
    return g_ctx.key_char;
}

void p_wait_for_event(I32 ms) {
    RGFW_waitForEvent(ms);
}

bool p_poll(void) {
    bool       any = false;
    RGFW_event event;

    g_ctx.prev_mouse   = g_ctx.current_mouse;
    g_ctx.key_pressed  = 0;
    g_ctx.key_released = 0;

    while (RGFW_window_checkEvent(g_ctx.window, &event)) {
        any = true;

        switch (event.type) {
        case RGFW_quit:
            g_ctx.running = false;
            break;

        case RGFW_windowResized: {
            /*
            I32 w, h;
            RGFW_window_getSize(g_ctx.window, &w, &h);
            g_ctx.window_width  = w;
            g_ctx.window_height = h;
            g_ctx.window_valid  = (w > 0 && h > 0);

            if (g_ctx.window_valid) {
                surface_resize(&g_ctx.target, w, h);
                RGFW_surface_free(g_ctx.surface);
                g_ctx.surface = RGFW_createSurface(
                    (U8 *)g_ctx.target.buffer,
                    g_ctx.target.width,
                    g_ctx.target.height,
                    RGFW_formatRGBA8);
            }
            */
            break;
        }
        case RGFW_mouseScroll:
            g_ctx.scroll += event.scroll.y;
            break;


        case RGFW_mousePosChanged:
            g_ctx.current_mouse = (point_t) {
                .x = event.mouse.x,
                .y = event.mouse.y,
            };
            break;

        case RGFW_keyPressed:
            g_ctx.key_pressed = (PKey)event.key.value;
            g_ctx.key_char    = event.key.sym;
            break;
        case RGFW_keyReleased:
            g_ctx.key_released = (PKey)event.key.value;
            break;

        case RGFW_mouseButtonPressed:
        case RGFW_mouseButtonReleased: {
            break;
        }
        }
    }

    return any;
}

void p_present(void) {
    if (g_ctx.window && g_ctx.surface && g_ctx.window_valid) {
        RGFW_window_blitSurface(g_ctx.window, g_ctx.surface);
    }
}

void p_sleep(u64 ms) {
    rt_sleep(ms);
}


I32 p_window_width(void) {
    return g_ctx.window_width;
}

I32 p_window_height(void) {
    return g_ctx.window_height;
}

bool p_is_window_valid(void) {
    return g_ctx.window_valid;
}

point_t p_mouse_pos(void) {
    I32 x = 0, y = 0;
    if (g_ctx.window)
        RGFW_window_getMouse(g_ctx.window, &x, &y);
    return (point_t) { (I32)x, (I32)y };
}

point_t p_mouse_delta(void) {
    point_t delta = {
        g_ctx.current_mouse.x - g_ctx.prev_mouse.x,
        g_ctx.current_mouse.y - g_ctx.prev_mouse.y,
    };
    return delta;
}

bool p_is_mouse_pressed(PMouseButton button) {
    if (!g_ctx.window)
        return false;
    RGFW_mouseButton b;
    switch (button) {
    case PLATFORM_MOUSE_LEFT:
        b = RGFW_mouseLeft;
        break;
    case PLATFORM_MOUSE_MIDDLE:
        b = RGFW_mouseMiddle;
        break;
    case PLATFORM_MOUSE_RIGHT:
        b = RGFW_mouseRight;
        break;
    default:
        return false;
    }
    return RGFW_isMousePressed(b);
}

F32 p_mouse_scroll(void) {
    F32 s        = g_ctx.scroll;
    g_ctx.scroll = 0.0f;
    return s;
}


bool p_is_key_pressed(PKey key) {
    return RGFW_isKeyPressed((U8)key);
}


bool p_is_key_released(PKey key) {
    return RGFW_isKeyReleased((U8)key);
}


bool p_is_key_down(PKey key) {
    return RGFW_isKeyDown((U8)key);
}

char const * p_read_clipboard(Sz * length) {
    return RGFW_readClipboard($ptrCast(USz, length));
}

void p_write_clipboard(char const * text, u32 length) {
    RGFW_writeClipboard(text, length);
}

// drawing

surface_t
surface_init(Allocator * allocator, I32 width, I32 height) {
    surface_t  s;
    uint32_t * buffer = ALLOC(allocator, u32, width * height);
    // memset(buffer, 0, (size_t)(width * height * (I32)sizeof(uint32_t)));
    s.buffer    = buffer;
    s.width     = width;
    s.height    = height;
    s.allocator = allocator;
    return s;
}

void surface_deinit(surface_t * sur) {
    DEALLOC(sur->allocator, sur->buffer, sur->width * sur->height);
    memset(sur, 0, sizeof(*sur));
}

void surface_resize(surface_t * sur, I32 width, I32 height) {
    surface_t new_surface = surface_init(sur->allocator, width, height);
    surface_deinit(sur);

    // sur->buffer    = new_surface.buffer;
    // sur->width     = new_surface.width;
    // sur->height    = new_surface.height;
    // sur->allocator = new_surface.allocator;

    *sur = new_surface;
}


void push_surface(surface_t const * sur, I32 dst_x, I32 dst_y) {
    for (I32 src_y = 0; src_y < sur->height; src_y += 1) {
        for (I32 src_x = 0; src_x < sur->width; src_x += 1) {
            Colour const color = sur->buffer[src_y * sur->width + src_x];

            draw_pixel(&g_ctx.target, dst_x + src_x, dst_y + src_y, color);
        }
    }
}


void p_clear(Colour colour) {
    draw_clear(&g_ctx.target, colour);
}

void draw_pixel(surface_t * sur, I32 x, I32 y, Colour color) {
    // TODO we should log this
    if (x >= sur->width || y >= sur->height || x < 0 || y < 0)
        return;
    sur->buffer[y * sur->width + x] = color;
}


void draw_clear(surface_t * sur, Colour color) {
    for (I32 i = 0; i < sur->width * sur->height; i += 1)
        sur->buffer[i] = color;
}


void draw_rect(surface_t * sur, I32 x, I32 y, I32 w, I32 h, Colour colour) {
    for (I32 i = 0; i < w; i += 1)
        for (I32 j = 0; j < h; j += 1)
            draw_pixel(sur, i + x, j + y, colour);
}

void draw_rect_lines(surface_t * sur, I32 x, I32 y, I32 w, I32 h, Colour colour) {
    I32 p0x = x;
    I32 p0y = y;

    I32 p1x = x + w;
    I32 p1y = y;

    I32 p2x = x + w;
    I32 p2y = y + h;

    I32 p3x = x;
    I32 p3y = y + h;

    draw_line(sur, p0x, p0y, p1x, p1y, colour);
    draw_line(sur, p1x, p1y, p2x, p2y, colour);
    draw_line(sur, p2x, p2y, p3x, p3y, colour);
    draw_line(sur, p3x, p3y, p0x, p0y, colour);
}


static void
swapi(I32 * a, I32 * b) {
    I32 t = *a;
    *a    = *b;
    *b    = t;
}

void draw_line(surface_t * sur, I32 start_x, I32 start_y, I32 end_x, I32 end_y, Colour colour) {
    if (start_x < end_x) {
        swapi(&start_x, &end_x);
        swapi(&start_y, &end_y);
    }

    F32 x = (F32)start_x;
    F32 y = (F32)start_y;

    F32 dx = (F32)(end_x - start_x);
    F32 dy = (F32)(end_y - start_y);

    F32 steps = fabsf(dx) >= fabsf(dy) ? fabsf(dx) : fabsf(dy);

    dx /= steps;
    dy /= steps;

    for (I32 i = 0; i <= (I32)steps; i += 1) {
        draw_pixel(sur, (I32)roundf(x), (I32)roundf(y), colour);
        x += dx;
        y += dy;
    }
}


static void
blend_pixel(surface_t * sur, I32 x, I32 y, Colour colour, U8 coverage) {
    if (coverage == 0 || x < 0 || y < 0 || x >= sur->width || y >= sur->height)
        return;
    if (coverage == 255) {
        draw_pixel(sur, x, y, colour);
        return;
    }

    Colour bg = sur->buffer[y * sur->width + x];

    U8 const * src = (U8 const *)&colour;
    U8 const * dst = (U8 const *)&bg;
    U8         out[4];
    for (I32 i = 0; i < 4; ++i)
        out[i] = (U8)((src[i] * coverage + dst[i] * (255 - coverage)) / 255);

    draw_pixel(sur, x, y, *(Colour *)out);
}

static unsigned int stb_decompress_length(unsigned char const * input);
static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length);


static bool font_load(U8 const * data, u32 data_size, F32 pixel_height) {
    U8 * ttf_buffer = malloc(stb_decompress_length(data) + 1); // extra byte for sentinel valuie

    // u32 const decompressed_size = stb_decompress_length(data);
    // U8 *      ttf_buffer        = ALLOC(allocator, U8, decompressed_size);

    if (!stb_decompress(ttf_buffer, data, data_size)) {
        free(ttf_buffer);

        return false;
    }

    I32 ok = stbtt_BakeFontBitmap(ttf_buffer, 0, pixel_height,
        g_font.atlas, g_font.atlas_w, g_font.atlas_h,
        32, 96, g_font.cdata);

    free(ttf_buffer);

    // DEALLOC(allocator, ttf_buffer, decompressed_size);
    return ok > 0;
}

static void font_unload(Allocator * allocator) {
    // if (g_font.atlas) {
    //     DEALLOC(allocator, g_font.atlas, g_font.atlas_w * g_font.atlas_h);
    // }
}


void draw_text(surface_t * sur, char const * str, I32 len, I32 x, I32 y, Colour colour) {
    F32 fx = (F32)x;
    F32 fy = (F32)y + 9.5f; /* works for proggy at size 13 */

    for (I32 i = 0; i < len; ++i) {
        char c = str[i];
        if (c < 32 /*|| c >= 128*/)
            continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(g_font.cdata, g_font.atlas_w, g_font.atlas_h,
            c - 32, &fx, &fy, &q, 1);

        I32 x0 = (I32)q.x0, x1 = (I32)q.x1;
        I32 y0 = (I32)q.y0, y1 = (I32)q.y1;
        F32 u_step = (q.s1 - q.s0) / (F32)(x1 - x0);
        F32 v_step = (q.t1 - q.t0) / (F32)(y1 - y0);

        for (I32 py = y0; py < y1; ++py) {
            for (I32 px = x0; px < x1; ++px) {
                F32 u = q.s0 + (F32)(px - x0) * u_step;
                F32 v = q.t0 + (F32)(py - y0) * v_step;

                U8 coverage = g_font.atlas[(I32)(v * (F32)g_font.atlas_h) * g_font.atlas_w + (I32)(u * (F32)g_font.atlas_w)];
                blend_pixel(sur, px, py, colour, coverage);
            }
        }
    }
}


void draw_circle(surface_t * sur, I32 centre_x, I32 centre_y, I32 radius, Colour colour) {
    I32 x0 = centre_x - radius;
    I32 x1 = centre_x + radius;

    I32 y0 = centre_y - radius;
    I32 y1 = centre_y + radius;

    for (I32 y = y0; y < y1; y += 1) {
        for (I32 x = x0; x < x1; x += 1) {
            I32 delta_x = x - centre_x;
            I32 delta_y = y - centre_y;

            if ((delta_x * delta_x) + (delta_y * delta_y) <= (radius * radius)) {
                draw_pixel(sur, x, y, colour);
            }
        }
    }
}


void draw_circle_lines(surface_t * sur, I32 centre_x, I32 centre_y, I32 radius, Colour colour) {
    // Source:
    // https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/

    I32 x = 0;
    I32 y = radius;
    I32 d = 3 - 2 * radius;


    draw_pixel(sur, centre_x + x, centre_y + y, colour);
    draw_pixel(sur, centre_x - x, centre_y + y, colour);
    draw_pixel(sur, centre_x + x, centre_y - y, colour);
    draw_pixel(sur, centre_x - x, centre_y - y, colour);
    draw_pixel(sur, centre_x + y, centre_y + x, colour);
    draw_pixel(sur, centre_x - y, centre_y + x, colour);
    draw_pixel(sur, centre_x + y, centre_y - x, colour);
    draw_pixel(sur, centre_x - y, centre_y - x, colour);

    while (y >= x) {
        x += 1;

        if (d > 0) {
            y -= 1;
            d += 4 * (x - y) + 10;
        } else {
            d += 4 * x + 6;
        }

        draw_pixel(sur, centre_x + x, centre_y + y, colour);
        draw_pixel(sur, centre_x - x, centre_y + y, colour);
        draw_pixel(sur, centre_x + x, centre_y - y, colour);
        draw_pixel(sur, centre_x - x, centre_y - y, colour);
        draw_pixel(sur, centre_x + y, centre_y + x, colour);
        draw_pixel(sur, centre_x - y, centre_y + x, colour);
        draw_pixel(sur, centre_x + y, centre_y - x, colour);
        draw_pixel(sur, centre_x - y, centre_y - x, colour);
    }
}


static void
arc_pixel(surface_t * sur, I32 px, I32 py, I32 centre_x, I32 centre_y, F32 start_angle, F32 end_angle, Colour colour) {

    F32 a = atan2f((F32)(py - centre_y), (F32)(px - centre_x));
    if (a < 0)
        a += 2.0f * PI_f;
    I32 in_arc = (start_angle <= end_angle)
                     ? (a >= start_angle && a <= end_angle)
                     : (a >= start_angle || a <= end_angle);
    if (in_arc)
        draw_pixel(sur, px, py, colour);
}

void draw_arc_lines(surface_t * sur, I32 centre_x, I32 centre_y, I32 radius, F32 start_angle, F32 end_angle, Colour colour) {
    // Normalise so start < end, both in [0, 2pi)
    while (start_angle < 0)
        start_angle += 2.0f * PI_f;
    while (end_angle < 0)
        end_angle += 2.0f * PI_f;
    while (start_angle >= 2 * PI_f)
        start_angle -= 2.0f * PI_f;
    while (end_angle >= 2 * PI_f)
        end_angle -= 2.0f * PI_f;

    I32 x = 0;
    I32 y = radius;
    I32 d = 3 - 2 * radius;

    arc_pixel(sur, centre_x + x, centre_y + y, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x - x, centre_y + y, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x + x, centre_y - y, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x - x, centre_y - y, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x + y, centre_y + x, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x - y, centre_y + x, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x + y, centre_y - x, centre_x, centre_y, start_angle, end_angle, colour);
    arc_pixel(sur, centre_x - y, centre_y - x, centre_x, centre_y, start_angle, end_angle, colour);

    while (y >= x) {
        x += 1;
        if (d > 0) {
            y -= 1;
            d += 4 * (x - y) + 10;
        } else {
            d += 4 * x + 6;
        }
        arc_pixel(sur, centre_x + x, centre_y + y, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x - x, centre_y + y, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x + x, centre_y - y, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x - x, centre_y - y, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x + y, centre_y + x, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x - y, centre_y + x, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x + y, centre_y - x, centre_x, centre_y, start_angle, end_angle, colour);
        arc_pixel(sur, centre_x - y, centre_y - x, centre_x, centre_y, start_angle, end_angle, colour);
    }
}


#ifdef _WIN32
#include <stdio.h>
#include <windows.h>

void init_console() {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
    }
}

void shutdown_console() {
    fflush(stdout);
    fflush(stderr);

    fclose(stdout);
    fclose(stderr);
    fclose(stdin);

    FreeConsole();
}

#else
void init_console() { }

void shutdown_console() { }

#endif

// --- stolen stb_decompress (public domain, Sean Barrett, via github.com/nothings/stb) ---

static unsigned int stb_decompress_length(unsigned char const * input) {
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *       stb__barrier_out_e, *stb__barrier_out_b;
static unsigned char const * stb__barrier_in_b;
static unsigned char *       stb__dout;

static void stb__match(unsigned char const * data, unsigned int length) {
    if (stb__dout + length > stb__barrier_out_e) {
        stb__dout += length;
        return;
    }
    if (data < stb__barrier_out_b) {
        stb__dout = stb__barrier_out_e + 1;
        return;
    }
    while (length--)
        *stb__dout++ = *data++;
}

static void stb__lit(unsigned char const * data, unsigned int length) {
    if (stb__dout + length > stb__barrier_out_e) {
        stb__dout += length;
        return;
    }
    if (data < stb__barrier_in_b) {
        stb__dout = stb__barrier_out_e + 1;
        return;
    }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x) ((i[x] << 8) + i[(x) + 1])
#define stb__in3(x) ((i[x] << 16) + stb__in2((x) + 1))
#define stb__in4(x) ((i[x] << 24) + stb__in3((x) + 1))

static unsigned char const * stb_decompress_token(unsigned char const * i) {
    if (*i >= 0x20) {
        if (*i >= 0x80)
            stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)
            stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
        else
            stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else {
        if (*i >= 0x18)
            stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
        else if (*i >= 0x10)
            stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1), i += 5;
        else if (*i >= 0x08)
            stb__lit(i + 2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)
            stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)
            stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
        else if (*i == 0x04)
            stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char * buffer, unsigned int buflen) {
    unsigned long const ADLER_MOD = 65521;
    unsigned long       s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long       blocklen = buflen % 5552;
    unsigned long       i;

    while (buflen) {
        for (i = 0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;
        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= (unsigned int)blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length) {
    (void)length;
    if (stb__in4(0) != 0x57bC0000)
        return 0;
    if (stb__in4(4) != 0)
        return 0;

    unsigned int const olen = stb_decompress_length(i);
    stb__barrier_in_b       = i;
    stb__barrier_out_e      = output + olen;
    stb__barrier_out_b      = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        unsigned char const * old_i = i;
        i                           = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                if (stb__dout != output + olen)
                    return 0;
                if (stb_adler32(1, output, olen) != (unsigned int)stb__in4(2))
                    return 0;
                return olen;
            }
            return 0;
        }
        if (stb__dout > output + olen)
            return 0;
    }
}
