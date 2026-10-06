#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STR_SIZE 20

#define CLR_RESET "\x1b[0m"
#define CLR_BOLD  "\x1b[1m"
#define CLR_DIM   "\x1b[2m"
#define CLR_RED   "\x1b[1;31m"
#define CLR_GRN   "\x1b[1;32m"
#define CLR_YEL   "\x1b[1;33m"
#define CLR_BLU   "\x1b[1;34m"
#define CLR_CYN   "\x1b[1;36m"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
typedef unsigned char byte;
int kbhit(void);
int sc_getch(void);
#endif

#ifdef _WIN32
void goto_xy(int, int);
#define sc_getch() _getch()
#define sc_kbhit() _kbhit()
#else
void goto_xy(int x, int y);
#define sc_kbhit() kbhit()
#endif

void cursor(int);
void sc_clear(void);
void sleep_ms(unsigned int);

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

void new_str(stringList**, char*);
void print_str(stringList*);
void free_str(stringList**);
char* read_line(void);
char* read_cmd(const char* prompt);
long read_int(void);
void print_dl_table(struct deadline*, int);
void print_list(struct deadline*, int);
void str_cd_list(struct deadline*, int, stringList** buffer);
unsigned int del_dl(struct deadline**, char*);
void ok_dl(struct deadline**, char*);
void not_dl(struct deadline**, char*);
void edit_dl(struct deadline**, char*);
struct deadline* mergeSort(struct deadline*);
void free_list(struct deadline*);
struct deadline* new_dl(struct deadline*);
void save_file(const char*, deadline*);
deadline* read_file(FILE* file);

extern volatile int g_resized;
void install_resize_handler(void);

#endif
