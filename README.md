# SIMPLE CALENDAR

<br>

## Welcome to the Simple Calendar project!

This program is designed to help you manage deadlines in a minimalist way.
It comes with features such as creating new deadlines, deleting and editing existing ones, and marking them as finished or unfinished.
Additionally, it includes a countdown feature based on the current time to keep track of upcoming deadlines.

<br>

## Install on Windows:

1. Create a local repository and give it a name, such as SimpleCalendar.
2. Download the executable file (SimpleCalendar.exe) to the SimpleCalendar directory.
3. Try to run SimpleCalendar.exe file.
4. Windows Defender may try to shut it down. Proceed only if you do not have any concerns.

Compile from source using [MinGW GCC](https://sourceforge.net/projects/gcc-win64/) in Windows PowerShell

```powershell
# Check if GCC is installed
gcc --version

# Create a resource script 'icon.rc' for the icon
'1 ICON "icon.ico"' | Out-File -Encoding ASCII icon.rc

# Compile the resource script into a GCC-compatible object file
windres icon.rc -O coff -o icon.o

# Compile the program and link the icon object
gcc main.c functions.c icon.o -o SimpleCalendar.exe

# Run the program
start SimpleCalendar.exe
```
<br>

## Linux & Mac

Unfortunately, this version is not compatible with Linux or macOS.

<br>

## Usage
### Here are the commands to the program:

```c
    h   or         help       // Print out the possible commands.
    n   or          new       // Add a new deadline.
    d   or       delete       // Delete a new deadline.
    o   or           ok       // Mark a deadline as finished.
   ic   or   incomplete       // Mark a deadline as incomplete.
    e   or         edit       // Edit a deadline.
   ls   or         list       // List remaining tasks.
  als   or        alist       // List all deadlines.
   cd   or    countdown       // Display a countdown of remaining tasks.
  acd   or   acountdown       // Display a countdown of all deadlines.
    c   or        clear       // Cleares the console so you never will get lost on the screen.
    s   or         save       // Saves deadlines in a local binary file.
   ex   or         exit       // Exit the program.
```
