#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "terminal.h"
#include "styles.h"


volatile sig_atomic_t running = 1;

void signal_handler(int signum) {
    (void)signum;
    running = 0;
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -c, --config        Show config file path\n");
    printf("  -r, --reset         Reset config to defaults\n");
    printf("\n");
    printf("Configuration is stored in ~/.clock/config.ini\n");
    printf("Edit this file to customize the clock.\n\n");
    printf("Available settings:\n");
    printf("  position   = top-left, top-center, top-right, center-left, center,\n");
    printf("               center-right, bottom-left, bottom-center, bottom-right\n");
    printf("  color      = #RRGGBB (hex color, e.g., #00FF00 for green)\n");
    printf("  style      = hash, block, binary\n");
    printf("  format     = 24 or 12\n");
    printf("  separator  = colon, dot, comma, slash\n");
    printf("  order      = hms, hm, ms, smh, mh\n");
    printf("  blink_separator = true or false\n");
}

int main(int argc, char *argv[]) {
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            char path[512];
            if (get_config_path(path, sizeof(path))) {
                printf("Config file: %s\n", path);
            } else {
                printf("Could not determine config path.\n");
            }
            return 0;
        }
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
            Config config;
            set_default_config(&config);
            if (save_config(&config)) {
                printf("Config reset to defaults.\n");
                char path[512];
                if (get_config_path(path, sizeof(path))) {
                    printf("Config file: %s\n", path);
                }
            } else {
                printf("Failed to reset config.\n");
                return 1;
            }
            return 0;
        }
    }
    
    
    enable_ansi_windows();
    
    
    Config config;
    if (!load_config(&config)) {
        fprintf(stderr, "Warning: Could not load config, using defaults.\n");
        set_default_config(&config);
    }
    
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    
    hide_cursor();
    clear_screen();
    init_terminal();
    
    ClockBuffer buffer;
    int blink_state = 0;
    int too_small = 0;
    
    
    int prev_h = -1, prev_m = -1, prev_s = -1;
    TerminalSize prev_term = {0, 0};
    
    while (running) {
        
        time_t rawtime;
        struct tm *timeinfo;
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        int h = timeinfo->tm_hour;
        int m = timeinfo->tm_min;
        int s = timeinfo->tm_sec;
        
        
        int display_h = h;
        if (!config.format_24h) {
            if (display_h == 0) display_h = 12;
            else if (display_h > 12) display_h -= 12;
        }
        
        
        int key = poll_key();
        if (key == 'q' || key == 'Q') {
            running = 0;
            break;
        }
        int force_redraw = (key != 0);
        
        
        TerminalSize current_term = get_terminal_size();
        int term_changed = (current_term.width != prev_term.width || 
                           current_term.height != prev_term.height);
        
        
        int time_changed = (h != prev_h || m != prev_m || s != prev_s);
        
        if (term_changed || force_redraw) {
            clear_screen();
            prev_term = current_term;
        }
        
        if (time_changed || term_changed || force_redraw) {
            
            
            switch (config.style) {
                case STYLE_HASH:
                    render_hash_clock(&buffer, display_h, m, s, &config);
                    break;
                case STYLE_BLOCK:
                    render_block_clock(&buffer, display_h, m, s, &config);
                    break;
                case STYLE_BINARY:
                    render_binary_clock(&buffer, display_h, m, s, &config);
                    break;
                default:
                    render_hash_clock(&buffer, display_h, m, s, &config);
            }
            
            
            too_small = (current_term.width < buffer.width ||
                        current_term.height < buffer.height);
            
            if (too_small) {
                display_too_small(h, m, s, &config);
            } else {
                display_clock(&buffer, &config, blink_state);
            }
            
            prev_h = h;
            prev_m = m;
            prev_s = s;
        } 
        else if (config.blink_separator && !too_small) {
            
            display_clock(&buffer, &config, blink_state);
        }
        
        
        blink_state = !blink_state;
        
        
        sleep_ms(config.blink_separator ? 500 : 100);
    }
    
    
    restore_terminal();
    clear_screen();
    show_cursor();
    reset_color();
    printf("Clock stopped.\n");
    
    return 0;
}