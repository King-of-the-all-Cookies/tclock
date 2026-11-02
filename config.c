#include "config.h"
#include <errno.h>  


int get_home_directory(char *buffer, size_t size) {
#ifdef _WIN32
    
    const char *home = getenv("USERPROFILE");
    if (home) {
        strncpy(buffer, home, size - 1);
        buffer[size - 1] = '\0';
        return 1;
    }
    
    
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buffer))) {
        return 1;
    }
    return 0;
#else
    
    const char *home = getenv("HOME");
    if (home) {
        strncpy(buffer, home, size - 1);
        buffer[size - 1] = '\0';
        return 1;
    }
    
    
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        strncpy(buffer, pw->pw_dir, size - 1);
        buffer[size - 1] = '\0';
        return 1;
    }
    return 0;
#endif
}


int ensure_config_directory(void) {
    char path[512];
    
    if (!get_home_directory(path, sizeof(path))) {
        return 0;
    }
    
    strcat(path, PATH_SEP);
    strcat(path, ".clock");
    
#ifdef _WIN32
    
    int result = _mkdir(path);
    if (result == 0) {
        
        SetFileAttributesA(path, FILE_ATTRIBUTE_HIDDEN);
        return 1;
    }
    
    if (errno == EEXIST) {
        return 1;
    }
    return 0;
#else
    
    int result = mkdir(path, 0755);
    if (result == 0) {
        return 1;
    }
    
    if (errno == EEXIST) {
        return 1;
    }
    return 0;
#endif
}


int get_config_path(char *buffer, size_t size) {
    if (!get_home_directory(buffer, size)) {
        return 0;
    }
    
    size_t len = strlen(buffer);
    if (len + 20 >= size) return 0;
    
    strcat(buffer, PATH_SEP);
    strcat(buffer, ".clock");
    strcat(buffer, PATH_SEP);
    strcat(buffer, "config.ini");
    
    return 1;
}


void set_default_config(Config *config) {
    config->position = POS_CENTER;
    config->color_r = 255;
    config->color_g = 255;
    config->color_b = 255;
    config->style = STYLE_HASH;
    config->format_24h = 1;
    config->separator = SEP_COLON;
    config->order = ORDER_HMS;
    config->show_seconds = 1;
    config->blink_separator = 0;
}


int parse_hex_color(const char *hex, int *r, int *g, int *b) {
    const char *start = hex;
    if (*start == '#') start++;
    
    if (strlen(start) != 6) return 0;
    
    unsigned int color;
    if (sscanf(start, "%06x", &color) != 1) return 0;
    
    *r = (color >> 16) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = color & 0xFF;
    
    return 1;
}


static char *trim(char *str) {
    while (*str == ' ' || *str == '\t') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}


