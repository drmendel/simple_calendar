#include "functions.h"

#ifdef _WIN32

void goto_xy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void cursor(int on)
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.dwSize   = 1;
    cursorInfo.bVisible = on ? TRUE : FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void sc_clear(void) { system("cls"); }

void sleep_ms(unsigned int ms) { Sleep(ms); }

#else

void goto_xy(int x, int y)
{
    printf("\x1b[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void cursor(int on)
{
    if (on)
        printf("\x1b[?25h");
    else
        printf("\x1b[?25l");
    fflush(stdout);
}

void sc_clear(void) { system("clear"); }

void sleep_ms(unsigned int ms) { usleep((useconds_t)ms * 1000); }

int kbhit(void)
{
    struct termios oldt, newt;
    int ch, oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt          = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int sc_getch(void)
{
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt          = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#endif

// ==================== STRING LIST ====================

void new_str(stringList** buffer, char* input)
{
    stringList* head  = (*buffer);
    unsigned int size = strlen(input) + 1;

    if ((*buffer) == NULL)
    {
        (*buffer)         = (stringList*)malloc(sizeof(stringList));
        (*buffer)->next   = NULL;
        (*buffer)->string = (char*)malloc(sizeof(char) * size);
        memset((*buffer)->string, 0, size);
        memcpy((*buffer)->string, input, size);
    }
    else
    {
        while (head->next != NULL)
            head = head->next;
        head->next         = (stringList*)malloc(sizeof(stringList));
        head->next->next   = NULL;
        head->next->string = (char*)malloc(sizeof(char) * size);
        memset(head->next->string, 0, size);
        memcpy(head->next->string, input, size);
    }
}

int calc_str_list_len(stringList* list)
{
    int length = 0;
    while (list != NULL)
    {
        if (list->string != NULL)
            for (int i = 0; list->string[i] != '\0'; i++)
                length++;
        list = list->next;
    }
    return length;
}

void print_str(stringList* buffer)
{
    unsigned int size = calc_str_list_len(buffer);
    char* big_buffer  = (char*)malloc(sizeof(char) * (size + 1));
    memset(big_buffer, 0, size + 1);
    while (buffer != NULL)
    {
        strcat(big_buffer, buffer->string);
        buffer = buffer->next;
    }
    goto_xy(0, 0);
    printf("%s", big_buffer);
    fflush(stdout);
    free(big_buffer);
}

void free_str(stringList** buffer)
{
    stringList* tmp;
    while ((*buffer) != NULL)
    {
        tmp = (*buffer)->next;
        free((*buffer)->string);
        free(*buffer);
        *buffer = tmp;
    }
}

// ==================== TABLE FORMATTING ====================

#define COL_STAT   4
#define COL_DATE  17
#define COL_CDOWN 22
#define COL_TEXT  25
#define COL_BUF  128

static const char* SECTION_NAMES[] = {
    "ALREADY LATE", "IN A DAY", "IN A WEEK", "IN A MONTH", "IN A LIFE"
};

static int get_section(time_t rem)
{
    if (rem <= 0)              return 0;
    if (rem <= 24 * 3600)      return 1;
    if (rem <= 7 * 24 * 3600)  return 2;
    if (rem <= 31 * 24 * 3600) return 3;
    return 4;
}

static int utf8_width(const char* s)
{
    int w = 0;
    for (; *s; s++)
        if ((*s & 0xC0) != 0x80) w++;
    return w;
}

static void trunc_field(char* dst, const char* src, int width)
{
    int disp_w  = utf8_width(src);
    int byte_len = (int)strlen(src);

    if (disp_w <= width)
    {
        memcpy(dst, src, byte_len);
        int pad = width - disp_w;
        memset(dst + byte_len, ' ', pad);
        dst[byte_len + pad] = '\0';
    }
    else
    {
        int cols = 0, i = 0;
        while (src[i] && cols < width - 3)
        {
            if ((src[i] & 0xC0) != 0x80)
            {
                if (cols == width - 3) break;
                cols++;
            }
            i++;
        }
        memcpy(dst, src, i);
        dst[i]     = '.';
        dst[i + 1] = '.';
        dst[i + 2] = '.';
        dst[i + 3] = '\0';
    }
}

static int table_width(int all, int date_w)
{
    int w = 1;
    if (all) w += COL_STAT + 3;
    w += date_w + 3;
    w += (COL_TEXT + 3) * 3;
    return w;
}

static void print_border(int all, int date_w,
                          const char* l, const char* h,
                          const char* c, const char* r)
{
    int widths[5], ncols;
    if (all)
    {
        widths[0] = COL_STAT; widths[1] = date_w;
        widths[2] = COL_TEXT; widths[3] = COL_TEXT; widths[4] = COL_TEXT;
        ncols = 5;
    }
    else
    {
        widths[0] = date_w;
        widths[1] = COL_TEXT; widths[2] = COL_TEXT; widths[3] = COL_TEXT;
        ncols = 4;
    }
    printf("%s", l);
    for (int col = 0; col < ncols; col++)
    {
        for (int i = 0; i < widths[col] + 2; i++) printf("%s", h);
        printf("%s", col < ncols - 1 ? c : r);
    }
    printf("\n");
}

static void print_hdr_row(int all, int date_w, const char* label)
{
    if (all) printf("│ %-*s ", COL_STAT, "Stat");
    printf("│ %-*s │ %-*s │ %-*s │ %-*s │\n",
           date_w, label, COL_TEXT, "Title", COL_TEXT, "Place", COL_TEXT, "Note");
}

static void print_section_title(const char* name)
{
    printf("\n %s\n", name);
}

static void str_border(stringList** buf, int all, int date_w,
                        const char* l, const char* h,
                        const char* c, const char* r)
{
    int widths[5], ncols;
    if (all)
    {
        widths[0] = COL_STAT; widths[1] = date_w;
        widths[2] = COL_TEXT; widths[3] = COL_TEXT; widths[4] = COL_TEXT;
        ncols = 5;
    }
    else
    {
        widths[0] = date_w;
        widths[1] = COL_TEXT; widths[2] = COL_TEXT; widths[3] = COL_TEXT;
        ncols = 4;
    }
    int bufsize = table_width(all, date_w) * 4 + 8;
    char* s     = (char*)malloc(bufsize);
    s[0]        = '\0';
    strcat(s, l);
    for (int col = 0; col < ncols; col++)
    {
        for (int i = 0; i < widths[col] + 2; i++) strcat(s, h);
        strcat(s, col < ncols - 1 ? c : r);
    }
    strcat(s, "\n");
    new_str(buf, s);
    free(s);
}

static void str_hdr_row(stringList** buf, int all, int date_w, const char* label)
{
    char row[256];
    if (all)
        snprintf(row, sizeof(row), "│ %-*s │ %-*s │ %-*s │ %-*s │ %-*s │\n",
                 COL_STAT, "Stat", date_w, label,
                 COL_TEXT, "Title", COL_TEXT, "Place", COL_TEXT, "Note");
    else
        snprintf(row, sizeof(row), "│ %-*s │ %-*s │ %-*s │ %-*s │\n",
                 date_w, label, COL_TEXT, "Title", COL_TEXT, "Place", COL_TEXT, "Note");
    new_str(buf, row);
}

static void str_section_title(stringList** buf, const char* name)
{
    int len = (int)strlen(name) + 4;
    char* s = (char*)malloc(len);
    sprintf(s, "\n %s\n", name);
    new_str(buf, s);
    free(s);
}

static void fmt_cd_time(time_t rem, char* out)
{
    time_t a = rem < 0 ? -rem : rem;
    int y    = a / (365 * 24 * 3600);
    a       %= (365 * 24 * 3600);
    int mo   = a / (30 * 24 * 3600);
    a       %= (30 * 24 * 3600);
    int d    = a / (24 * 3600);
    a       %= (24 * 3600);
    int h    = a / 3600;
    a       %= 3600;
    int mi   = a / 60;
    int s    = a % 60;

    sprintf(out, "%c %04d.%02d.%02d. %02d:%02d:%02d",
            rem < 0 ? '-' : ' ', y, mo, d, h, mi, s);
}

// ==================== DEADLINE HELPERS ====================

int same_title(deadline* List, char* Title)
{
    while (List != NULL)
    {
        if (!strcmp(List->title, Title)) return 1;
        List = List->next;
    }
    return 0;
}

char* read_line()
{
    size_t bufferSize = STR_SIZE;
    size_t length     = 0;
    char curChar      = '\0';
    char* input       = (char*)malloc(sizeof(char) * bufferSize);
    if (input == NULL)
    {
        perror("Error allocating memory!");
        exit(EXIT_FAILURE);
    }
    memset(input, 0, bufferSize);

    while ((curChar = getchar()) != '\n' && curChar != '\0' && curChar != EOF)
    {
        if (length < bufferSize - 1)
        {
            input[length++] = curChar;
        }
        else
        {
            bufferSize *= 2;
            char* temp  = (char*)realloc(input, bufferSize);
            if (temp == NULL)
            {
                perror("Error allocating memory!");
                free(input);
                exit(EXIT_FAILURE);
            }
            input           = temp;
            input[length++] = curChar;
        }
    }

    input[length]   = '\0';
    size_t ret_size = length + 1;
    char* ret_input = (char*)malloc(sizeof(char) * ret_size);
    memcpy(ret_input, input, ret_size);
    free(input);
    return ret_input;
}

long read_int()
{
    char* str = read_line();
    char* endptr;
    long num = strtol(str, &endptr, 10);
    if (*endptr == '\0')
    {
        free(str);
        return num;
    }
    free(str);
    return -1;
}

void print_tm_time(struct tm* time, unsigned int is_sec)
{
    if (is_sec)
        printf("%04d.%02d.%02d. %02d:%02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour,
               time->tm_min, time->tm_sec);
    else
        printf("%04d.%02d.%02d. %02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour,
               time->tm_min);
}

void print_dl(struct deadline* dl, int all)
{
    struct tm* t = gmtime(&dl->time);
    char title[COL_BUF], place[COL_BUF], note[COL_BUF];
    trunc_field(title, dl->title, COL_TEXT);
    trunc_field(place, dl->place, COL_TEXT);
    trunc_field(note, dl->note, COL_TEXT);

    if (all)
        printf("│ %-*s │ %04d.%02d.%02d. %02d:%02d │ %s │ %s │ %s │\n",
               COL_STAT, dl->ok ? "OK" : "X",
               t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
               t->tm_hour, t->tm_min, title, place, note);
    else
        printf("│ %04d.%02d.%02d. %02d:%02d │ %s │ %s │ %s │\n",
               t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
               t->tm_hour, t->tm_min, title, place, note);
}

void print_dl_table(struct deadline* dl, int all)
{
    print_border(all, COL_DATE, "┌", "─", "┬", "┐");
    print_hdr_row(all, COL_DATE, "Date");
    print_border(all, COL_DATE, "├", "─", "┼", "┤");
    print_dl(dl, all);
    print_border(all, COL_DATE, "└", "─", "┴", "┘");
}

void str_cd_dl(struct deadline* dl, int all, stringList** buffer)
{
    time_t now;
    time(&now);

    char cd[COL_CDOWN + 1];
    fmt_cd_time(dl->time - now - 3600, cd);

    char title[COL_BUF], place[COL_BUF], note[COL_BUF];
    trunc_field(title, dl->title, COL_TEXT);
    trunc_field(place, dl->place, COL_TEXT);
    trunc_field(note, dl->note, COL_TEXT);

    char row[512];
    if (all)
        snprintf(row, sizeof(row), "│ %-*s │ %s │ %s │ %s │ %s │\n",
                 COL_STAT, dl->ok ? "OK" : "X", cd, title, place, note);
    else
        snprintf(row, sizeof(row), "│ %s │ %s │ %s │ %s │\n",
                 cd, title, place, note);
    new_str(buffer, row);
}

void print_list(struct deadline* List, int all)
{
    time_t now;
    time(&now);
    int cur_sec  = -1;
    int in_table = 0;

    while (List != NULL)
    {
        time_t rem = List->time - now - 3600;
        if (all || !List->ok)
        {
            int sec = get_section(rem);
            if (sec != cur_sec)
            {
                if (in_table)
                    print_border(all, COL_DATE, "└", "─", "┴", "┘");
                print_section_title(SECTION_NAMES[sec]);
                print_border(all, COL_DATE, "┌", "─", "┬", "┐");
                print_hdr_row(all, COL_DATE, "Date");
                print_border(all, COL_DATE, "├", "─", "┼", "┤");
                cur_sec  = sec;
                in_table = 1;
            }
            print_dl(List, all);
        }
        List = List->next;
    }
    if (in_table)
        print_border(all, COL_DATE, "└", "─", "┴", "┘");
}

void str_cd_list(struct deadline* List, int all, stringList** buffer)
{
    time_t now;
    time(&now);
    int cur_sec  = -1;
    int in_table = 0;

    while (List != NULL)
    {
        time_t rem = List->time - now - 3600;
        if (all || !List->ok)
        {
            int sec = get_section(rem);
            if (sec != cur_sec)
            {
                if (in_table)
                    str_border(buffer, all, COL_CDOWN, "└", "─", "┴", "┘");
                str_section_title(buffer, SECTION_NAMES[sec]);
                str_border(buffer, all, COL_CDOWN, "┌", "─", "┬", "┐");
                str_hdr_row(buffer, all, COL_CDOWN, "Remaining");
                str_border(buffer, all, COL_CDOWN, "├", "─", "┼", "┤");
                cur_sec  = sec;
                in_table = 1;
            }
            str_cd_dl(List, all, buffer);
        }
        List = List->next;
    }
    if (in_table)
        str_border(buffer, all, COL_CDOWN, "└", "─", "┴", "┘");
}

struct deadline* new_dl(struct deadline* List)
{
    struct deadline* newNode = (deadline*)malloc(sizeof(deadline));
    newNode->next            = List;

    int sameTitle = 1;
    do
    {
        printf("Title: ");
        newNode->title = read_line();
        sameTitle      = same_title(List, newNode->title);
        if (sameTitle == 1)
        {
            printf("%s already exists!\n", newNode->title);
            free(newNode->title);
        }
    } while (sameTitle);

    printf("Place: ");
    newNode->place = read_line();
    printf("Note: ");
    newNode->note = read_line();

    struct tm dateTime = {0};
    int num;

    do
    {
        printf("Year: ");
        num = read_int();
    } while (num == -1);
    dateTime.tm_year = num - 1900;

    do
    {
        printf("Month: ");
        num = read_int();
    } while (num == -1 || 1 > num || num > 12);
    dateTime.tm_mon = num - 1;

    do
    {
        printf("Day: ");
        num        = read_int();
        int m      = dateTime.tm_mon + 1;
        int maxDay = 30;
        if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
            maxDay = 31;
        else if (m == 2)
            maxDay = ((dateTime.tm_year + 1900) % 4 == 0) ? 29 : 28;
        if (1 > num || num > maxDay) num = -1;
    } while (num == -1);
    dateTime.tm_mday = num;

    do
    {
        printf("Hour: ");
        num = read_int();
    } while (num == -1 || 0 > num || num > 23);
    dateTime.tm_hour = num;

    do
    {
        printf("Minute: ");
        num = read_int();
    } while (num == -1 || 0 > num || num > 59);
    dateTime.tm_min = num;

    newNode->time  = mktime(&dateTime);
    newNode->time += 3600;
    newNode->ok    = 0;
    return newNode;
}

unsigned int del_dl(struct deadline** List, char* Title)
{
    struct deadline* current = *List;
    struct deadline* prev    = NULL;

    while (current != NULL && strcmp(current->title, Title) != 0)
    {
        prev    = current;
        current = current->next;
    }

    if (current == NULL)
    {
        printf("Couldn't find this deadline: %s\n", Title);
        return 1;
    }

    if (prev == NULL)
        *List = current->next;
    else
        prev->next = current->next;

    free(current->title);
    free(current->place);
    free(current->note);
    free(current);
    return 0;
}

void ok_dl(struct deadline** List, char* Title)
{
    struct deadline* current = *List;
    while (current != NULL && strcmp(current->title, Title) != 0)
        current = current->next;
    if (current == NULL)
    {
        printf("Couldn't find this deadline: %s\n", Title);
        return;
    }
    current->ok = 1;
    printf("%s is now finished!\n", Title);
}

void not_dl(struct deadline** List, char* Title)
{
    struct deadline* current = *List;
    while (current != NULL && strcmp(current->title, Title) != 0)
        current = current->next;
    if (current == NULL)
    {
        printf("Couldn't find this deadline: %s\n", Title);
        return;
    }
    current->ok = 0;
    printf("%s is now incomplete!\n", Title);
}

void edit_dl(struct deadline** List, char* Title)
{
    struct deadline* current = *List;
    while (current != NULL && strcmp(current->title, Title) != 0)
        current = current->next;
    if (current == NULL)
    {
        printf("There is no deadline: %s\n", Title);
        return;
    }

    printf("EDIT %s\n", Title);
    printf("1 - Title\n2 - Place\n3 - Note\n4 - Time\n5 - OK\n 6 - all\n");
    printf("\nNumber: ");
    unsigned int mode = read_int();
    switch (mode)
    {
    case 1:
        free(current->title);
        printf("Title: ");
        current->title = read_line();
        break;
    case 2:
        free(current->place);
        printf("Place: ");
        current->place = read_line();
        break;
    case 3:
        free(current->note);
        printf("Note: ");
        current->note = read_line();
        break;
    case 4:
    {
        int num;
        struct tm dateTime = {0};
        do
        {
            printf("Year: ");
            num = read_int();
        } while (num == -1);
        dateTime.tm_year = num - 1900;
        do
        {
            printf("Month: ");
            num = read_int();
        } while (num == -1 || 1 > num || num > 12);
        dateTime.tm_mon = num - 1;
        do
        {
            printf("Day: ");
            num        = read_int();
            int m      = dateTime.tm_mon + 1;
            int maxDay = 30;
            if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
                maxDay = 31;
            else if (m == 2)
                maxDay = ((dateTime.tm_year + 1900) % 4 == 0) ? 29 : 28;
            if (1 > num || num > maxDay) num = -1;
        } while (num == -1);
        dateTime.tm_mday = num;
        do
        {
            printf("Hour: ");
            num = read_int();
        } while (num == -1 || 0 > num || num > 23);
        dateTime.tm_hour = num;
        do
        {
            printf("Minute: ");
            num = read_int();
        } while (num == -1 || 0 > num || num > 59);
        dateTime.tm_min  = num;
        current->time    = mktime(&dateTime);
        current->time   += 3600;
        break;
    }
    case 5:
        printf("Is it ready [0-no, 1-yes]: ");
        while (current->ok = read_int(), current->ok != 0 && current->ok != 1)
            ;
        break;
    default:
        del_dl(List, Title);
        *List = new_dl(*List);
    }
    printf("\n%s was edited:\n", Title);
    print_dl_table(current, 0);
}

struct deadline* merge(struct deadline* list1, struct deadline* list2)
{
    if (!list1) return list2;
    if (!list2) return list1;
    if (list1->time <= list2->time)
    {
        list1->next = merge(list1->next, list2);
        return list1;
    }
    list2->next = merge(list1, list2->next);
    return list2;
}

struct deadline* mergeSort(struct deadline* head)
{
    if (!head || !head->next) return head;
    struct deadline *slow = head, *fast = head->next;
    while (fast && fast->next)
    {
        slow = slow->next;
        fast = fast->next->next;
    }
    struct deadline* secondHalf = slow->next;
    slow->next                  = NULL;
    return merge(mergeSort(head), mergeSort(secondHalf));
}

void free_list(struct deadline* List)
{
    deadline* tmp;
    while (List != NULL)
    {
        tmp = List->next;
        free(List->title);
        free(List->place);
        free(List->note);
        free(List);
        List = tmp;
    }
}

void save_file(const char* filename, deadline* head)
{
    FILE* file = fopen(filename, "wb");
    if (!file)
    {
        perror("Error opening file");
        return;
    }
    deadline* current = head;
    while (current != NULL)
    {
        fwrite(&(current->time), sizeof(time_t), 1, file);
        size_t len;
        len = strlen(current->title) + 1;
        fwrite(&len, sizeof(size_t), 1, file);
        fwrite(current->title, 1, len, file);
        len = strlen(current->place) + 1;
        fwrite(&len, sizeof(size_t), 1, file);
        fwrite(current->place, 1, len, file);
        len = strlen(current->note) + 1;
        fwrite(&len, sizeof(size_t), 1, file);
        fwrite(current->note, 1, len, file);
        fwrite(&(current->ok), sizeof(unsigned int), 1, file);
        current = current->next;
    }
    fclose(file);
}

deadline* read_file(FILE* file)
{
    deadline *head = NULL, *current = NULL;
    while (1)
    {
        deadline* newNode = (deadline*)malloc(sizeof(deadline));
        if (!newNode)
        {
            perror("Error allocating memory");
            fclose(file);
            return NULL;
        }
        size_t len;
        if (fread(&(newNode->time), sizeof(time_t), 1, file) != 1)
        {
            free(newNode);
            break;
        }
        fread(&len, sizeof(size_t), 1, file);
        newNode->title = (char*)malloc(len);
        fread(newNode->title, 1, len, file);
        fread(&len, sizeof(size_t), 1, file);
        newNode->place = (char*)malloc(len);
        fread(newNode->place, 1, len, file);
        fread(&len, sizeof(size_t), 1, file);
        newNode->note = (char*)malloc(len);
        fread(newNode->note, 1, len, file);
        fread(&(newNode->ok), sizeof(unsigned int), 1, file);
        newNode->next = NULL;
        if (!head)
        {
            head    = newNode;
            current = newNode;
        }
        else
        {
            current->next = newNode;
            current       = newNode;
        }
    }
    fclose(file);
    return head;
}
