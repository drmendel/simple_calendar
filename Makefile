# Detect OS
UNAME_S := $(shell uname -s)

CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter
SRC     = main.c functions.c

ifeq ($(UNAME_S), Linux)
    TARGET  = simple_calendar
    LDFLAGS =
endif
ifeq ($(UNAME_S), Darwin)
    TARGET  = SimpleCalendar
    LDFLAGS =
endif
ifeq ($(OS), Windows_NT)
    TARGET  = SimpleCalendar.exe
    LDFLAGS =
endif

all: $(TARGET)

$(TARGET): $(SRC) functions.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) deadlines.bin

.PHONY: all clean
