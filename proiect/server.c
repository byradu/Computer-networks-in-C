/* servTcpConc.c - Exemplu de server TCP concurent
    Asteapta un nume de la clienti; intoarce clientului sirul
    SURSA: Georgiana Calancea - laborator 7
    */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void getDate(char **data);
const char *getFileContent(char *name);
void makeHeader(char info[7][60], char **msgrasp);

int main()
{
	struct sockaddr_in server; // structura folosita de server
	struct sockaddr_in from;
	char msg[1000];
	int sd; //descriptorul de socket

	/* crearea unui socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("[server]Eroare la socket().\n");
		return errno;
	}

	/* pregatirea structurilor de date */
	bzero(&server, sizeof(server));
	bzero(&from, sizeof(from));

	/* umplem structura folosita de server */
	/* stabilirea familiei de socket-uri */
	server.sin_family = AF_INET;
	/* acceptam orice adresa */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	/* utilizam un port utilizator */
	server.sin_port = htons(PORT);

	/* atasam socketul */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("[server]Eroare la bind().\n");
		return errno;
	}

	/* punem serverul sa asculte daca vin clienti sa se conecteze */
	if (listen(sd, 1) == -1)
	{
		perror("[server]Eroare la listen().\n");
		return errno;
	}

	/* servim in mod concurent clientii... */
	while (1)
	{
		int client;
		int length = sizeof(from);

		printf("[server]Asteptam la portul %d...\n", PORT);
		fflush(stdout);

		/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
		client = accept(sd, (struct sockaddr *)&from, &length);
		

		/* eroare la acceptarea conexiunii de la un client */
		if (client < 0)
		{
			perror("[server]Eroare la accept().\n");
			continue;
		}

		int pid;
		if ((pid = fork()) == -1)
		{
			close(client);
			continue;
		}
		else if (pid > 0)
		{
			// parinte
			close(client);
			while (waitpid(-1, NULL, WNOHANG))
				;
			continue;
		}
		else if (pid == 0)
		{
			// copil
			close(sd);

			/* s-a realizat conexiunea, se astepta mesajul */
			bzero(msg, 1000);
			printf("[server]Asteptam mesajul...\n");
			fflush(stdout);

			/* citirea mesajului */
			if (read(client, msg, 1000) <= 0)
			{
				perror("[server]Eroare la read() de la client.\n");
				close(client); /* inchidem conexiunea cu clientul */
				continue;	   /* continuam sa ascultam */
			}

			printf("[server]Mesajul a fost receptionat:\n%s\n", msg);

			char *msgrasp = (char *)malloc(512);
			char response[40] = "HTTP/1.1 200 OK";
			char server[10] = "Server: C";
			char conType[60] = "Content-Type: text/";
			char connection[40] = "Connection: Closed";
			char expires[40] ="Expires: 0";
			char cacheControl[41] = "Cache-Control: no-store, must-revalidate";
			
			
			char info[7][60];
			strcpy(info[0], response);
			strcpy(info[2], server);
			strcpy(info[4], connection);
			strcpy(info[5], cacheControl);
			strcpy(info[6], expires);
			

			//split pentru a verifica existenta fisierului
			char *pch = strtok(msg, " ");
			pch = strtok(NULL, " ");
			struct stat st;

			if (strcmp(pch, "/") != 0) //verificam ce se cere
			{
				if (stat(pch + 1, &st) == 0) //verificarea existentei fisierul respectiv
				{
					if (strchr(pch, '.') != NULL)
					{
						char *fileExtension = strtok(pch, ".");
						fileExtension = strtok(NULL, "."); //split pentru a verifica formatul si a completa header ul pentru respuns
						if (strcmp(fileExtension, "html") == 0)
						{
							strcat(conType, "html; charset=utf-8");
							strcpy(info[3], conType);
						}
						else if (strcmp(fileExtension, "txt") == 0)
						{
							strcat(conType, "plain; charset=utf-8");
							strcpy(info[3], conType);
						}
						else
						{
							strcat(conType, "html; charset=utf-8");
							strcpy(response, "HTTP/1.1 422 Unprocessable Entity");
							strcpy(info[0], response);
							strcpy(info[3], conType);
						}
					}
					else
					{
						
						strcat(conType, "html; charset=utf-8");
						strcpy(response, "HTTP/1.1 422 Unprocessable Entity");
						
						strcpy(info[0], response);
						strcpy(info[3], conType);
						
					}

					char *date = (char *)malloc(50);
					getDate(&date);
					strcpy(info[1], date);

					makeHeader(info, &msgrasp);
					msgrasp = (char *)realloc(msgrasp, strlen(msgrasp) + st.st_size + 1);

					if (strstr(msgrasp, "html") != NULL)
						strcat(pch, ".html");
					else if (strstr(msgrasp, "plain") != NULL)
						strcat(pch, ".txt");
					if (strstr(msgrasp, "422") != NULL)
						strcat(msgrasp, getFileContent("error.html"));
					else
						strcat(msgrasp, getFileContent(pch + 1));
				}
				else
				{
					strcat(conType, "html; charset=utf-8");
					strcpy(response, "HTTP/1.1 404 Not found");
					strcpy(info[0], response);
					strcpy(info[3], conType);
					char *date = (char *)malloc(50);
					getDate(&date);
					strcpy(info[1], date);

					makeHeader(info, &msgrasp);

					strcat(msgrasp, "File not found");
				}
			}
			else
			{
				strcat(conType, "html; charset=utf-8");
				strcpy(info[3], conType);
				char *date = (char *)malloc(50);
				getDate(&date);
				strcpy(info[1], date);

				makeHeader(info, &msgrasp);

				stat("index.html", &st);
				msgrasp = (char *)realloc(msgrasp, strlen(msgrasp) + st.st_size + 1);
				strcat(msgrasp, getFileContent("index.html"));
			}

			printf("[server]Trimitem mesajul inapoi:\n%s\n", msgrasp);

			/* returnam mesajul clientului */
			if (write(client, msgrasp, strlen(msgrasp)) <= 0)
			{
				perror("[server]Eroare la write() catre client.\n");
				continue; /* continuam sa ascultam */
			}
			else
				printf("[server]Mesajul a fost trasmis cu succes.\n");

			/* am terminat cu acest client, inchidem conexiunea */
			close(client);
			exit(0);
		}

	} /* while */
} /* main */

void getDate(char **data)
{
	char currentDate[50];
	struct tm *ts;
	size_t last;
	time_t timestamp = time(NULL);
	ts = localtime(&timestamp);
	last = strftime(currentDate, 50, "Date: %a, %F %X", ts);
	currentDate[last] = '\0';
	strcpy(*data, currentDate);
}
const char *getFileContent(char *name)
{
	struct stat f;
	stat(name, &f);
	FILE *fp = fopen(name, "r");
	char ch, *buff = (char *)malloc(f.st_size + 1);
	int i = 0;
	//citim fisierul
	while ((ch = fgetc(fp)) != EOF)
	{
		buff[i] = ch;
		i++;
	}
	buff[i] = '\0';
	return buff;
}
void makeHeader(char info[7][60], char **msgrasp)
{
	for (int i = 0; i < 7; i++)
	{
		strcat(*msgrasp, info[i]);
		if (i == 6)
			strcat(*msgrasp, "\r\n\r\n");
		else
			strcat(*msgrasp, "\n");
	}
}