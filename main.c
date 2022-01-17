#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#include<stdarg.h>

#pragma region "Global Variables"
char current_working_directory[1024];
char current_user[1024];
char home_dir[1024] = "/home/";

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

void print(const char* format, ...)
{
    char str[1024];
    va_list args;
    va_start(args, format);
    vsprintf(str,format, args);
    va_end(args);

    write(1, str, strlen(str));
}
void print_nav()
{
    update_current_user();
    update_current_working_dir();
    print("%s:%s>",current_user, current_working_directory);
    
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
        perror("cd error");
        return -1;
    }
    return 0;
}

void init()
{
    update_current_user();
    strcat(home_dir, current_user);
    chdir(home_dir);
    update_current_working_dir();
}


int main(int argc, char const *argv[])
{
    init();
    print_nav();
    char dir[1024];
    // fgets(dir, sizeof(dir), stdin);
    read(0, dir, sizeof(dir));
    dir[strlen(dir)-1] = '\0';
    printf("!%s!\n", dir);
    cd(dir);
    update_current_working_dir();
    print_nav();

    return 0;
}
