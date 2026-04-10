#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #include "debugmalloc_better_version.h"

#define STR_SIZE 20

// ==================== DEFINE WINDOWS OR LINUX FUNCTIONS ====================

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

/* byte type is available on Windows but not on Linux */
typedef unsigned char byte;

/* Non-blocking keyboard hit check (POSIX) */
int kbhit(void);

/* Single char read without echo (POSIX) */
int sc_getch(void);
#endif

#ifdef _WIN32
void goto_xy(int, int);
/* Map portable names to Windows CRT equivalents */
#define sc_getch() _getch()
#define sc_kbhit() _kbhit()
#else
/* Move terminal cursor to (x, y) using ANSI escape */
void goto_xy(int x, int y);
#define sc_kbhit() kbhit()
#endif

void cursor(int);

/* Terminal clear — portable name avoids conflict with ncurses wclear() */
void sc_clear(void);

/* Wait milliseconds — portable across Windows and Linux */
void sleep_ms(unsigned int);

// ==================== DEFINE LINKED LIST DEADLINE STRUCTURE ====================

typedef struct deadline
{
    time_t time;
    char* title;
    char* place;
    char* note;
    unsigned int ok;
    struct deadline* next;
} deadline;

typedef struct stringList
{
    char* string;
    struct stringList* next;
} stringList;

int calc_str_list_len(stringList*);
void new_str(stringList**, char*);
void print_str(stringList*);
void free_str(stringList**);
int same_title(deadline* List, char* Title);
char* read_line();
long read_int();
void print_tm_time(struct tm*, unsigned int);
void print_dl(struct deadline*, int);
void str_cd_dl(struct deadline*, int, stringList** buffer);
void str_cd_time(time_t, stringList** buffer);
void print_list(struct deadline*, int);
void str_cd_list(struct deadline*, int, stringList** buffer);
unsigned int del_dl(struct deadline**, char*);
void ok_dl(struct deadline**, char*);
void not_dl(struct deadline**, char*);
void edit_dl(struct deadline**, char*);
struct deadline* merge(struct deadline*, struct deadline*);
struct deadline* mergeSort(struct deadline*);
void free_list(struct deadline*);
struct deadline* new_dl(struct deadline*);
void save_file(const char*, deadline*);
deadline* read_file(FILE* file);

#endif
