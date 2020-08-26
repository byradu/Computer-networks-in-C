#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <dirent.h>

#define green "\033[0;32m"
#define red "\033[0;31m"
#define blue "\033[1;34m"
#define white "\033[0m"

char *mystat(char *file);
char *myfind(char *path, char *pattern);

int main()
{

    int fd[2];
    pid_t pid;
    int sckt[2];

    if (pipe(fd) == -1)
    {
        perror(red "Pipe error\n");
        exit(1);
    }

    if (mkfifo("test", 0600) == -1)
    {
        perror(red "Creating fifo error\n");
        exit(2);
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sckt) == -1)
    {
        perror(red "Socket error...\n");
        exit(6);
    }

    if ((pid = fork()) < 0)
    {
        perror(red "Fork error\n");
        exit(3);
    }
    else if (pid == 0)
    { //fiu
        while (1)
        {

            char command[55];
            if (read(fd[0], command, 55) == -1)
            {
                perror(red "Reading error child..\n");
                exit(4);
            }

            if (strstr(command, "quit") != NULL)
            {
                break;
            }
            else if (strstr(command, "login : ") != NULL)
            { //pipe
                int gasit = 0;
                FILE *file = fopen("users.txt", "r");
                char username[35];
                while (fgets(username, 35, file))
                {
                    username[strlen(username) - 1] = NULL;
                    if (strstr(command, username) != NULL)
                    {
                        gasit = 1;
                    }
                }
                int wr = open("test", O_WRONLY);
                char *raspuns = (char *)malloc(55);

                if (gasit == 1)
                    strcpy(raspuns, green "Success!");
                else
                    strcpy(raspuns, red "Wrong username");

                if (write(wr, raspuns, 55) == -1)
                {
                    perror(red "Writing error child..\n");
                    exit(5);
                }
                close(wr);
            }
            else if (strstr(command, "mystat ") != NULL)
            {
                char *rasp = mystat(command + strlen("mystat "));

                if (write(sckt[1], rasp, 500) == -1)
                {
                    perror(red "Writing error child..\n");
                    exit(9);
                }
            }
            else if (strstr(command, "myfind ") != NULL)
            {

                char *path = (char *)malloc(256);
                char *pattern = (char *)malloc(256);
                char *answ = (char *)malloc(1000);
                int errnum;

                if (strchr(command, '/') != NULL)
                { //daca exista / inseamna ca avem un director si/sau un pattern/alt director
                    int i = 0, lastChar = strlen(command + 7) - 1;
                    while (strlen(command + 7) - i - 1 != 0)
                    {
                        if (command[7 + lastChar] == '/')
                            break;
                        lastChar--;
                        i++;
                    }
                    char *testDirectory = (char*)malloc(256);
                    strcpy(testDirectory,command+7);
                    DIR*dirz=opendir(testDirectory);
                    /* if(dirz){
                        strcpy(pattern,"*");
                        strcpy(answ,myfind(testDirectory,pattern));
                        if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(39);
                            }
                    } */
                    if (lastChar == strlen(command + 7) - 1)
                    {
                        strncpy(path, command + 7, lastChar);
                        DIR *dir = opendir(path);
                        if (dir)
                        { //daca e un director existent
                            closedir(dir);
                            strcpy(pattern, "*");
                            strcpy(answ, myfind(path, pattern));
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(30);
                            }
                        }
                        else if (ENOENT == errno)
                        { //daca directorul e inexistent
                            strcpy(answ, red "Wrong path");
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(31);
                            }
                        }
                        else
                        {
                            strcpy(answ, red "Something went wrong\n");
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(32);
                            }
                        }
                    }
                    else if (lastChar < strlen(command + 7) - 1)
                    { //ex: myfind /home/radu
                        strcpy(pattern, command + lastChar + 7 + 1);
                        DIR *dir = opendir(pattern);
                        if (dir)
                        {
                            closedir(dir);
                            strcpy(path, command + 7);
                            strcpy(pattern, "*");
                            strcpy(answ, myfind(path, pattern));
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(33);
                            }
                        }
                        else if (ENOENT == errno)
                        { //nu e director e pattern
                            strncpy(path, command + 7, lastChar);
                            //verificam daca path-ul exista
                            DIR *dir2 = opendir(path);
                            if (dir2)
                            {
                                closedir(dir2);
                                strcpy(answ, myfind(path, pattern));
                            }
                            else if (ENOENT == errno)
                            {
                                strcpy(answ, red "Wrong path\n");
                            }
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(34);
                            }
                        }
                        else
                        {
                            strcpy(answ, red "Something went wrong\n");
                            if (write(sckt[1], answ, 1000) == -1)
                            {
                                perror(red "Writing error child..\n");
                                exit(35);
                            }
                        }
                    }
                }
                else
                { //se da direct pattern-ul
                    char cwd[PATH_MAX];
                    if (getcwd(cwd, sizeof(cwd)) != NULL)
                    {
                        strcpy(path, cwd);
                        strcpy(pattern, command + 7);
                        strcpy(answ, myfind(path, pattern));
                        if (write(sckt[1], answ, 1000) == -1)
                        {
                            perror(red "Writing error child..\n");
                            exit(36);
                        }
                    }
                    else
                    { //avem eroare daca nu e director
                        strcpy(answ, strerror(errnum));
                        if (write(sckt[1], answ, 1000) == -1)
                        {
                            perror(red "Writing error child..\n");
                            exit(37);
                        }
                    }
                }
            }
        }
    }
    else if (pid > 0)
    {
        while (1)
        {
            sleep(1);
            char command[55];

            printf(blue "Insert your command: " white);
            fflush(stdout);
            fgets(command, 55, stdin);

            command[strlen(command) - 1] = NULL;

            if (write(fd[1], command, 55) == -1)
            {
                perror(red "Writing form parent error..\n");
                exit(7);
            }

            if (strstr(command, "quit") != NULL)
            {
                printf(green "Have a great day!\n");
                break;
            }

            if (strstr(command, "login : ") != NULL)
            {
                char ans[55];
                int rd = open("test", O_RDONLY);
                if (read(rd, ans, 55) == -1)
                {
                    perror(red "Reading in parent error...\n");
                    exit(8);
                }
                printf("%s\n", ans);

                close(rd);
            }

            if (strstr(command, "mystat ") != NULL)
            {
                char *mystt = (char *)malloc(500);
                if (read(sckt[0], mystt, 500) == -1)
                {
                    perror(red "Reading in parent error...\n");
                    exit(10);
                }
                printf("%s\n", mystt);
            }

            if (strstr(command, "myfind ") != NULL)
            {
                char *answ = (char *)malloc(1000);
                if (read(sckt[0], answ, 1000) == -1)
                {
                    perror(red "Reading error..\n");
                    exit(40);
                }
                printf("%s", answ);
            }
        }
    }
    remove("test");
    return 0;
}

