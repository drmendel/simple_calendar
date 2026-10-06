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
    printf(on ? "\x1b[?25h" : "\x1b[?25l");
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

/* ── String list ────────────────────────────────────────────── */

void new_str(stringList** buffer, char* input)
{
    unsigned int size  = strlen(input) + 1;
    stringList* node   = (stringList*)malloc(sizeof(stringList));
    node->next         = NULL;
    node->string       = (char*)malloc(sizeof(char) * size);
    memcpy(node->string, input, size);

    if (*buffer == NULL)
    {
        *buffer = node;
        return;
    }
    stringList* tail = *buffer;
    while (tail->next != NULL)
        tail = tail->next;
    tail->next = node;
}

void print_str(stringList* buffer)
{
    int total = 0;
    for (stringList* p = buffer; p; p = p->next)
        total += strlen(p->string);

    char* out = (char*)malloc(total + 1);
    out[0]    = '\0';
    while (buffer != NULL)
    {
        strcat(out, buffer->string);
        buffer = buffer->next;
    }
    goto_xy(0, 0);
    printf("%s\x1b[J", out);
    fflush(stdout);
    free(out);
}

void free_str(stringList** buffer)
{
    stringList* tmp;
    while (*buffer != NULL)
    {
        tmp = (*buffer)->next;
        free((*buffer)->string);
        free(*buffer);
        *buffer = tmp;
    }
}

/* ── Command line editor ────────────────────────────────────── */

#define HIST_SIZE 32
#define CMD_BUF   256

static const char* COMMANDS[] = {
    "help", "new", "del", "ok", "incomplete", "edit",
    "ls", "als", "cd", "acd", "clear", "save", "exit", NULL
};

static char* cmd_history[HIST_SIZE];
static int hist_count = 0;

static void hist_add(const char* cmd)
{
    if (cmd[0] == '\0') return;
    if (hist_count > 0 &&
        strcmp(cmd_history[(hist_count - 1) % HIST_SIZE], cmd) == 0)
        return;
    int idx  = hist_count % HIST_SIZE;
    int slen = (int)strlen(cmd) + 1;
    free(cmd_history[idx]);
    cmd_history[idx] = (char*)malloc(slen);
    memcpy(cmd_history[idx], cmd, slen);
    hist_count++;
}

static void line_clear(int pos, int len)
{
    for (int i = 0; i < pos; i++) printf("\b");
    for (int i = 0; i < len; i++) printf(" ");
    for (int i = 0; i < len; i++) printf("\b");
    fflush(stdout);
}

