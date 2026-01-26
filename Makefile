ifdef OS
   UNAME := Windows_NT
endif

ifeq ($(UNAME), Windows_NT)
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -lm -lshell32
    TARGET = clock.exe
    RM = del /Q $(OBJECTS) $(TARGET) 2>nul || true
else
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -lm
    TARGET = clock
    RM = rm -f $(OBJECTS) $(TARGET)
endif

SOURCES = main.c config.c terminal.c styles.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM)

.PHONY: all clean
