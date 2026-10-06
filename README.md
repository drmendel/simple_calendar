# SIMPLE CALENDAR

A minimalist terminal deadline manager with ANSI-colored UI, Unicode box-drawing tables, and live countdown mode.

## Features

- Create, edit, delete, and mark deadlines as done/incomplete
- Sorted list view with color-coded urgency sections (late, day, week, month, beyond)
- Live countdown mode with auto-refresh and terminal resize handling
- Adaptive table layout — columns shrink and drop progressively on narrow terminals
- Content-fit column widths sized to the longest entry
- Tab completion and arrow-key line editing at the command prompt
- Cross-platform: Linux, macOS, and Windows

## Build

```bash
# Linux / macOS
make

# Windows (MinGW)
gcc main.c functions.c -o SimpleCalendar.exe

# With icon on Windows
windres icon.rc -O coff -o icon.o
gcc main.c functions.c icon.o -o SimpleCalendar.exe
```

## Usage

```
  h     help          Print available commands
  n     new           Add a new deadline
  d     del           Delete a deadline
  ok                  Mark as done
  ic    incomplete    Mark as incomplete
  e     edit          Edit a deadline
  ls    list          List remaining deadlines
  als   alist         List all deadlines
  cd    countdown     Live countdown of remaining deadlines
  acd   acountdown    Live countdown of all deadlines
  c     clear         Clear the screen
  s     save          Save to deadlines.bin
  ex    exit          Exit the program
```
