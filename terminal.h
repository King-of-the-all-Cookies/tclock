#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include "config.h"
#include "styles.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

typedef struct {
    int width;
    int height;
} TerminalSize;


void sleep_ms(int milliseconds);
void clear_screen(void);
void hide_cursor(void);
void show_cursor(void);
void move_cursor(int x, int y);
void set_color_rgb(int r, int g, int b);
void reset_color(void);
TerminalSize get_terminal_size(void);
void enable_ansi_windows(void);
void init_terminal(void);
void restore_terminal(void);


void display_clock(ClockBuffer *buf, const Config *config, int blink_state);

int poll_key(void);
void display_too_small(int h, int m, int s, const Config *config);

#endif