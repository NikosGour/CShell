#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#include<stdarg.h>

#define MAX_LINE_LENGTH 1024 * 2

#pragma region "Global Variables"
char current_working_directory[MAX_LINE_LENGTH];
char current_user[MAX_LINE_LENGTH];
char home_dir[MAX_LINE_LENGTH] = "/home/";

#pragma endregion

#pragma region get_current user/directory
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
    char* pwd = getcwd(current_working_directory, sizeof(current_working_directory));
    if(pwd == NULL)
    {
        perror("getcwd error");
        exit(1);
    }
    
}
#pragma endregion

/**
 * @brief  Custom printf function using the write system call
 */
void _print(const char* format, ...)
{
    // string to hold the final string to be printed
    char str[MAX_LINE_LENGTH];
    va_list args;
    va_start(args, format);

    // format the string
    vsprintf(str,format, args);
    
    va_end(args);

    // write the string to the console
    write(1, str, strlen(str));
}


void get_u_input( char* u_input , size_t size)
{
    //fill u_input with '\0'
    memset(u_input, '\0', size);

    //read user input
    read(0, u_input, size);

    //remove newline character
    u_input[strlen(u_input) - 1] = '\0';

    //debug
    // printf("!%s!\n", u_input);
}
void print_nav()
{
    // Update current working directory and user variables
    update_current_user();
    update_current_working_dir();

    // Print current working directory and user in bash style
    _print("%s:%s> ",current_user, current_working_directory);
    
    // backup plan:
    // printf("%s:%s>",current_user, current_working_directory);
    // char x[1024];
    // sprintf(x, "%s:%s>",current_user, current_working_directory);
    // write(1, x, strlen(x));
}

int cd(char* path)
{
    if(chdir(path) != 0)
    {
        printf("ERROR: No file or directory named `%s`\n", path);
        return -1;
    }
    
    update_current_working_dir();
    return 0;
}

void init()
{
    //She sells CShells
    update_current_user();
    strcat(home_dir, current_user);
    chdir(home_dir);
    update_current_working_dir();
}


int main(int argc, char const *argv[])
{
    init();
    
    while (true)
    {    
        
        print_nav();
        char u_input[MAX_LINE_LENGTH];
        get_u_input(u_input , sizeof(u_input));
        if (strcmp(u_input, "exit") == 0)
        {
            break;
        }
        else if (strstr(u_input, "cd") != NULL)
        {
            char path[MAX_LINE_LENGTH];
            char* token = strtok(u_input, " ");
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                printf("ERROR: No path specified\n");
                continue;
            }
            strcpy(path, token);            
            cd(path);
        }
        else
        {
            char* command = strtok(u_input, " ");
            if (command == NULL)
            {
                printf("ERROR: Command can't be empty\n");
            }
            else
            {
                printf("ERROR: No such command `%s`\n", command);
            }
        }
    }
    
    return 0;
}
