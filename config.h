#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <shlobj.h>
    #define PATH_SEP "\\"
#else
    #include <unistd.h>
    #include <pwd.h>
    #define PATH_SEP "/"
#endif


typedef enum {
    POS_TOP_LEFT = 0,
    POS_TOP_CENTER,
    POS_TOP_RIGHT,
    POS_CENTER_LEFT,
    POS_CENTER,
    POS_CENTER_RIGHT,
    POS_BOTTOM_LEFT,
    POS_BOTTOM_CENTER,
    POS_BOTTOM_RIGHT
} Position;


typedef enum {
    STYLE_HASH = 0,      
    STYLE_BLOCK,         
    STYLE_BINARY         
} ClockStyle;


typedef enum {
    SEP_COLON = 0,       
    SEP_DOT,             
    SEP_COMMA,           
    SEP_SLASH            
} Separator;


typedef enum {
    ORDER_HMS = 0,       
    ORDER_HM,            
    ORDER_MS,            
    ORDER_SMH,           
    ORDER_MH             
} TimeOrder;


typedef struct {
    Position position;
    int color_r;
    int color_g;
    int color_b;
    ClockStyle style;
    int format_24h;      
    Separator separator;
    TimeOrder order;
    int show_seconds;    
    int blink_separator; 
} Config;


int get_home_directory(char *buffer, size_t size);
int ensure_config_directory(void);
int get_config_path(char *buffer, size_t size);
int load_config(Config *config);
int save_config(const Config *config);
void set_default_config(Config *config);
int parse_hex_color(const char *hex, int *r, int *g, int *b);

#endif