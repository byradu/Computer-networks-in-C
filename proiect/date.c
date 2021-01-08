#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

int main()
{
    struct stat st;
    char *s="/a.out";
    
    stat(s+1,&st);
    if((st.st_mode&S_IFMT)==S_IFREG)
        printf("regular file\n");
    else 
        printf("unkown\n");
}