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

```c
    gcc --version                                           // check if gcc is available
    '1 ICON "icon.ico"' | Out-File -Encoding ASCII icon.rc  // create a resource script 'icon.rc'
    windres icon.rc -O coff -o icon.o                       // make a gcc compatable object file for icon
    gcc main.c functions.c -i icon.o -o SimpleCalendar.exe  // make sure that you are not compiling a virus
    .\SimpleCalendar.exe                                    // run the program from terminal the or with the GUI
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
