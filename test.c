#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(){

    int fd[2];

    pid_t pid;
    pipe(fd);

    pid = fork();

    if(pid>0)
    {
        while(1){
            char * command = (char*)malloc(55);

            printf("Command: ");
            fgets(command,55,stdin);

            int mode=1;

            write(fd[1],&mode,sizeof(int));//ce canal utilizam....




            wait(NULL);

            read(fd[0],&command,55);
            printf("Return: %s",command);
        }
        
    }
    else if(pid)
    {   
        char * command = (char*)malloc(55);
        int usedMethod;

        read(fd[0],&usedMethod,sizeof(int));

        if(usedMethod==1){//tot pipe

        read(fd[0],&command,55);

        }
        else if(usedMethod==2){

            read(fd[0],&command,55);//fifo

        }
        else if(usedMethod==3){

            read(fd[0],&command,55);//socketpair

        }
        else if(usedMethod==-1){
            return 0;
        }


        printf("Child command: %s",command);
        
        write(fd[1],&command,55);

    }
    
    return 0;
}