int load_config(Config *config) {
    set_default_config(config);
    
    char path[512];
    if (!get_config_path(path, sizeof(path))) {
        return 0;
    }
    
    FILE *file = fopen(path, "r");
    if (!file) {
        
        ensure_config_directory();
        save_config(config);
        return 1;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        
        char *trimmed = trim(line);
        if (*trimmed == '#' || *trimmed == ';' || *trimmed == '\0') {
            continue;
        }
        
        char key[64], value[128];
        if (sscanf(trimmed, "%63[^=]=%127[^\n]", key, value) == 2) {
            char *k = trim(key);
            char *v = trim(value);
            
            if (strcmp(k, "position") == 0) {
                if (strcmp(v, "top-left") == 0) config->position = POS_TOP_LEFT;
                else if (strcmp(v, "top-center") == 0) config->position = POS_TOP_CENTER;
                else if (strcmp(v, "top-right") == 0) config->position = POS_TOP_RIGHT;
                else if (strcmp(v, "center-left") == 0) config->position = POS_CENTER_LEFT;
                else if (strcmp(v, "center") == 0) config->position = POS_CENTER;
                else if (strcmp(v, "center-right") == 0) config->position = POS_CENTER_RIGHT;
                else if (strcmp(v, "bottom-left") == 0) config->position = POS_BOTTOM_LEFT;
                else if (strcmp(v, "bottom-center") == 0) config->position = POS_BOTTOM_CENTER;
                else if (strcmp(v, "bottom-right") == 0) config->position = POS_BOTTOM_RIGHT;
            }
            else if (strcmp(k, "color") == 0) {
                parse_hex_color(v, &config->color_r, &config->color_g, &config->color_b);
            }
            else if (strcmp(k, "style") == 0) {
                if (strcmp(v, "hash") == 0) config->style = STYLE_HASH;
                else if (strcmp(v, "block") == 0) config->style = STYLE_BLOCK;
                else if (strcmp(v, "binary") == 0) config->style = STYLE_BINARY;
            }
            else if (strcmp(k, "format") == 0) {
                if (strcmp(v, "24") == 0 || strcmp(v, "24h") == 0) config->format_24h = 1;
                else if (strcmp(v, "12") == 0 || strcmp(v, "12h") == 0) config->format_24h = 0;
            }
            else if (strcmp(k, "separator") == 0) {
                if (strcmp(v, "colon") == 0 || strcmp(v, ":") == 0) config->separator = SEP_COLON;
                else if (strcmp(v, "dot") == 0 || strcmp(v, ".") == 0) config->separator = SEP_DOT;
                else if (strcmp(v, "comma") == 0 || strcmp(v, ",") == 0) config->separator = SEP_COMMA;
                else if (strcmp(v, "slash") == 0 || strcmp(v, "/") == 0) config->separator = SEP_SLASH;
            }
            else if (strcmp(k, "order") == 0) {
                if (strcmp(v, "hms") == 0) config->order = ORDER_HMS;
                else if (strcmp(v, "hm") == 0) config->order = ORDER_HM;
                else if (strcmp(v, "ms") == 0) config->order = ORDER_MS;
                else if (strcmp(v, "smh") == 0) config->order = ORDER_SMH;
                else if (strcmp(v, "mh") == 0) config->order = ORDER_MH;
            }
            else if (strcmp(k, "show_seconds") == 0) {
                config->show_seconds = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);
            }
            else if (strcmp(k, "blink_separator") == 0) {
                config->blink_separator = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);
            }
        }
    }
    
    fclose(file);
    return 1;
}


int save_config(const Config *config) {
    char path[512];
    if (!get_config_path(path, sizeof(path))) {
        return 0;
    }
    
    ensure_config_directory();
    
    FILE *file = fopen(path, "w");
    if (!file) {
        return 0;
    }
    
    fprintf(file, "# Clock Configuration File\n");
    fprintf(file, "# Generated automatically\n\n");
    
    
    const char *pos_names[] = {
        "top-left", "top-center", "top-right",
        "center-left", "center", "center-right",
        "bottom-left", "bottom-center", "bottom-right"
    };
    fprintf(file, "# Position: top-left, top-center, top-right, center-left, center,\n");
    fprintf(file, "#           center-right, bottom-left, bottom-center, bottom-right\n");
    fprintf(file, "position = %s\n\n", pos_names[config->position]);
    
    
    fprintf(file, "# Color in hex format (e.g., #FFFFFF for white)\n");
    fprintf(file, "color = #%02X%02X%02X\n\n", config->color_r, config->color_g, config->color_b);
    
    
    const char *style_names[] = {"hash", "block", "binary"};
    fprintf(file, "# Style: hash, block, binary\n");
    fprintf(file, "style = %s\n\n", style_names[config->style]);
    
    
    fprintf(file, "# Time format: 24 or 12\n");
    fprintf(file, "format = %s\n\n", config->format_24h ? "24" : "12");
    
    
    const char *sep_names[] = {"colon", "dot", "comma", "slash"};
    fprintf(file, "# Separator: colon (:), dot (.), comma (,), slash (/)\n");
    fprintf(file, "separator = %s\n\n", sep_names[config->separator]);
    
    
    const char *order_names[] = {"hms", "hm", "ms", "smh", "mh"};
    fprintf(file, "# Order: hms, hm, ms, smh, mh\n");
    fprintf(file, "order = %s\n\n", order_names[config->order]);
    
    
    fprintf(file, "# Show seconds: true or false\n");
    fprintf(file, "show_seconds = %s\n\n", config->show_seconds ? "true" : "false");
    
    
    fprintf(file, "# Blink separator: true or false\n");
    fprintf(file, "blink_separator = %s\n", config->blink_separator ? "true" : "false");
    
    fclose(file);
    return 1;
}