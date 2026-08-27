#include "platform.h"

#include <RGFW.h>
#include <rtime.h>
#include <stb_truetype.h>

#include "base.h"
#include "proggy_font.h"

#define PI_f 3.14159265358979323846f

static struct {
    RGFW_window *  window;
    RGFW_surface * surface;
    surface_t      target;
    point_t        prev_mouse;
    point_t        current_mouse;
    i32            window_width;
    i32            window_height;
    f32            scroll;
    PKey           key_pressed;
    PKey           key_released;
    char           key_char;
    bool           window_valid;
    bool           running;

    Allocator * allocator;
} g_ctx;

typedef struct {
    u8 *            atlas;
    i32             atlas_w;
    i32             atlas_h;
    stbtt_bakedchar cdata[96];
} font_t;

static font_t g_font;


static bool font_load(Allocator * allocator, u8 const * data, u32 data_size, f32 pixel_height);
static void font_unload(Allocator * allocator);

bool p_init(Allocator * allocator, char const * title, i32 width, i32 height)
{

    g_ctx.allocator     = allocator;
    g_ctx.window_width  = width;
    g_ctx.window_height = height;

    g_ctx.target = surface_init(g_ctx.allocator, g_ctx.window_width, g_ctx.window_height);

    g_ctx.window = RGFW_createWindow(title, 100, 100, g_ctx.window_width, g_ctx.window_height, 0u);

    if (!g_ctx.window)
        return false;

    RGFW_window_setExitKey(g_ctx.window, RGFW_escape);

    g_ctx.current_mouse = p_mouse_pos();
    g_ctx.prev_mouse    = g_ctx.current_mouse;

    g_ctx.surface = RGFW_createSurface((u8 *)g_ctx.target.buffer, g_ctx.target.width, g_ctx.target.height, RGFW_formatRGBA8);

    if (!g_ctx.surface) {
        RGFW_window_close(g_ctx.window);
        g_ctx.window = NULL;
        return false;
    }

    if (!font_load(g_ctx.allocator, proggy_clean_ttf_compressed_data, proggy_clean_ttf_compressed_size, GLYPH_HEIGHT))
        return false;

    g_ctx.window_valid = true;
    g_ctx.running      = true;
    return true;
}

void p_deinit(void)
{
    if (g_ctx.surface) {
        RGFW_surface_free(g_ctx.surface);
        g_ctx.surface = NULL;
    }
    if (g_ctx.window) {
        RGFW_window_close(g_ctx.window);
        g_ctx.window = NULL;
    }
    if (g_ctx.target.buffer) {
        surface_deinit(&g_ctx.target);
    }

    font_unload(g_ctx.allocator);

    g_ctx.running = false;
}

void p_request_close(void)
{
    g_ctx.running = false;
}

bool p_running(void)
{
    if (!g_ctx.window)
        return false;
    if (RGFW_window_shouldClose(g_ctx.window))
        return false;
    return g_ctx.running;
}

bool p_get_pressed_key(PKey * key)
{
    if (g_ctx.key_pressed) {
        *key = g_ctx.key_pressed;
        return true;
    }
    return false;
}

bool p_get_released_key(PKey * key)
{
    if (g_ctx.key_released) {
        *key = g_ctx.key_released;
        return true;
    }
    return false;
}

char p_get_last_key_char(void)
{
    return g_ctx.key_char;
}

void p_wait_for_event(i32 ms)
{
    RGFW_waitForEvent(ms);
}

