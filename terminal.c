#include "terminal.h"
#include <string.h>

#ifdef _WIN32

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void init_terminal(void) {
}

void restore_terminal(void) {
}

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

    SetConsoleOutputCP(CP_UTF8);
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

#include <termios.h>
#include <fcntl.h>

static struct termios saved_termios;
static int saved_flags = -1;
static int terminal_ready = 0;

void init_terminal(void) {
    struct termios t;

    if (tcgetattr(STDIN_FILENO, &saved_termios) != 0)
        return;

    t = saved_termios;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &t) != 0)
        return;

    saved_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (saved_flags != -1)
        fcntl(STDIN_FILENO, F_SETFL, saved_flags | O_NONBLOCK);

    terminal_ready = 1;
}

void restore_terminal(void) {
    if (!terminal_ready)
        return;

    if (saved_flags != -1)
        fcntl(STDIN_FILENO, F_SETFL, saved_flags);
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
    terminal_ready = 0;
}

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

int poll_key(void) {
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return 0;

    if (GetFileType(hIn) != FILE_TYPE_CHAR) {
        char c = 0;
        DWORD read = 0;
        DWORD available = 0;
        if (PeekNamedPipe(hIn, NULL, 0, NULL, &available, NULL)) {
            if (available == 0) return 0;
        }
        if (ReadFile(hIn, &c, 1, &read, NULL) && read == 1) return c;
        return 0;
    }

    while (1) {
        DWORD events = 0;
        if (!GetNumberOfConsoleInputEvents(hIn, &events) || events == 0) {
            return 0;
        }
        INPUT_RECORD rec;
        DWORD consumed = 0;
        if (!ReadConsoleInput(hIn, &rec, 1, &consumed) || consumed == 0) {
            return 0;
        }
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            char c = (char)rec.Event.KeyEvent.uChar.AsciiChar;
            if (c != 0) return c;
        }
    }
#else
    if (!terminal_ready)
        return 0;

    char c = 0;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return (unsigned char)c;
    return 0;
#endif
}

void display_too_small(int h, int m, int s, const Config *config) {
    TerminalSize term = get_terminal_size();

    int display_h = h;
    if (!config->format_24h) {
        if (display_h == 0) display_h = 12;
        else if (display_h > 12) display_h -= 12;
    }

    char sep;
    switch (config->separator) {
        case SEP_DOT:   sep = '.'; break;
        case SEP_COMMA: sep = ','; break;
        case SEP_SLASH: sep = '/'; break;
        default:        sep = ':'; break;
    }

    char tstr[32];
    if (config->show_seconds) {
        snprintf(tstr, sizeof(tstr), "%02d%c%02d%c%02d",
                 display_h, sep, m, sep, s);
    } else {
        snprintf(tstr, sizeof(tstr), "%02d%c%02d", display_h, sep, m);
    }

    const char *message = "Window is too small";
    const char *hint = "Enlarge the window or press Q to quit";

    set_color_rgb(config->color_r, config->color_g, config->color_b);

    int msg_x = (term.width - (int)strlen(message)) / 2;
    if (msg_x < 1) msg_x = 1;
    int msg_y = term.height / 2;
    if (msg_y < 1) msg_y = 1;
    move_cursor(msg_x, msg_y);
    printf("%s", message);

    int hint_x = (term.width - (int)strlen(hint)) / 2;
    if (hint_x < 1) hint_x = 1;
    move_cursor(hint_x, msg_y + 1);
    printf("%s", hint);

    int tx = (term.width - (int)strlen(tstr)) / 2;
    if (tx < 1) tx = 1;
    move_cursor(tx, term.height - 1);
    printf("%s", tstr);

    reset_color();
    fflush(stdout);
}

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
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

static void put_codepoint(unsigned int cp) {
    if (cp < 0x80) {
        putchar((int)cp);
    } else if (cp < 0x800) {
        putchar(0xC0 | (cp >> 6));
        putchar(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        putchar(0xE0 | (cp >> 12));
        putchar(0x80 | ((cp >> 6) & 0x3F));
        putchar(0x80 | (cp & 0x3F));
    } else {
        putchar(0xF0 | (cp >> 18));
        putchar(0x80 | ((cp >> 12) & 0x3F));
        putchar(0x80 | ((cp >> 6) & 0x3F));
        putchar(0x80 | (cp & 0x3F));
    }
}

void set_color_rgb(int r, int g, int b) {
    printf("\033[38;2;%d;%d;%dm", r, g, b);
}

void reset_color(void) {
    printf("\033[0m");
}

void display_clock(ClockBuffer *buf, const Config *config, int blink_state) {
    (void)blink_state;
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
            put_codepoint(buf->data[row][col]);
        }
    }
    
    reset_color();
    fflush(stdout);
}