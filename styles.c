#include "styles.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static void clear_buffer(ClockBuffer *buf) {
    memset(buf->data, ' ', sizeof(buf->data));
    buf->width = 0;
    buf->height = 0;
}

static void put_char(ClockBuffer *buf, int x, int y, char c) {
    if (x < 0 || y < 0 || x >= MAX_CLOCK_WIDTH || y >= MAX_CLOCK_HEIGHT)
        return;
    buf->data[y][x] = c;
    if (x + 1 > buf->width) buf->width = x + 1;
    if (y + 1 > buf->height) buf->height = y + 1;
}

static void put_str(ClockBuffer *buf, int x, int y, const char *s) {
    for (int i = 0; s[i]; i++)
        put_char(buf, x + i, y, s[i]);
}

static const char *digit_7seg[10][5] = {
    {" ### ", "#   #", "#   #", "#   #", " ### "}, 
    {"  #  ", " ##  ", "  #  ", "  #  ", " ### "}, 
    {" ### ", "    #", " ### ", "#    ", " ### "}, 
    {" ### ", "    #", " ### ", "    #", " ### "}, 
    {"#   #", "#   #", " ### ", "    #", "    #"}, 
    {" ### ", "#    ", " ### ", "    #", " ### "}, 
    {" ### ", "#    ", " ### ", "#   #", " ### "}, 
    {" ### ", "    #", "    #", "    #", "    #"}, 
    {" ### ", "#   #", " ### ", "#   #", " ### "}, 
    {" ### ", "#   #", " ### ", "    #", " ### "}  
};

static void draw_digit(ClockBuffer *buf, int x, int y, int d, char on) {
    const char *digit = digit_7seg[d][0];
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            char ch = digit_7seg[d][r][c];
            if (ch != ' ') {
                put_char(buf, x + c, y + r, on);
            }
        }
    }
}

static void draw_colon(ClockBuffer *buf, int x, int y, char c) {
    put_char(buf, x, y + 1, c);
    put_char(buf, x, y + 3, c);
}

static void render_digital(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *cfg,
    char pixel
) {
    clear_buffer(buf);

    int x = 0;
    int y = 0;

    draw_digit(buf, x, y, h / 10, pixel); x += 6;
    draw_digit(buf, x, y, h % 10, pixel); x += 6;
    
    draw_colon(buf, x, y, pixel); x += 2;
    
    draw_digit(buf, x, y, m / 10, pixel); x += 6;
    draw_digit(buf, x, y, m % 10, pixel); x += 6;
    
    if (cfg->show_seconds) {
        draw_colon(buf, x, y, pixel); x += 2;
        draw_digit(buf, x, y, s / 10, pixel); x += 6;
        draw_digit(buf, x, y, s % 10, pixel);
    }
}


void render_hash_clock(ClockBuffer *buf, int h, int m, int s, const Config *cfg) {
    render_digital(buf, h, m, s, cfg, '#');
}

void render_block_clock(ClockBuffer *buf, int h, int m, int s, const Config *cfg) {
    render_digital(buf, h, m, s, cfg, (char)219);  
}



static void draw_binary_row(ClockBuffer *buf, int y, int value, int bits) {
    for (int i = 0; i < bits; i++) {
        int bit = (value >> (bits - 1 - i)) & 1;
        put_char(buf, i * 2, y, bit ? (char)219 : '.');
    }
}

void render_binary_clock(ClockBuffer *buf, int h, int m, int s, const Config *cfg) {
    clear_buffer(buf);

    put_str(buf, 0, 0, "H:");
    draw_binary_row(buf, 1, h, 6);

    put_str(buf, 0, 3, "M:");
    draw_binary_row(buf, 4, m, 6);

    if (cfg->show_seconds) {
        put_str(buf, 0, 6, "S:");
        draw_binary_row(buf, 7, s, 6);
    }
}



static void draw_circle(ClockBuffer *buf, int cx, int cy, int r) {
    for (int a = 0; a < 360; a += 5) {  
        double rad = a * M_PI / 180.0;
        int x = cx + (int)(r * cos(rad));
        int y = cy + (int)(r * sin(rad));
        put_char(buf, x, y, 'o');
    }
}

static void draw_line(
    ClockBuffer *buf,
    int x0, int y0,
    int x1, int y1,
    char c
) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        put_char(buf, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void render_analog_clock(ClockBuffer *buf, int h, int m, int s, const Config *cfg) {
    clear_buffer(buf);

    const int size = 25;
    const int cx = size / 2;
    const int cy = size / 2;
    const int r  = size / 2 - 2;

    buf->width  = size;
    buf->height = size;

    draw_circle(buf, cx, cy, r);

    
    for (int n = 1; n <= 12; n++) {
        double a = (n - 3) * 30 * M_PI / 180.0;
        int x = cx + (int)((r - 2) * cos(a));
        int y = cy + (int)((r - 2) * sin(a));
        if (n < 10) {
            put_char(buf, x, y, '0' + n);
        } else {
            put_char(buf, x - 1, y, '1');
            put_char(buf, x,     y, '2');
        }
    }

    
    double ah = ((h % 12) + m / 60.0) * 30 * M_PI / 180.0;
    double am = m * 6 * M_PI / 180.0;
    double as = s * 6 * M_PI / 180.0;

    
    draw_line(buf, cx, cy,
        cx + (int)((r - 6) * cos(ah)),
        cy + (int)((r - 6) * sin(ah)),
        'H');

    
    draw_line(buf, cx, cy,
        cx + (int)((r - 3) * cos(am)),
        cy + (int)((r - 3) * sin(am)),
        'M');

    
    if (cfg->show_seconds) {
        draw_line(buf, cx, cy,
            cx + (int)((r - 1) * cos(as)),
            cy + (int)((r - 1) * sin(as)),
            '.');
    }

    
    put_char(buf, cx, cy, '+');
}