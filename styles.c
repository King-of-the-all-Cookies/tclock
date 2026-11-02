#include "styles.h"
#include <stdio.h>
#include <string.h>

#define FULL_BLOCK 0x2588

static void clear_buffer(ClockBuffer *buf) {
    for (int y = 0; y < MAX_CLOCK_HEIGHT; y++)
        for (int x = 0; x < MAX_CLOCK_WIDTH; x++)
            buf->data[y][x] = ' ';
    buf->width = 0;
    buf->height = 0;
}

static void put_char(ClockBuffer *buf, int x, int y, unsigned int c) {
    if (x < 0 || y < 0 || x >= MAX_CLOCK_WIDTH || y >= MAX_CLOCK_HEIGHT)
        return;
    buf->data[y][x] = c;
    if (x + 1 > buf->width) buf->width = x + 1;
    if (y + 1 > buf->height) buf->height = y + 1;
}

static void put_str(ClockBuffer *buf, int x, int y, const char *s) {
    for (int i = 0; s[i]; i++)
        put_char(buf, x + i, y, (unsigned char)s[i]);
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

static void draw_digit(ClockBuffer *buf, int x, int y, int d, unsigned int on) {
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            char ch = digit_7seg[d][r][c];
            if (ch != ' ') {
                put_char(buf, x + c, y + r, on);
            }
        }
    }
}

static void draw_colon(ClockBuffer *buf, int x, int y, unsigned int c) {
    put_char(buf, x, y + 1, c);
    put_char(buf, x, y + 3, c);
}

static void render_digital(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *cfg,
    unsigned int pixel
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
    render_digital(buf, h, m, s, cfg, FULL_BLOCK);
}



static void draw_binary_row(ClockBuffer *buf, int y, int value, int bits) {
    for (int i = 0; i < bits; i++) {
        int bit = (value >> (bits - 1 - i)) & 1;
        put_char(buf, i * 2, y, bit ? FULL_BLOCK : '.');
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