char* read_cmd(const char* prompt)
{
    printf("%s", prompt);
    fflush(stdout);

    char buf[CMD_BUF];
    int len = 0, pos = 0;
    int hist_idx = hist_count;
    char saved[CMD_BUF];
    saved[0] = '\0';
    buf[0]   = '\0';

#ifdef _WIN32
    /* _getch() is already raw on Windows */
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt          = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN]  = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

    while (1)
    {
#ifdef _WIN32
        int ch = _getch();
#else
        int ch = getchar();
#endif

        if (ch == '\n' || ch == '\r')
        {
            printf("\n");
            break;
        }
        else if (ch == '\t')
        {
            if (len == 0) continue;
            int matches = 0;
            const char* match = NULL;
            for (int i = 0; COMMANDS[i]; i++)
                if (strncmp(buf, COMMANDS[i], len) == 0)
                {
                    matches++;
                    match = COMMANDS[i];
                }
            if (matches == 1 && match)
            {
                line_clear(pos, len);
                strcpy(buf, match);
                len = (int)strlen(buf);
                pos = len;
                printf("%s", buf);
                fflush(stdout);
            }
            else if (matches > 1)
            {
                printf("\n");
                for (int i = 0; COMMANDS[i]; i++)
                    if (strncmp(buf, COMMANDS[i], len) == 0)
                        printf("  %s", COMMANDS[i]);
                printf("\n%s%s", prompt, buf);
                for (int i = len; i > pos; i--) printf("\b");
                fflush(stdout);
            }
        }
        else if (ch == 127 || ch == 8)
        {
            if (pos > 0)
            {
                memmove(buf + pos - 1, buf + pos, len - pos);
                len--;
                pos--;
                buf[len] = '\0';
                printf("\b%s ", buf + pos);
                for (int i = 0; i < len - pos + 1; i++) printf("\b");
                fflush(stdout);
            }
        }
        else if (ch == 21)
        {
            line_clear(pos, len);
            len = pos = 0;
            buf[0] = '\0';
        }
        else if (ch == 1)
        {
            for (int i = 0; i < pos; i++) printf("\b");
            pos = 0;
            fflush(stdout);
        }
        else if (ch == 5)
        {
            printf("%s", buf + pos);
            pos = len;
            fflush(stdout);
        }
#ifdef _WIN32
        else if (ch == 0 || ch == 0xE0)
        {
            int ch2    = _getch();
            int oldest = hist_count > HIST_SIZE ? hist_count - HIST_SIZE : 0;
            if (ch2 == 72 && hist_idx > oldest)
            {
                if (hist_idx == hist_count) memcpy(saved, buf, len + 1);
                hist_idx--;
                line_clear(pos, len);
                strcpy(buf, cmd_history[hist_idx % HIST_SIZE]);
                len = (int)strlen(buf);
                pos = len;
                printf("%s", buf);
                fflush(stdout);
            }
            else if (ch2 == 80 && hist_idx < hist_count)
            {
                hist_idx++;
                line_clear(pos, len);
                if (hist_idx == hist_count)
                    memcpy(buf, saved, strlen(saved) + 1);
                else
                    strcpy(buf, cmd_history[hist_idx % HIST_SIZE]);
                len = (int)strlen(buf);
                pos = len;
                printf("%s", buf);
                fflush(stdout);
            }
            else if (ch2 == 75 && pos > 0)
            {
                printf("\b");
                pos--;
                fflush(stdout);
            }
            else if (ch2 == 77 && pos < len)
            {
                printf("%c", buf[pos]);
                pos++;
                fflush(stdout);
            }
            else if (ch2 == 83 && pos < len)
            {
                memmove(buf + pos, buf + pos + 1, len - pos - 1);
                len--;
                buf[len] = '\0';
                printf("%s ", buf + pos);
                for (int i = 0; i < len - pos + 1; i++) printf("\b");
                fflush(stdout);
            }
        }
#else
        else if (ch == 27)
        {
            int seq1 = getchar();
            if (seq1 != '[') continue;
            int seq2   = getchar();
            int oldest = hist_count > HIST_SIZE ? hist_count - HIST_SIZE : 0;

            if (seq2 == 'A' && hist_idx > oldest)
            {
                if (hist_idx == hist_count) memcpy(saved, buf, len + 1);
                hist_idx--;
                line_clear(pos, len);
                strcpy(buf, cmd_history[hist_idx % HIST_SIZE]);
                len = (int)strlen(buf);
                pos = len;
                printf("%s", buf);
                fflush(stdout);
            }
            else if (seq2 == 'B' && hist_idx < hist_count)
            {
                hist_idx++;
                line_clear(pos, len);
                if (hist_idx == hist_count)
                    memcpy(buf, saved, strlen(saved) + 1);
                else
                    strcpy(buf, cmd_history[hist_idx % HIST_SIZE]);
                len = (int)strlen(buf);
                pos = len;
                printf("%s", buf);
                fflush(stdout);
            }
            else if (seq2 == 'C' && pos < len)
            {
                printf("\x1b[C");
                pos++;
                fflush(stdout);
            }
            else if (seq2 == 'D' && pos > 0)
            {
                printf("\x1b[D");
                pos--;
                fflush(stdout);
            }
            else if (seq2 == '3')
            {
                int seq3 = getchar();
                if (seq3 == '~' && pos < len)
                {
                    memmove(buf + pos, buf + pos + 1, len - pos - 1);
                    len--;
                    buf[len] = '\0';
                    printf("%s ", buf + pos);
                    for (int i = 0; i < len - pos + 1; i++) printf("\b");
                    fflush(stdout);
                }
            }
            else if (seq2 == 'H')
            {
                for (int i = 0; i < pos; i++) printf("\b");
                pos = 0;
                fflush(stdout);
            }
            else if (seq2 == 'F')
            {
                printf("%s", buf + pos);
                pos = len;
                fflush(stdout);
            }
        }
#endif
        else if (ch >= 32 && ch < 127)
        {
            if (len < CMD_BUF - 1)
            {
                memmove(buf + pos + 1, buf + pos, len - pos);
                buf[pos] = (char)ch;
                len++;
                buf[len] = '\0';
                printf("%s", buf + pos);
                pos++;
                for (int i = len; i > pos; i--) printf("\b");
                fflush(stdout);
            }
        }
    }

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    buf[len] = '\0';
    hist_add(buf);

    char* result = (char*)malloc(len + 1);
    memcpy(result, buf, len + 1);
    return result;
}

