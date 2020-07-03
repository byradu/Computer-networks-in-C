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

bool login();

int main()
{

    sleep(1);
    if (-1 == mkfifo("login", 0600))
    {
        if (errno != EEXIST) // errno=17 for "File already exists"
        {
            perror("Eroare la crearea canalului 'login'. Cauza erorii");
            exit(1);
        }
    }
    if (-1 == mkfifo("login2", 0600))
    {
        if (errno != EEXIST) // errno=17 for "File already exists"
        {
            perror("Eroare la crearea canalului 'login2'. Cauza erorii");
            exit(2);
        }
    }
    int citeste, scrie, wait;
    bool r;
    citeste = open("login", O_RDONLY);
    scrie = open("login2", O_WRONLY);
    while (1)
    {
        if (read(citeste, &wait, 1) == -1)
        {
            perror("Programul (login) a esuat in a porni procesul de login.");
            exit(23);
        }
        break;
    }
    r = login();
    if (write(scrie, &r, sizeof(bool)) == -1)
    {
        perror("Programul a esuat in a transmite raspunsul de login.");
        exit(20);
    }

    return 0;
}

bool login()
{
    char username[100], check[100], parola[100], check2[100]; //*check=NULL
    printf("Introduceti user-ul: ");
pam:
    scanf("%s", username);
    FILE *users = fopen("Usernames.txt", "r");
    bool verif = false, verif1 = false;
    //size_t lungimea_liniei;
    while (fgets(check, 100, users))
    { //(getline(&check,&lungimea_liniei,users))!=-1
        check[strlen(check) - 2] = '\0';
        if (strcmp(check, username) == 0)
            verif = true;
    }
    if (verif == false)
    {
        printf("\nUtilizatorul introdus nu exista. Introduceti din nou user-ul: ");
        goto pam;
    }
    printf("Introduceti parola: ");
    scanf(" %s", parola);
    FILE *pass = fopen("Passwords.txt", "r");
    while (fgets(check2, 100, pass))
    {
        check2[strlen(check2) - 1] = '\0';
        if (strcmp(check2, parola) == 0)
        {
            verif1 = true;
        }
    }
    printf("\n");
    if (verif == true && verif1 == true)
    {
        return true;
    }
    return false;
}