char *mystat(char *file)
{ // Functia este preluata de pe site-ul disciplinei Sisteme de Operarea din anul 1

    char *result = (char *)malloc(500);
    struct stat st;
    struct passwd *pwd;          /* aceasta structura o vom folosi pentru a afla username-ul asociat unui UID */
    char perm[10] = "---------"; /* aici vom construi forma simbolica pentru permisiunile fisierului */

    if (0 != stat(file, &st))
    {

        snprintf(result, 500, "Eroare la stat pentru %s .\tCauza este %s", file, strerror(errno));
        return result; /* aici nu terminam executia cu exit(), ci revenim in apelant pentru a continua cu ceea ce a mai ramas de procesat */
    }
    strcpy(result, "Tip fisier: ");
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
        strcat(result, "Director\n");
        break;
    case S_IFREG:
        strcat(result, "Fisier obisnuit\n");
        break;
    case S_IFLNK:
        strcat(result, "Link\n");
        break;
    case S_IFIFO:
        strcat(result, "FIFO\n");
        break;
    case S_IFSOCK:
        strcat(result, "Socket\n");
        break;
    case S_IFBLK:
        strcat(result, "Block device\n");
        break;
    case S_IFCHR:
        strcat(result, "Character device\n");
        break;
    default:
        strcat(result, "Unknown file type\n");
    }
    char dim[60];
    snprintf(dim, 60, "\tDimensiunea acestuia: %lld octeti\n", (long long)st.st_size); //nu pot cast de la int la char
    strcat(result, dim);
    if (S_IRUSR & st.st_mode)
        perm[0] = 'r';
    if (S_IWUSR & st.st_mode)
        perm[1] = 'w';
    if (S_IXUSR & st.st_mode)
        perm[2] = 'x';
    if (S_IRGRP & st.st_mode)
        perm[3] = 'r';
    if (S_IWGRP & st.st_mode)
        perm[4] = 'w';
    if (S_IXGRP & st.st_mode)
        perm[5] = 'x';
    if (S_IROTH & st.st_mode)
        perm[6] = 'r';
    if (S_IWOTH & st.st_mode)
        perm[7] = 'w';
    if (S_IXOTH & st.st_mode)
        perm[8] = 'x';
    char *stats = (char *)malloc(100);
    snprintf(stats, 100, "\tPermisiunile acestuia: %s\n\tUltimul status:       %s\tUltima accesare:       %s\tUltima modificare:   %s", perm,
             ctime(&st.st_ctime), ctime(&st.st_atime), ctime(&st.st_mtime));
    strcat(result, stats);
    char *prop = (char *)malloc(50);
    if (NULL != (pwd = getpwuid(st.st_uid)))
        snprintf(prop, 50, "\tProprietarul acestuia: %s (cu UID-ul: %ld)\n", pwd->pw_name, (long)st.st_uid);
    else
        snprintf(prop, 50, "\tProprietarul acestuia are UID-ul: %ld", (long)st.st_uid);

    strcat(result, prop);

    return result;
}
char *myfind(char *path, char *pattern)
{ //sursa: www.unix.com/programming/26901-using-find-c.html
    //codul e modificat pentru a returna un string
    char file[50], *answ = (char *)malloc(1000);
    DIR *dirp = opendir(path);
    struct dirent entry;
    struct dirent *dp = &entry;
    while (dp = readdir(dirp))
    {
        if ((fnmatch(pattern, dp->d_name, 0)) == 0)
        {
            strcpy(file, "\0");
            strcat(file, dp->d_name);
            strcat(file, "\n");
            strcat(answ, file);
        }
    }
    closedir(dirp);
    return answ;
}