#include "functions.h"

int main(void)
{
    deadline* List = NULL;
    char* cmd      = NULL;

    FILE* fp = fopen("deadlines.bin", "rb");
    if (fp != NULL) List = read_file(fp);

    sc_clear();

    while (1)
    {
        cmd = NULL;
        printf(CLR_BOLD "  SIMPLE CALENDAR" CLR_RESET "\n\n");
        cmd = read_cmd(" ~ ");

        if (cmd == NULL || cmd[0] == '\0')
        {
            free(cmd);
            continue;
        }

        else if (strcmp(cmd, "h") == 0 || strcmp(cmd, "he") == 0 || strcmp(cmd, "hel") == 0 || strcmp(cmd, "help") == 0)
        {
            printf("\n  " CLR_BOLD "COMMANDS" CLR_RESET "\n\n");
            printf("  " CLR_CYN "n" CLR_RESET "    new deadline\n");
            printf("  " CLR_CYN "d" CLR_RESET "    delete deadline\n");
            printf("  " CLR_CYN "ok" CLR_RESET "   mark as done\n");
            printf("  " CLR_CYN "ic" CLR_RESET "   mark as incomplete\n");
            printf("  " CLR_CYN "e" CLR_RESET "    edit deadline\n\n");
            printf("  " CLR_CYN "ls" CLR_RESET "   list remaining\n");
            printf("  " CLR_CYN "als" CLR_RESET "  list all\n");
            printf("  " CLR_CYN "cd" CLR_RESET "   countdown\n");
            printf("  " CLR_CYN "acd" CLR_RESET "  countdown all\n\n");
            printf("  " CLR_CYN "c" CLR_RESET "    clear screen\n");
            printf("  " CLR_CYN "s" CLR_RESET "    save\n");
            printf("  " CLR_CYN "ex" CLR_RESET "   exit\n\n");
        }

        else if (strcmp(cmd, "n") == 0 || strcmp(cmd, "ne") == 0 || strcmp(cmd, "new") == 0)
        {
            printf("\n  " CLR_BOLD "NEW DEADLINE" CLR_RESET "\n\n");
            List = new_dl(List);
            printf("\nNew deadline:\n");
            print_dl_table(List, 0);
            printf("\n");
        }

        else if (strcmp(cmd, "d") == 0 || strcmp(cmd, "de") == 0 || strcmp(cmd, "del") == 0)
        {
            printf("\n  " CLR_BOLD "DELETE DEADLINE" CLR_RESET "\n\n");
            printf("Title: ");
            char* Title      = read_line();
            unsigned int err = del_dl(&List, Title);
            if (!err) printf("%s is deleted!\n\n", Title);
            free(Title);
        }

        else if (strcmp(cmd, "o") == 0 || strcmp(cmd, "ok") == 0)
        {
            printf("\n  " CLR_BOLD "FINISH DEADLINE" CLR_RESET "\n\n");
            printf("Title: ");
            char* Title = read_line();
            ok_dl(&List, Title);
            printf("\n\n");
            free(Title);
        }

        else if (strcmp(cmd, "i") == 0 || strcmp(cmd, "ic") == 0 || strcmp(cmd, "inc") == 0 ||
                 strcmp(cmd, "inco") == 0 || strcmp(cmd, "incom") == 0 || strcmp(cmd, "incomp") == 0 ||
                 strcmp(cmd, "incompl") == 0 || strcmp(cmd, "incomple") == 0 || strcmp(cmd, "incomplet") == 0 ||
                 strcmp(cmd, "incomplete") == 0)
        {
            printf("\n  " CLR_BOLD "INCOMPLETE DEADLINE" CLR_RESET "\n\n");
            printf("Title: ");
            char* Title = read_line();
            not_dl(&List, Title);
            printf("\n\n");
            free(Title);
        }

        else if (strcmp(cmd, "e") == 0 || strcmp(cmd, "ed") == 0 || strcmp(cmd, "edi") == 0 || strcmp(cmd, "edit") == 0)
        {
            printf("\n  " CLR_BOLD "EDIT DEADLINE" CLR_RESET "\n\n");
            printf("Title: ");
            char* Title = read_line();
            printf("\n");
            edit_dl(&List, Title);
            printf("\n");
            free(Title);
        }

        else if (strcmp(cmd, "l") == 0 || strcmp(cmd, "ls") == 0 || strcmp(cmd, "li") == 0 || strcmp(cmd, "lis") == 0 ||
                 strcmp(cmd, "list") == 0)
        {
            printf("\n  " CLR_BOLD "DEADLINES" CLR_RESET "\n");
            List = mergeSort(List);
            print_list(List, 0);
            printf("\n");
        }

        else if (strcmp(cmd, "al") == 0 || strcmp(cmd, "als") == 0 || strcmp(cmd, "ali") == 0 ||
                 strcmp(cmd, "alis") == 0 || strcmp(cmd, "alist") == 0)
        {
            printf("\n  " CLR_BOLD "ALL DEADLINES" CLR_RESET "\n");
            List = mergeSort(List);
            print_list(List, 1);
            printf("\n");
        }

        else if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "co") == 0 || strcmp(cmd, "cou") == 0 ||
                 strcmp(cmd, "coun") == 0 || strcmp(cmd, "count") == 0 || strcmp(cmd, "countd") == 0 ||
                 strcmp(cmd, "countdo") == 0 || strcmp(cmd, "countdow") == 0 || strcmp(cmd, "countdown") == 0 ||
                 strcmp(cmd, "acd") == 0 || strcmp(cmd, "aco") == 0 || strcmp(cmd, "acou") == 0 ||
                 strcmp(cmd, "acoun") == 0 || strcmp(cmd, "acount") == 0 || strcmp(cmd, "acountd") == 0 ||
                 strcmp(cmd, "acountdo") == 0 || strcmp(cmd, "acountdow") == 0 || strcmp(cmd, "acountdown") == 0)
        {
            int show_all       = (cmd[0] == 'a');
            stringList* buffer = NULL;
            List               = mergeSort(List);
            int cleared        = 0;

            while (1)
            {
                new_str(&buffer, CLR_BOLD "  SIMPLE CALENDAR" CLR_RESET "\n\n");
                new_str(&buffer, CLR_BOLD "  COUNTDOWN" CLR_RESET "\n");
                str_cd_list(List, show_all, &buffer);
                new_str(&buffer, "\n  " CLR_DIM "Press ESC to go back" CLR_RESET);

                if (!cleared)
                {
                    sc_clear();
                    cleared = 1;
                }

                cursor(0);
                print_str(buffer);
                free_str(&buffer);

                if (sc_kbhit())
                {
                    int key = sc_getch();
                    if (key == 27) break;
                }
                sleep_ms(1000);
            }
            sc_clear();
            cursor(1);
        }

        else if (strcmp(cmd, "c") == 0 || strcmp(cmd, "cl") == 0 || strcmp(cmd, "cls") == 0 ||
                 strcmp(cmd, "cle") == 0 || strcmp(cmd, "clea") == 0 || strcmp(cmd, "clear") == 0)
        {
            sc_clear();
        }

        else if (strcmp(cmd, "ex") == 0 || strcmp(cmd, "exi") == 0 || strcmp(cmd, "exit") == 0)
        {
            sc_clear();
            printf(CLR_BOLD "  SIMPLE CALENDAR" CLR_RESET "\n\n");
            printf("Are you sure that you want to do now these interesting tasks? [Y/N]\n");
            printf(" ~ ");
            char* c = read_line();
            if (c != NULL && (!strcmp(c, "N") || !strcmp(c, "n")))
            {
                sc_clear();
                free(cmd);
                free(c);
                printf("\n");
                continue;
            }
            else
            {
                sc_clear();
                printf("GET RID OF YOUR TASKS!!!");
                fflush(stdout);
                sleep_ms(700);
                sc_clear();
                printf("SO DO THEM!!!");
                fflush(stdout);
                sleep_ms(700);
                save_file("deadlines.bin", List);
                sc_clear();
                printf("Bye!");
                fflush(stdout);
                sleep_ms(700);
                sc_clear();
                free_list(List);
                free(cmd);
                if (c) free(c);
                break;
            }
        }

        else if (strcmp(cmd, "s") == 0 || strcmp(cmd, "sa") == 0 || strcmp(cmd, "sav") == 0 || strcmp(cmd, "save") == 0)
        {
            printf("\n  " CLR_BOLD "SAVE" CLR_RESET "\n\n");
            List = mergeSort(List);
            save_file("deadlines.bin", List);
            printf("Deadlines are saved!\n\n");
        }

        free(cmd);
    }

    return 0;
}
