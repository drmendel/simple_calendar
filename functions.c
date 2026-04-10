#include "functions.h"

// ==================== PLATFORM-SPECIFIC IMPLEMENTATIONS ====================

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

#else /* Linux / macOS */

/* Move cursor to (x,y) using ANSI escape codes — no ncurses needed */
void goto_xy(int x, int y)
{
    printf("\x1b[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void cursor(int on)
{
    if (on)
        printf("\x1b[?25h"); /* show cursor */
    else
        printf("\x1b[?25l"); /* hide cursor */
    fflush(stdout);
}

void sc_clear(void) { system("clear"); }

void sleep_ms(unsigned int ms) { usleep((useconds_t)ms * 1000); }

/* Non-blocking kbhit() for POSIX terminals */
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

/* Single-char read without echo for POSIX */
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

#endif /* _WIN32 */

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

void print_dl(struct deadline* DeadLine, int all)
{
    struct tm* time = gmtime(&DeadLine->time);
    if (all)
        printf(DeadLine->ok == 1 ? "\tOK\t" : "\t X\t");
    else
        printf("\t");
    printf("%04d.%02d.%02d. %02d:%02d\t", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour,
           time->tm_min);
    printf("%-15s\t%-15s\t%s", DeadLine->title, DeadLine->place, DeadLine->note);
}

void str_cd_time(time_t remTime, stringList** buffer)
{
    int remainingYears, remainingMonths, remainingDays, remainingHours, remainingMinutes, remainingSeconds;

    remainingYears    = remTime / (365 * 24 * 60 * 60);
    remTime          %= (365 * 24 * 60 * 60);
    remainingMonths   = remTime / (30 * 24 * 60 * 60);
    remTime          %= (30 * 24 * 60 * 60);
    remainingDays     = remTime / (24 * 60 * 60);
    remTime          %= (24 * 60 * 60);
    remainingHours    = remTime / (60 * 60);
    remTime          %= (60 * 60);
    remainingMinutes  = remTime / 60;
    remainingSeconds  = remTime % 60;

    char* buf = (char*)malloc(100);
    if (remTime < 0)
        sprintf(buf, "- %04d.%02d.%02d. %02d:%02d:%02d", -remainingYears, -remainingMonths, -remainingDays,
                -remainingHours, -remainingMinutes, -remainingSeconds);
    else
        sprintf(buf, "  %04d.%02d.%02d. %02d:%02d:%02d", remainingYears, remainingMonths, remainingDays, remainingHours,
                remainingMinutes, remainingSeconds);
    new_str(buffer, buf);
    free(buf);
}

void str_cd_dl(struct deadline* DeadLine, int all, stringList** buffer)
{
    time_t currentTime;
    time(&currentTime);

    if (all)
        new_str(buffer, DeadLine->ok == 1 ? "\tOK\t" : "\t X\t");
    else
        new_str(buffer, "\t");

    str_cd_time(DeadLine->time - currentTime - 3600, buffer);

    unsigned int size = 1 + strlen(DeadLine->title) + 1 + strlen(DeadLine->place) + 1 + strlen(DeadLine->note) + 100;
    char* dl_str      = (char*)malloc(sizeof(char) * size);
    memset(dl_str, 0, size);
    sprintf(dl_str, "\t%-15s\t%-15s\t%s", DeadLine->title, DeadLine->place, DeadLine->note);
    new_str(buffer, dl_str);
    free(dl_str);
}

void print_list(struct deadline* List, int all)
{
    time_t currentTime;
    time(&currentTime);
    byte late = 1, inDay = 1, inWeek = 1, inMonth = 1, inLife = 1;
    time_t remTime;
    while (List != NULL)
    {
        remTime = List->time - currentTime - 3600;
        if (all || !List->ok)
        {
            if (late)
            {
                printf("ALREADY LATE\n");
                late = 0;
            }
            if (0 < remTime && inDay)
            {
                printf("\nIN A DAY\n");
                inDay = 0;
            }
            if (24 * 3600 < remTime && inWeek)
            {
                printf("\nIN A WEEK\n");
                inWeek = 0;
            }
            if (7 * 24 * 3600 < remTime && inMonth)
            {
                printf("\nIN A MONTH\n");
                inMonth = 0;
            }
            if (31 * 24 * 3600 < remTime && inLife)
            {
                printf("\nIN A LIFE\n");
                inLife = 0;
            }
            print_dl(List, all);
            printf("\n");
        }
        List = List->next;
    }
}

void str_cd_list(struct deadline* List, int all, stringList** buffer)
{
    time_t currentTime;
    time(&currentTime);
    byte late = 1, inDay = 1, inWeek = 1, inMonth = 1, inLife = 1;
    time_t remTime;
    while (List != NULL)
    {
        remTime = List->time - currentTime - 3600;
        if (all || !List->ok)
        {
            if (late)
            {
                new_str(buffer, "ALREADY LATE\n");
                late = 0;
            }
            if (0 < remTime && inDay)
            {
                new_str(buffer, "\nIN A DAY\n");
                inDay = 0;
            }
            if (24 * 3600 < remTime && inWeek)
            {
                new_str(buffer, "\nIN A WEEK\n");
                inWeek = 0;
            }
            if (7 * 24 * 3600 < remTime && inMonth)
            {
                new_str(buffer, "\nIN A MONTH\n");
                inMonth = 0;
            }
            if (31 * 24 * 3600 < remTime && inLife)
            {
                new_str(buffer, "\nIN A LIFE\n");
                inLife = 0;
            }
            str_cd_dl(List, all, buffer);
            new_str(buffer, "\n");
        }
        List = List->next;
    }
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
        printf("Couldn't find this deadline: %s", Title);
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
        printf("Couldn't find this deadline: %s", Title);
        return;
    }
    current->ok = 1;
    printf("%s is now finished!", Title);
}

void not_dl(struct deadline** List, char* Title)
{
    struct deadline* current = *List;
    while (current != NULL && strcmp(current->title, Title) != 0)
        current = current->next;
    if (current == NULL)
    {
        printf("Couldn't find this deadline: %s", Title);
        return;
    }
    current->ok = 0;
    printf("%s is now incomplete!", Title);
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
    print_dl(current, 0);
    printf("\n");
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
