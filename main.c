#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <pthread.h>

#define MAX_LINE_LENGTH 1024 * 2

// Global variables
char u_input[MAX_LINE_LENGTH];
char current_working_directory[MAX_LINE_LENGTH];
char current_user[MAX_LINE_LENGTH];
char home_dir[MAX_LINE_LENGTH] = "/home/";
FILE *log_file;
int pattern_count = 0;
pthread_mutex_t pattern_count_mutex;
// end

typedef struct
{
    char *line;
    char *pattern;

} thread_data;


void update_current_user()
{
    struct passwd *pw = getpwuid(getuid());
    char *user_name = pw->pw_name;
    if (strcmp(user_name, "root") == 0)
    {
        printf("Can't run this program as root.\n");
        exit(0);
    }
    strcpy(current_user, user_name);
}

void update_current_working_dir()
{
    char *pwd = getcwd(current_working_directory, sizeof(current_working_directory));
    if (pwd == NULL)
    {
        perror("getcwd error");
        exit(1);
    }
}

void log_action()
{

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log_file, "%d:%d:%d = %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, u_input);
    fflush(log_file);
}

void get_u_input()
{
    // fill u_input with '\0'
    memset(u_input, '\0', MAX_LINE_LENGTH);

    // read user input
    fgets(u_input, MAX_LINE_LENGTH, stdin);

    // remove newline character
    u_input[strlen(u_input) - 1] = '\0';

    // !FOR DEBUGING!
    //  printf("!%s!\n", u_input);
}

void print_nav()
{
    // Update current working directory and user variables
    update_current_user();
    update_current_working_dir();

    // Print current working directory and user in bash style
    printf("%s:%s> ", current_user, current_working_directory);
}


void *find_match(void *arg)
{
    thread_data *data = (thread_data *)arg;

    // !FOR DEBUGING!
    // printf("%s\n", (char* ) data->line);
    // printf("%s\n", (char* ) data->pattern);

    if (strstr(data->line, data->pattern) != NULL)
    {
        printf("%s\n", data->line);

        pthread_mutex_lock(&pattern_count_mutex);
        pattern_count++;
        pthread_mutex_unlock(&pattern_count_mutex);
    }

    pthread_exit(NULL);
}

int cd(char *path)
{
    if (chdir(path) != 0)
    {
        printf("ERROR: No file or directory named `%s`\n", path);
        return -1;
    }

    update_current_working_dir();
    return 0;
}

bool run_command()
{
    // initialize status variable
    int status;

    char *command = strtok(u_input, " ");
    if (command == NULL)
    {
        printf("ERROR: No command specified\n");
        return true;
    }

    char *args[30];
    args[0] = command;
    int i = 1;
    while (true)
    {
        char *token = strtok(NULL, " ");
        if (token == NULL)
        {
            args[i] = NULL;
            break;
        }
        args[i] = token;
        i++;
    }

    int pid = fork();
    if (pid == 0)
    {
        execvp(command, args);
        fprintf(stderr, "ERROR: Command not found `%s`\n", command);
        exit(255);
    }
    else if (pid > 0)
    {
        wait(&status);

        if (WIFEXITED(status))
        {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        }
    }
    else
    {
        perror("fork");
        exit(1);
    }

    return false;
}

bool mygrep()
{
    char *discard = strtok(u_input, " ");
    char *pattern = strtok(NULL, " ");
    if (pattern == NULL)
    {
        fprintf(stderr, "ERROR: No pattern specified\n");
        return true;
    }
    printf("%s\n", pattern);
    char *path = strtok(NULL, " ");
    if (path == NULL)
    {
        fprintf(stderr, "ERROR: No path specified\n");
        return true;
    }
    printf("%s\n", path);

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        perror("ERROR");
        return true;
    }

    int num_of_lines = 1;
    char ch;
    do
    {
        ch = fgetc(fp);

        // !FOR DEBUGING!
        // printf("%c", ch);

        if (ch == '\n')
        {
            num_of_lines++;
        }

    } while (ch != EOF);
    // !FOR DEBUGING!
    // printf("\n");
    // printf("%d\n", num_of_lines);
    // /\ this is 0 based

    fseek(fp, 0, SEEK_SET);
    char lines[num_of_lines][MAX_LINE_LENGTH];

    for (int i = 0; i < num_of_lines; i++)
    {
        memset(lines[i], '\0', MAX_LINE_LENGTH);
    }
    int i = 0;

    do
    {
        ch = fgetc(fp);

        // !FOR DEBUGING!
        printf("%c", ch);

        if (ch == '\n' || ch == EOF)
        {
            strcat(lines[i], "\0");
            i++;
        }
        else
        {
            strncat(lines[i], &ch, 1);
        }
    } while (ch != EOF);
    // !FOR DEBUGING!
    // printf("\n");
    // for(int i = 0; i < num_of_lines; i++)
    // {
    //         printf("~%s~", lines[i]);
    //         printf("\n");
    // }
    // printf("\n");

    pattern_count = 0;
    pthread_t threads[num_of_lines];
    thread_data thread_data_array[num_of_lines];

    printf("\n");
    for (int i = 0; i < num_of_lines; i++)
    {
        thread_data_array[i].line = lines[i];
        thread_data_array[i].pattern = pattern;
        pthread_create(&threads[i], NULL, &find_match, (void *)&thread_data_array[i]);
    }

    for (int i = 0; i < num_of_lines; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("\n");
    printf("[%d matches found]\n", pattern_count);

    fclose(fp);
    return false;
}

void init()
{
    // She sells CShells
    update_current_user();
    update_current_working_dir();
    log_file = fopen(".myshlog", "w");
    if (log_file == NULL)
    {
        printf("Error opening log file!\n");
        exit(1);
    }

    strcat(home_dir, current_user);
    chdir(home_dir);
    update_current_working_dir();
    pthread_mutex_init(&pattern_count_mutex, NULL);
}

int main()
{

    init();
    while (true)
    {

        print_nav();
        get_u_input();
        log_action();

        if (strcmp(u_input, "exit") == 0)
        {
            break;
        }
        else if (strncmp(u_input, "cd ", 3) == 0)
        {
            strtok(u_input, " ");

            char *path = strtok(NULL, " ");
            if (path == NULL)
            {
                printf("ERROR: No path specified\n");
                continue;
            }
            cd(path);
        }
        else if (strncmp(u_input, "mygrep", 6) == 0)
        {
            bool failed = mygrep(u_input);
            if (failed)
            {
                continue;
            }
        }
        else
        {
            bool failed = run_command(u_input);
            if (failed)
            {
                continue;
            }
        }
    }

    pthread_mutex_destroy(&pattern_count_mutex);
    fclose(log_file);

    return 0;
}