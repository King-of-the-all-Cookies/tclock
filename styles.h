#ifndef STYLES_H
#define STYLES_H

#include "config.h"

#define MAX_CLOCK_WIDTH  80
#define MAX_CLOCK_HEIGHT 40

typedef struct {
    int width;
    int height;
    char data[MAX_CLOCK_HEIGHT][MAX_CLOCK_WIDTH];
} ClockBuffer;



void render_hash_clock(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *config
);

void render_block_clock(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *config
);

void render_binary_clock(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *config
);

void render_analog_clock(
    ClockBuffer *buf,
    int h, int m, int s,
    const Config *config
);

#endif