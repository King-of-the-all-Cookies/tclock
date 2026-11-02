ifdef OS
   UNAME := Windows_NT
   UNAME_S := Windows_NT
endif

ifeq ($(UNAME), Windows_NT)
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -lm -lshell32
    TARGET = clock.exe
    RM = del /Q $(OBJECTS) $(TARGET) 2>nul || true
else
    UNAME_S := $(shell uname -s 2>/dev/null)
    CC = gcc
    CFLAGS = -Wall -Wextra -O2
    LDFLAGS = -lm
    TARGET = clock
    RM = rm -f $(OBJECTS) $(TARGET)
endif

SOURCES = main.c config.c terminal.c styles.c
OBJECTS = $(SOURCES:.c=.o)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
INSTALL ?= install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

ifeq ($(UNAME_S),Linux)
install: $(TARGET)
	$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	$(INSTALL) -m 755 $(TARGET) "$(DESTDIR)$(BINDIR)/$(TARGET)"
else
install: $(TARGET)
	@echo "Warning: installation is only supported on Linux. $(TARGET) was built but not installed."
endif

clean:
	$(RM)

.PHONY: all clean install