/* ── Table formatting ───────────────────────────────────────── */

static int utf8_width(const char* s)
{
    int w = 0;
    for (; *s; s++)
        if ((*s & 0xC0) != 0x80) w++;
    return w;
}

static void trunc_field(char* dst, const char* src, int width)
{
    if (width <= 0) { dst[0] = '\0'; return; }

    int disp_w   = utf8_width(src);
    int byte_len = (int)strlen(src);

    if (disp_w <= width)
    {
        memcpy(dst, src, byte_len);
        int pad = width - disp_w;
        memset(dst + byte_len, ' ', pad);
        dst[byte_len + pad] = '\0';
    }
    else if (width < 4)
    {
        int cols = 0, i = 0;
        while (src[i] && cols < width)
        {
            if ((src[i] & 0xC0) != 0x80) cols++;
            i++;
        }
        memcpy(dst, src, i);
        int pad = width - cols;
        if (pad > 0) memset(dst + i, ' ', pad);
        dst[i + (pad > 0 ? pad : 0)] = '\0';
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

static void fmt_cd_time(time_t rem, char* out)
{
    time_t a = rem < 0 ? -rem : rem;
    int d    = (int)(a / (24 * 3600));
    a       %= (24 * 3600);
    int h    = (int)(a / 3600);
    a       %= 3600;
    int m    = (int)(a / 60);
    int s    = (int)(a % 60);

    char* p = out;
    if (rem < 0) *p++ = '-';
    if (d > 365)
        sprintf(p, "%dy %dd %02d:%02d:%02d", d / 365, d % 365, h, m, s);
    else if (d > 0)
        sprintf(p, "%dd %02d:%02d:%02d", d, h, m, s);
    else
        sprintf(p, "%02d:%02d:%02d", h, m, s);
}

#define COL_STAT   4
#define COL_DATE  17
#define COL_CDOWN 22
#define COL_MIN    4
#define COL_BUF  128

static int g_date_w     = COL_DATE;
static int g_title_w    = 5;
static int g_place_w    = 5;
static int g_note_w     = 4;
static int g_show_title = 1;
static int g_show_place = 1;
static int g_show_note  = 1;

volatile int g_resized = 0;

#ifndef _WIN32
static void handle_winch(int sig) { (void)sig; g_resized = 1; }
#endif

void install_resize_handler(void)
{
#ifndef _WIN32
    signal(SIGWINCH, handle_winch);
#endif
}

static int get_term_width(void)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
#endif
    return 120;
}

static void compute_col_widths(struct deadline* dl, int all, int date_w, int single)
{
    g_show_title = 1; g_show_place = 1; g_show_note = 1;

    int max_t = 5, max_p = 5, max_n = 4;
    int max_d = date_w;
    int cd_mode = (date_w == COL_CDOWN);

    if (single)
    {
        int tw = utf8_width(dl->title);
        int pw = utf8_width(dl->place);
        int nw = utf8_width(dl->note);
        if (tw > max_t) max_t = tw;
        if (pw > max_p) max_p = pw;
        if (nw > max_n) max_n = nw;
        if (cd_mode)
        {
            char tmp[64];
            time_t now; time(&now);
            fmt_cd_time(dl->time - now - 3600, tmp);
            max_d = utf8_width(tmp);
            if (max_d < 9) max_d = 9;
        }
    }
    else
    {
        time_t now; time(&now);
        if (cd_mode) max_d = 9;
        struct deadline* p = dl;
        while (p)
        {
            if (all || !p->ok)
            {
                int tw = utf8_width(p->title);
                int pw = utf8_width(p->place);
                int nw = utf8_width(p->note);
                if (tw > max_t) max_t = tw;
                if (pw > max_p) max_p = pw;
                if (nw > max_n) max_n = nw;
                if (cd_mode)
                {
                    char tmp[64];
                    fmt_cd_time(p->time - now - 3600, tmp);
                    int dw = utf8_width(tmp);
                    if (dw > max_d) max_d = dw;
                }
            }
            p = p->next;
        }
    }
    if (max_d > date_w) max_d = date_w;
    g_date_w = max_d;

    int term_w = get_term_width();
    int base   = 1 + (all ? COL_STAT + 3 : 0) + (max_d + 3);
    int avail, total;

    /* Phase 1: all 3 text columns */
    avail = term_w - base - 9;
    total = max_t + max_p + max_n;
    if (avail >= total)
    {
        g_title_w = max_t; g_place_w = max_p; g_note_w = max_n;
        return;
    }
    if (avail >= COL_MIN * 3)
    {
        double ratio = (double)avail / total;
        g_title_w = (int)(max_t * ratio);
        g_place_w = (int)(max_p * ratio);
        g_note_w  = avail - g_title_w - g_place_w;
        if (g_title_w < COL_MIN) g_title_w = COL_MIN;
        if (g_place_w < COL_MIN) g_place_w = COL_MIN;
        if (g_note_w  < COL_MIN) g_note_w  = COL_MIN;
        return;
    }

    /* Phase 2: drop note */
    g_show_note = 0;
    avail = term_w - base - 6;
    total = max_t + max_p;
    if (avail >= total)
    {
        g_title_w = max_t; g_place_w = max_p;
        return;
    }
    if (avail >= COL_MIN * 2)
    {
        double ratio = (double)avail / total;
        g_title_w = (int)(max_t * ratio);
        g_place_w = avail - g_title_w;
        if (g_title_w < COL_MIN) g_title_w = COL_MIN;
        if (g_place_w < COL_MIN) g_place_w = COL_MIN;
        return;
    }

    /* Phase 3: drop place, shrink title */
    g_show_place = 0;
    avail = term_w - base - 3;
    if (avail >= COL_MIN)
    {
        g_title_w = avail > max_t ? max_t : avail;
        return;
    }

    /* Phase 4: shrink date too, keep title */
    avail = term_w - 1 - (all ? COL_STAT + 3 : 0) - 6;
    if (avail >= COL_MIN * 2)
    {
        g_title_w = COL_MIN;
        g_date_w  = avail - COL_MIN;
        if (g_date_w > max_d) g_date_w = max_d;
        return;
    }

    /* Phase 5: drop title */
    g_show_title = 0;
    avail = term_w - 1 - (all ? COL_STAT + 3 : 0) - 3;
    if (avail >= 1)
    {
        g_date_w = avail > max_d ? max_d : avail;
        return;
    }

    g_date_w = 1;
}

static const char* SECTION_NAMES[] = {
    "ALREADY LATE", "IN A DAY", "IN A WEEK", "IN A MONTH", "IN A LIFE"
};

static const char* SECTION_COLORS[] = {
    CLR_RED, CLR_YEL, CLR_GRN, CLR_CYN, CLR_BLU
};

static int get_section(time_t rem)
{
    if (rem <= 0)              return 0;
    if (rem <= 24 * 3600)      return 1;
    if (rem <= 7 * 24 * 3600)  return 2;
    if (rem <= 31 * 24 * 3600) return 3;
    return 4;
}

static int table_width(int all)
{
    int w = 1;
    if (all) w += COL_STAT + 3;
    w += g_date_w + 3;
    if (g_show_title) w += g_title_w + 3;
    if (g_show_place) w += g_place_w + 3;
    if (g_show_note)  w += g_note_w + 3;
    return w;
}

static void print_border(int all,
                          const char* l, const char* h,
                          const char* c, const char* r)
{
    int widths[5], ncols = 0;
    if (all) widths[ncols++] = COL_STAT;
    widths[ncols++] = g_date_w;
    if (g_show_title) widths[ncols++] = g_title_w;
    if (g_show_place) widths[ncols++] = g_place_w;
    if (g_show_note)  widths[ncols++] = g_note_w;

    printf("%s", l);
    for (int col = 0; col < ncols; col++)
    {
        for (int i = 0; i < widths[col] + 2; i++) printf("%s", h);
        printf("%s", col < ncols - 1 ? c : r);
    }
    printf("\n");
}

static void print_hdr_row(int all, const char* label)
{
    if (all)
        printf("│ " CLR_DIM "%-*s" CLR_RESET " ", COL_STAT, "Stat");
    printf("│ " CLR_DIM "%-*s" CLR_RESET " ", g_date_w, label);
    if (g_show_title)
        printf("│ " CLR_DIM "%-*s" CLR_RESET " ", g_title_w, "Title");
    if (g_show_place)
        printf("│ " CLR_DIM "%-*s" CLR_RESET " ", g_place_w, "Place");
    if (g_show_note)
        printf("│ " CLR_DIM "%-*s" CLR_RESET " ", g_note_w, "Note");
    printf("│\n");
}

static void print_section_title(int sec)
{
    printf("\n %s%s%s\n", SECTION_COLORS[sec], SECTION_NAMES[sec], CLR_RESET);
}

static void str_border(stringList** buf, int all,
                        const char* l, const char* h,
                        const char* c, const char* r)
{
    int bufsize = table_width(all) * 4 + 8;
    char* s     = (char*)malloc(bufsize);
    s[0]        = '\0';

    int widths[5], ncols = 0;
    if (all) widths[ncols++] = COL_STAT;
    widths[ncols++] = g_date_w;
    if (g_show_title) widths[ncols++] = g_title_w;
    if (g_show_place) widths[ncols++] = g_place_w;
    if (g_show_note)  widths[ncols++] = g_note_w;

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

static void str_hdr_row(stringList** buf, int all, const char* label)
{
    char row[512];
    int pos = 0;
    if (all)
        pos += snprintf(row + pos, sizeof(row) - pos,
                        "│ " CLR_DIM "%-*s" CLR_RESET " ", COL_STAT, "Stat");
    pos += snprintf(row + pos, sizeof(row) - pos,
                    "│ " CLR_DIM "%-*s" CLR_RESET " ", g_date_w, label);
    if (g_show_title)
        pos += snprintf(row + pos, sizeof(row) - pos,
                        "│ " CLR_DIM "%-*s" CLR_RESET " ", g_title_w, "Title");
    if (g_show_place)
        pos += snprintf(row + pos, sizeof(row) - pos,
                        "│ " CLR_DIM "%-*s" CLR_RESET " ", g_place_w, "Place");
    if (g_show_note)
        pos += snprintf(row + pos, sizeof(row) - pos,
                        "│ " CLR_DIM "%-*s" CLR_RESET " ", g_note_w, "Note");
    snprintf(row + pos, sizeof(row) - pos, "│\n");
    new_str(buf, row);
}

static void str_section_title(stringList** buf, int sec)
{
    char s[64];
    sprintf(s, "\n %s%s%s\n", SECTION_COLORS[sec], SECTION_NAMES[sec], CLR_RESET);
    new_str(buf, s);
}

/* ── Deadline helpers ───────────────────────────────────────── */

static int same_title(deadline* List, char* Title)
{
    while (List != NULL)
    {
        if (!strcmp(List->title, Title)) return 1;
        List = List->next;
    }
    return 0;
}

char* read_line(void)
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

long read_int(void)
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

static void print_dl(struct deadline* dl, int all)
{
    struct tm* t = gmtime(&dl->time);
    char date_raw[64];
    sprintf(date_raw, "%04d.%02d.%02d. %02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);
    char date_buf[COL_BUF];
    trunc_field(date_buf, date_raw, g_date_w);

    if (all)
        printf("│ %s%-*s%s ", dl->ok ? CLR_GRN : CLR_RED,
               COL_STAT, dl->ok ? "OK" : "X", CLR_RESET);
    printf("│ %s ", date_buf);
    if (g_show_title)
    {
        char title[COL_BUF]; trunc_field(title, dl->title, g_title_w);
        printf("│ %s ", title);
    }
    if (g_show_place)
    {
        char place[COL_BUF]; trunc_field(place, dl->place, g_place_w);
        printf("│ %s ", place);
    }
    if (g_show_note)
    {
        char note[COL_BUF]; trunc_field(note, dl->note, g_note_w);
        printf("│ %s ", note);
    }
    printf("│\n");
}

void print_dl_table(struct deadline* dl, int all)
{
    compute_col_widths(dl, all, COL_DATE, 1);
    print_border(all, "┌", "─", "┬", "┐");
    print_hdr_row(all, "Date");
    print_border(all, "├", "─", "┼", "┤");
    print_dl(dl, all);
    print_border(all, "└", "─", "┴", "┘");
}

static void str_cd_dl(struct deadline* dl, int all, time_t now, stringList** buffer)
{
    char cd_raw[64];
    time_t rem = dl->time - now - 3600;
    fmt_cd_time(rem, cd_raw);
    char cd[COL_BUF];
    trunc_field(cd, cd_raw, g_date_w);

    char row[512];
    int pos = 0;
    if (all)
        pos += snprintf(row + pos, sizeof(row) - pos,
                        "│ %s%-*s%s ",
                        dl->ok ? CLR_GRN : CLR_RED,
                        COL_STAT, dl->ok ? "OK" : "X", CLR_RESET);
    pos += snprintf(row + pos, sizeof(row) - pos,
                    "│ %s%s%s ",
                    rem < 0 ? CLR_RED : "", cd, rem < 0 ? CLR_RESET : "");
    if (g_show_title)
    {
        char title[COL_BUF]; trunc_field(title, dl->title, g_title_w);
        pos += snprintf(row + pos, sizeof(row) - pos, "│ %s ", title);
    }
    if (g_show_place)
    {
        char place[COL_BUF]; trunc_field(place, dl->place, g_place_w);
        pos += snprintf(row + pos, sizeof(row) - pos, "│ %s ", place);
    }
    if (g_show_note)
    {
        char note[COL_BUF]; trunc_field(note, dl->note, g_note_w);
        pos += snprintf(row + pos, sizeof(row) - pos, "│ %s ", note);
    }
    snprintf(row + pos, sizeof(row) - pos, "│\n");
    new_str(buffer, row);
}

void print_list(struct deadline* List, int all)
{
    compute_col_widths(List, all, COL_DATE, 0);
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
                    print_border(all, "└", "─", "┴", "┘");
                print_section_title(sec);
                print_border(all, "┌", "─", "┬", "┐");
                print_hdr_row(all, "Date");
                print_border(all, "├", "─", "┼", "┤");
                cur_sec  = sec;
                in_table = 1;
            }
            print_dl(List, all);
        }
        List = List->next;
    }
    if (in_table)
        print_border(all, "└", "─", "┴", "┘");
}

void str_cd_list(struct deadline* List, int all, stringList** buffer)
{
    compute_col_widths(List, all, COL_CDOWN, 0);
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
                    str_border(buffer, all, "└", "─", "┴", "┘");
                str_section_title(buffer, sec);
                str_border(buffer, all, "┌", "─", "┬", "┐");
                str_hdr_row(buffer, all, "Remaining");
                str_border(buffer, all, "├", "─", "┼", "┤");
                cur_sec  = sec;
                in_table = 1;
            }
            str_cd_dl(List, all, now, buffer);
        }
        List = List->next;
    }
    if (in_table)
        str_border(buffer, all, "└", "─", "┴", "┘");
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

static struct deadline* merge(struct deadline* list1, struct deadline* list2)
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
