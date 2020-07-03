/*  Tot ce este comentariu reprezinta: 
    -debbuging
    -incercari alternative/esuate   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/sysmacros.h>
#include <time.h>
//#define _GNU_SOURCE

void manual_utilizare();
void schimb_verde();
void schimb_alb();
bool raspuns_login();
void Comenzi();
void myfind(char *fisier);
void mystat(char *fisier);
void porneste(char *fil);

int main(int argc, char *argv[])
{
    
    bool rasp;
    manual_utilizare();
    sleep(3);
    rasp = raspuns_login();
    if (rasp == false)
    {
        printf("Utilizator neinregistrat. Programul se va inchide.\n");
        return 0;
    }
    Comenzi();
    return 0;
}

void manual_utilizare()
{

    printf("\n                Bun venit in myShell ! \n\n");
    printf("Doriti sa vedeti manualul de utilizare? Raspunsul va fi [y/n  1 caracter] : ");
    char raspuns;
Din_nou:
    scanf(" %c", &raspuns);
    if (raspuns == 'y')
    {

        FILE *man = fopen("Manual Utilizare.txt", "r");

        if (man == NULL)
        {
            perror("Eroare la deschiderea manualului de utilizare. Probabil lipsa lui din directorul curent.");
            exit(1);
        }

        char caracter;
        printf("\n\n\n");
        schimb_verde();

        while ((caracter = fgetc(man)) != EOF)
        {
            printf("%c", caracter);
        }

        schimb_alb();
        printf("\n\n\n");
    }
    else if (raspuns != 'n')
    {

        printf("\nRaspunsul introdus este gresit. Introduceti din nou raspunsul:  ");
        goto Din_nou;
    }
}

void schimb_verde()
{
    printf("\033[0;32m");
}

void schimb_alb()
{
    printf("\033[0m");
}

bool raspuns_login()
{
    if (-1 == mkfifo("login", 0600))
    {
        if (errno != EEXIST) // errno=17 for "File already exists"
        {
            perror("Eroare la crearea canalului 'login'. Cauza erorii");
            exit(1);
        }
    }
    int log_in, start, porneste_procesul_login = 1;
    bool raspuns;
    start = open("login", O_WRONLY);
    log_in = open("login2", O_RDONLY);
    if (write(start, &porneste_procesul_login, 1) == -1)
    {
        perror("Programul a esuat in a porni procesul de login.");
        exit(20);
    }
    close(start);
    while (1)
    {
        if (read(log_in, &raspuns, sizeof(bool)) == -1)
        {
            perror("Eroare la citirea raspunsului din canalul extern.");
            exit(21);
        }

        break;
    }
    return raspuns;
}

void Comenzi()
{

    while (1)
    {
        sleep(1);
        char in[20], comanda[10];
        printf("\033[0;32mIntroduceti comanda:\033[0m ");
    Reintrodu:
        scanf(" %s", in);
        int i = 0;
        while (in[i] != ' ' && in[i] != '\0')
        {
            comanda[i] = in[i];
            i++;
        }
        comanda[i] = '\0';
        if (strcmp(comanda, "quit") == 0)
        {
            printf("Programul se va termina. O zi buna!\n");
            break;
        }
        else if (strcmp(comanda, "myfind") == 0)
        {
        }
        else if (strcmp(comanda, "mystat") == 0)
        {
            char nume_fisier[100];
            printf("\033[0;32mIntroduceti fisierul dorit:\033[0m ");
            scanf(" %s", nume_fisier);
            mystat(nume_fisier);
            //porneste(nume_fisier);
        }
        else
        {
            printf("Comanda introdusa gresit.\n\033[0;32mReintroduceti comanda:\033[0m ");
            goto Reintrodu;
        }
    }
}

void myfind(char *fisier)
{
}

void mystat(char *fisier) //se va mai adauga inca un int pentru a utiliza functia si in myfind si in mystat
{
    struct stat st;
    struct passwd *pwd;          /* aceasta structura o vom folosi pentru a afla username-ul asociat unui UID */
    char perm[10] = "---------"; /* aici vom construi forma simbolica pentru permisiunile fisierului */
    int result = 0;

    if (0 != stat(fisier, &st))
    {
        fprintf(stderr, "Eroare la stat pentru %s .\t", fisier);
        perror("Cauza este");
        return; /* aici nu terminam executia cu exit(), ci revenim in apelant pentru a continua cu ceea ce a mai ramas de procesat */
    }
    printf("Tip fisier: ");
    switch (st.st_mode & S_IFMT)
    {
    case S_IFDIR:
        printf("Director\n");
        result = 1;
        break;
    case S_IFREG:
        printf("Fisier obisnuit\n");
        break;
    case S_IFLNK:
        printf("Link\n");
        break;
    case S_IFIFO:
        printf("FIFO\n");
        break;
    case S_IFSOCK:
        printf("Socket\n");
        break;
    case S_IFBLK:
        printf("Block device\n");
        break;
    case S_IFCHR:
        printf("Character device\n");
        break;
    default:
        printf("Unknown file type");
    }
    printf("\tDimensiunea acestuia: %lld octeti\n", (long long)st.st_size);
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

    printf("\tPermisiunile acestuia: %s\n", perm);
    printf("\tUltimul status:       %s", ctime(&st.st_ctime));
    printf("\tUltima accesare:       %s", ctime(&st.st_atime));
    printf("\tUltima modificare:   %s", ctime(&st.st_mtime));

    if (NULL != (pwd = getpwuid(st.st_uid)))
        printf("\tProprietarul acestuia: %s (cu UID-ul: %ld)\n", pwd->pw_name, (long)st.st_uid);
    else
        printf("\tProprietarul acestuia are UID-ul: %ld\n", (long)st.st_uid);

    return;
}

void porneste(char *fil)
{
    pid_t pid;
    int d[2], cop[2];
    if (pipe(d) == -1)
    {
        perror("Eroare la crearea pipe-ului din mystat");
        exit(30);
    }
    cop[0] = dup(d[0]);
    cop[1] = dup(d[1]);
    if (-1 == (pid = fork()))
    {
        perror("Eroare la crearea fork-ului din mystat");
        exit(31);
    }
    if (pid == 0) //copil
    {
        close(d[1]);
        close(d[0]);
        mystat(fil);
    }
    else
    {
        wait(NULL);
        close(cop[1]);
        close(cop[0]);
       
    }
}