#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>

#pragma region "Global Variables"
char current_working_directory[1024];
char current_user[1024];
#pragma endregion

#pragma region get_current user/directory
void update_current_user()
{
    struct passwd *pw = getpwuid(getuid());
    char *homedir = pw->pw_name;
    strcpy(current_user, homedir);
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

void print_nav()
{
    printf("%s:%s>\n",current_user, current_working_directory);
}
int main(int argc, char const *argv[])
{
    update_current_user();
    update_current_working_dir();
    print_nav();
    return 0;
}
