#include "terminal.h"
#include <string.h>

#ifdef _WIN32


#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void enable_ansi_windows(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;
    }
    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return;
    }
    
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}

TerminalSize get_terminal_size(void) {
    TerminalSize size = {80, 24};
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    return size;
}

#else

void enable_ansi_windows(void) {
    
}

void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

TerminalSize get_terminal_size(void) {
    TerminalSize size = {80, 24};
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        size.width = w.ws_col;
        size.height = w.ws_row;
    }
    return size;
}

#endif

void clear_screen(void) {
    printf("\033[H\033[J");
    fflush(stdout);
}

void hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
    fflush(stdout);
}

void set_color_rgb(int r, int g, int b) {
    printf("\033[38;2;%d;%d;%dm", r, g, b);
}

void reset_color(void) {
    printf("\033[0m");
}

void display_clock(ClockBuffer *buf, const Config *config, int blink_state) {
    TerminalSize term = get_terminal_size();
    int x = 0, y = 0;
    
    
    switch (config->position) {
        case POS_TOP_LEFT:
            x = 0;
            y = 0;
            break;
        case POS_TOP_CENTER:
            x = (term.width - buf->width) / 2;
            y = 0;
            break;
        case POS_TOP_RIGHT:
            x = term.width - buf->width;
            y = 0;
            break;
        case POS_CENTER_LEFT:
            x = 0;
            y = (term.height - buf->height) / 2;
            break;
        case POS_CENTER:
            x = (term.width - buf->width) / 2;
            y = (term.height - buf->height) / 2;
            break;
        case POS_CENTER_RIGHT:
            x = term.width - buf->width;
            y = (term.height - buf->height) / 2;
            break;
        case POS_BOTTOM_LEFT:
            x = 0;
            y = term.height - buf->height;
            break;
        case POS_BOTTOM_CENTER:
            x = (term.width - buf->width) / 2;
            y = term.height - buf->height;
            break;
        case POS_BOTTOM_RIGHT:
            x = term.width - buf->width;
            y = term.height - buf->height;
            break;
    }
    
    
    set_color_rgb(config->color_r, config->color_g, config->color_b);
    
    
    for (int row = 0; row < buf->height; row++) {
        move_cursor(x, y + row);
        for (int col = 0; col < buf->width; col++) {
            putchar(buf->data[row][col]);
        }
    }
    
    reset_color();
    fflush(stdout);
}