#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char const *argv[])
{
    char *buf;
    buf=(char *)malloc(10*sizeof(char));
    buf=getlogin(); 
  
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s:%s>\n",buf, cwd);
    return 0;
}