bool p_poll(void)
{
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
            i32 w, h;
            RGFW_window_getSize(g_ctx.window, &w, &h);
            g_ctx.window_width  = w;
            g_ctx.window_height = h;
            g_ctx.window_valid  = (w > 0 && h > 0);

            if (g_ctx.window_valid) {
                surface_resize(&g_ctx.target, w, h);
                RGFW_surface_free(g_ctx.surface);
                g_ctx.surface = RGFW_createSurface(
                    (u8 *)g_ctx.target.buffer,
                    g_ctx.target.width,
                    g_ctx.target.height,
                    RGFW_formatRGBA8);
            }
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

void p_present(void)
{
    if (g_ctx.window && g_ctx.surface && g_ctx.window_valid) {
        RGFW_window_blitSurface(g_ctx.window, g_ctx.surface);
    }
}

void p_sleep(u64 ms)
{
    rt_sleep(ms);
}


i32 p_window_width(void)
{
    return g_ctx.window_width;
}

i32 p_window_height(void)
{
    return g_ctx.window_height;
}

bool p_is_window_valid(void)
{
    return g_ctx.window_valid;
}

point_t p_mouse_pos(void)
{
    i32 x = 0, y = 0;
    if (g_ctx.window)
        RGFW_window_getMouse(g_ctx.window, &x, &y);
    return (point_t) { (i32)x, (i32)y };
}

point_t p_mouse_delta(void)
{
    point_t delta = {
        g_ctx.current_mouse.x - g_ctx.prev_mouse.x,
        g_ctx.current_mouse.y - g_ctx.prev_mouse.y,
    };
    return delta;
}

bool p_is_mouse_pressed(PMouseButton button)
{
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

f32 p_mouse_scroll(void)
{
    f32 s        = g_ctx.scroll;
    g_ctx.scroll = 0.0f;
    return s;
}


bool p_is_key_pressed(PKey key)
{
    return RGFW_isKeyPressed((u8)key);
}


bool p_is_key_released(PKey key)
{
    return RGFW_isKeyReleased((u8)key);
}


bool p_is_key_down(PKey key)
{
    return RGFW_isKeyDown((u8)key);
}

char const * p_read_clipboard(usize * length)
{
    return RGFW_readClipboard(length);
}

void p_write_clipboard(char const * text, u32 length)
{
    RGFW_writeClipboard(text, length);
}

// drawing

surface_t
surface_init(Allocator * allocator, i32 width, i32 height)
{
    surface_t  s;
    uint32_t * buffer = ALLOC(allocator, u32, width * height);
    // memset(buffer, 0, (size_t)(width * height * (i32)sizeof(uint32_t)));
    s.buffer    = buffer;
    s.width     = width;
    s.height    = height;
    s.allocator = allocator;
    return s;
}

void surface_deinit(surface_t * sur)
{
    DEALLOC(sur->allocator, sur->buffer, sur->width * sur->height);
    memset(sur, 0, sizeof(*sur));
}

void surface_resize(surface_t * sur, i32 width, i32 height)
{
    surface_t new_surface = surface_init(sur->allocator, width, height);
    surface_deinit(sur);

    // sur->buffer    = new_surface.buffer;
    // sur->width     = new_surface.width;
    // sur->height    = new_surface.height;
    // sur->allocator = new_surface.allocator;

    *sur = new_surface;
}


void push_surface(surface_t const * sur, i32 dst_x, i32 dst_y)
{
    for (i32 src_y = 0; src_y < sur->height; src_y += 1) {
        for (i32 src_x = 0; src_x < sur->width; src_x += 1) {
            Colour const color = sur->buffer[src_y * sur->width + src_x];

            put_pixel(&g_ctx.target, dst_x + src_x, dst_y + src_y, color);
        }
    }
}


void p_clear(Colour colour)
{
    draw_clear(&g_ctx.target, colour);
}

void put_pixel(surface_t * sur, i32 x, i32 y, Colour color)
{
    // TODO we should log this
    if (x >= sur->width || y >= sur->height || x < 0 || y < 0)
        return;
    sur->buffer[y * sur->width + x] = color;
}


void draw_clear(surface_t * sur, Colour color)
{
    for (i32 i = 0; i < sur->width * sur->height; i += 1)
        sur->buffer[i] = color;
}


void draw_rect(surface_t * sur, i32 x, i32 y, i32 w, i32 h, Colour colour)
{
    for (i32 i = 0; i < w; i += 1)
        for (i32 j = 0; j < h; j += 1)
            put_pixel(sur, i + x, j + y, colour);
}

void draw_rect_lines(surface_t * sur, i32 x, i32 y, i32 w, i32 h, Colour colour)
{
    i32 p0x = x;
    i32 p0y = y;

    i32 p1x = x + w;
    i32 p1y = y;

    i32 p2x = x + w;
    i32 p2y = y + h;

    i32 p3x = x;
    i32 p3y = y + h;

    draw_line(sur, p0x, p0y, p1x, p1y, colour);
    draw_line(sur, p1x, p1y, p2x, p2y, colour);
    draw_line(sur, p2x, p2y, p3x, p3y, colour);
    draw_line(sur, p3x, p3y, p0x, p0y, colour);
}


static void
swapi(i32 * a, i32 * b)
{
    i32 t = *a;
    *a    = *b;
    *b    = t;
}

void draw_line(surface_t * sur, i32 start_x, i32 start_y, i32 end_x, i32 end_y, Colour colour)
{
    if (start_x < end_x) {
        swapi(&start_x, &end_x);
        swapi(&start_y, &end_y);
    }

    f32 x = (f32)start_x;
    f32 y = (f32)start_y;

    f32 dx = (f32)(end_x - start_x);
    f32 dy = (f32)(end_y - start_y);

    f32 steps = fabsf(dx) >= fabsf(dy) ? fabsf(dx) : fabsf(dy);

    dx /= steps;
    dy /= steps;

    for (i32 i = 0; i <= (i32)steps; i += 1) {
        put_pixel(sur, (i32)roundf(x), (i32)roundf(y), colour);
        x += dx;
        y += dy;
    }
}


static void
blend_pixel(surface_t * sur, i32 x, i32 y, Colour colour, u8 coverage)
{
    if (coverage == 0 || x < 0 || y < 0 || x >= sur->width || y >= sur->height)
        return;
    if (coverage == 255) {
        put_pixel(sur, x, y, colour);
        return;
    }

    Colour bg = sur->buffer[y * sur->width + x];

    u8 const * src = (u8 const *)&colour;
    u8 const * dst = (u8 const *)&bg;
    u8         out[4];
    for (i32 i = 0; i < 4; ++i)
        out[i] = (u8)((src[i] * coverage + dst[i] * (255 - coverage)) / 255);

    put_pixel(sur, x, y, *(Colour *)out);
}

static unsigned int stb_decompress_length(unsigned char const * input);
static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length);


static bool font_load(Allocator * allocator, u8 const * data, u32 data_size, f32 pixel_height)
{
    u32 const decompressed_size = stb_decompress_length(data);
    u8 *      ttf_buffer        = ALLOC(allocator, u8, decompressed_size);

    if (!stb_decompress(ttf_buffer, data, data_size)) {
        DEALLOC(allocator, ttf_buffer, decompressed_size);
        return false;
    }

    g_font.atlas_w = 512;
    g_font.atlas_h = 512;
    g_font.atlas   = ALLOC(allocator, u8, g_font.atlas_w * g_font.atlas_h);

    i32 ok = stbtt_BakeFontBitmap(ttf_buffer, 0, pixel_height,
        g_font.atlas, g_font.atlas_w, g_font.atlas_h,
        32, 96, g_font.cdata);

    DEALLOC(allocator, ttf_buffer, decompressed_size);
    return ok > 0;
}

static void font_unload(Allocator * allocator)
{
    if (g_font.atlas) {
        DEALLOC(allocator, g_font.atlas, g_font.atlas_w * g_font.atlas_h);
    }
}


void draw_text(surface_t * sur, char const * str, i32 len, i32 x, i32 y, Colour colour)
{
    f32 fx = (f32)x;
    f32 fy = (f32)y + 9.5f; /* works for proggy at size 13 */

    for (i32 i = 0; i < len; ++i) {
        char c = str[i];
        if (c < 32 /*|| c >= 128*/)
            continue;

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(g_font.cdata, g_font.atlas_w, g_font.atlas_h,
            c - 32, &fx, &fy, &q, 1);

        i32 x0 = (i32)q.x0, x1 = (i32)q.x1;
        i32 y0 = (i32)q.y0, y1 = (i32)q.y1;
        f32 u_step = (q.s1 - q.s0) / (f32)(x1 - x0);
        f32 v_step = (q.t1 - q.t0) / (f32)(y1 - y0);

        for (i32 py = y0; py < y1; ++py) {
            for (i32 px = x0; px < x1; ++px) {
                f32 u = q.s0 + (f32)(px - x0) * u_step;
                f32 v = q.t0 + (f32)(py - y0) * v_step;

                u8 coverage = g_font.atlas[(i32)(v * (f32)g_font.atlas_h) * g_font.atlas_w + (i32)(u * (f32)g_font.atlas_w)];
                blend_pixel(sur, px, py, colour, coverage);
            }
        }
    }
}


void draw_circle(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, Colour colour)
{
    i32 x0 = centre_x - radius;
    i32 x1 = centre_x + radius;

    i32 y0 = centre_y - radius;
    i32 y1 = centre_y + radius;

    for (i32 y = y0; y < y1; y += 1) {
        for (i32 x = x0; x < x1; x += 1) {
            i32 delta_x = x - centre_x;
            i32 delta_y = y - centre_y;

            if ((delta_x * delta_x) + (delta_y * delta_y) <= (radius * radius)) {
                put_pixel(sur, x, y, colour);
            }
        }
    }
}


void draw_circle_lines(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, Colour colour)
{
    // Source:
    // https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/

    i32 x = 0;
    i32 y = radius;
    i32 d = 3 - 2 * radius;


    put_pixel(sur, centre_x + x, centre_y + y, colour);
    put_pixel(sur, centre_x - x, centre_y + y, colour);
    put_pixel(sur, centre_x + x, centre_y - y, colour);
    put_pixel(sur, centre_x - x, centre_y - y, colour);
    put_pixel(sur, centre_x + y, centre_y + x, colour);
    put_pixel(sur, centre_x - y, centre_y + x, colour);
    put_pixel(sur, centre_x + y, centre_y - x, colour);
    put_pixel(sur, centre_x - y, centre_y - x, colour);

    while (y >= x) {
        x += 1;

        if (d > 0) {
            y -= 1;
            d += 4 * (x - y) + 10;
        }
        else {
            d += 4 * x + 6;
        }

        put_pixel(sur, centre_x + x, centre_y + y, colour);
        put_pixel(sur, centre_x - x, centre_y + y, colour);
        put_pixel(sur, centre_x + x, centre_y - y, colour);
        put_pixel(sur, centre_x - x, centre_y - y, colour);
        put_pixel(sur, centre_x + y, centre_y + x, colour);
        put_pixel(sur, centre_x - y, centre_y + x, colour);
        put_pixel(sur, centre_x + y, centre_y - x, colour);
        put_pixel(sur, centre_x - y, centre_y - x, colour);
    }
}


static void
arc_pixel(surface_t * sur, i32 px, i32 py, i32 centre_x, i32 centre_y, f32 start_angle, f32 end_angle, Colour colour)
{

    f32 a = atan2f((f32)(py - centre_y), (f32)(px - centre_x));
    if (a < 0)
        a += 2.0f * PI_f;
    i32 in_arc = (start_angle <= end_angle)
                     ? (a >= start_angle && a <= end_angle)
                     : (a >= start_angle || a <= end_angle);
    if (in_arc)
        put_pixel(sur, px, py, colour);
}

void draw_arc_lines(surface_t * sur, i32 centre_x, i32 centre_y, i32 radius, f32 start_angle, f32 end_angle, Colour colour)
{
    // Normalise so start < end, both in [0, 2pi)
    while (start_angle < 0)
        start_angle += 2.0f * PI_f;
    while (end_angle < 0)
        end_angle += 2.0f * PI_f;
    while (start_angle >= 2 * PI_f)
        start_angle -= 2.0f * PI_f;
    while (end_angle >= 2 * PI_f)
        end_angle -= 2.0f * PI_f;

    i32 x = 0;
    i32 y = radius;
    i32 d = 3 - 2 * radius;

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
        }
        else {
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


void platform_sleep(u64 ms)
{
    rt_sleep(ms);
}


#ifdef _WIN32
#include <stdio.h>
#include <windows.h>

void init_console()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
    }
}

void shutdown_console()
{
    fflush(stdout);
    fflush(stderr);

    fclose(stdout);
    fclose(stderr);
    fclose(stdin);

    FreeConsole();
}

#else
void init_console()
{ }

void shutdown_console()
{ }

#endif

// --- stolen stb_decompress (public domain, Sean Barrett, via github.com/nothings/stb) ---

static unsigned int stb_decompress_length(unsigned char const * input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *       stb__barrier_out_e, *stb__barrier_out_b;
static unsigned char const * stb__barrier_in_b;
static unsigned char *       stb__dout;

static void stb__match(unsigned char const * data, unsigned int length)
{
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

static void stb__lit(unsigned char const * data, unsigned int length)
{
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

static unsigned char const * stb_decompress_token(unsigned char const * i)
{
    if (*i >= 0x20) {
        if (*i >= 0x80)
            stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)
            stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
        else
            stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    }
    else {
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

static unsigned int stb_adler32(unsigned int adler32, unsigned char * buffer, unsigned int buflen)
{
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

static unsigned int stb_decompress(unsigned char * output, unsigned char const * i, unsigned int length)
{
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
