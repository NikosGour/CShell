#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdarg.h>
#include <pthread.h>

#define MAX_LINE_LENGTH 1024 * 2

#pragma region "Global Variables"
char current_working_directory[MAX_LINE_LENGTH];
char current_user[MAX_LINE_LENGTH];
char home_dir[MAX_LINE_LENGTH] = "/home/";
int pattern_count = 0;
pthread_mutex_t pattern_count_mutex;

#pragma endregion

#pragma region get_current user / directory
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
#pragma endregion

typedef struct
{
    char* line;
    char* pattern;

} thread_data;

void get_u_input(char *u_input, size_t size)
{
    // fill u_input with '\0'
    memset(u_input, '\0', size);

    // read user input
    fgets(u_input, size , stdin);

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

void init()
{
    // She sells CShells
    update_current_user();
    strcat(home_dir, current_user);
    chdir(home_dir);
    update_current_working_dir();
    pthread_mutex_init(&pattern_count_mutex, NULL); 
}

void* find_match(void *arg)
{
    thread_data *data = (thread_data*)arg;

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
}


int main(int argc, char const *argv[])
{
    init();

    while (true)
    {

        print_nav();
        char u_input[MAX_LINE_LENGTH];
        get_u_input(u_input, sizeof(u_input));
        if (strcmp(u_input, "exit") == 0)
        {
            break;
        }
        else if (strncmp(u_input, "cd ", 3) == 0)
        {
            char path[MAX_LINE_LENGTH];
            char *token = strtok(u_input, " ");
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("ERROR: No path specified\n");
                continue;
            }
            strcpy(path, token);
            cd(path);
        }
        else if (strncmp(u_input, "mygrep", 6) == 0)
        {
            strtok(u_input, " ");
            char *pattern = strtok(NULL, " ");
            if (pattern == NULL)
            {
                fprintf(stderr, "ERROR: No pattern specified\n");
                continue;
            }
            // printf("%s\n", pattern);
            char *path = strtok(NULL, " ");
            if (path == NULL)
            {
                fprintf(stderr, "ERROR: No path specified\n");
                continue;
            }
            // printf("%s\n", path);

            FILE *fp = fopen(path, "r");
            if (fp == NULL)
            {
                fprintf(stderr, "ERROR: No file or directory named `%s`\n", path);
                continue;
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
              
            }while(ch != EOF);
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
                // printf("%c", ch);

                if (ch == '\n' || ch == EOF)
                {
                    strcat(lines[i], "\0");
                    i++;
                }
                else
                {
                    strcat(lines[i], &ch);
                }
            }while(ch != EOF);
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
            for(int i = 0; i < num_of_lines; i++)
            {
                thread_data_array[i].line = lines[i];
                thread_data_array[i].pattern = pattern;
                pthread_create(&threads[i], NULL, &find_match, ( void* ) &thread_data_array[i]);
            }

            for(int i = 0; i < num_of_lines; i++)
            {
                pthread_join(threads[i], NULL);
            }

            printf("\n");
            printf("[%d matches found]\n", pattern_count);

            fclose(fp);
        }
        else
        {
            // initialize status variable
            int status;

            char *command = strtok(u_input, " ");
            if (command == NULL)
            {
                printf("ERROR: No command specified\n");
                continue;
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
                fprintf(stderr, "ERROR: Command not found `%s`\n" , command);
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
        }
    }

    pthread_mutex_destroy(&pattern_count_mutex); 
    return 0;
}