CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm -lshell32
TARGET = clock.exe
SOURCES = main.c config.c terminal.c styles.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul || true

.PHONY: all clean