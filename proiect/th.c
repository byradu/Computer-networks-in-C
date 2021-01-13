/*
			VA ROG A SE CITI CU ATENTIE!
	
	URMATOAREA EROARE 	:[server]Eroare la read() de la client.
						: Success
						
	ESTE CAUZATA DE FAPTUL CA UNELE BROWSERE CAND DAI FAC HOOVER PE LINK REALIZEAZA O CONEXIUNE CU SERVER-UL 
	PENTRU A-SI SPORI PERFORMANTA (ACT OBSERVAT LA MOZILLA ).
	Uneori si dupa inchiderea browserului apar 2 astfel de erori.
	(unele browsere cand tastezi link-ul realizeaza deja o conexiune sau 2 cu server-ul, dar nu realizeaza cereri.)
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
#include <pthread.h>

#define _REENTRANT1

#define PORT 2021
extern int errno;

void getDate(char **data);						   //preluare data
const char *getFileContent(char *name);			   //citire fisier
void makeHeader(char info[7][60], char **msgrasp); //creare header pentru raspuns
void *takeCareOfClient(void *arg);

int main()
{
	struct sockaddr_in server, from;
	int sd; //socket descriptor

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Eroare la socket().\n");
		return errno;
	}

	bzero(&server, sizeof(server));
	bzero(&from, sizeof(from));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;// vrem sa atasam adresa noastra locala
	server.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("Eroare la bind().\n");
		close(sd);
		return errno;
	}

	if (listen(sd, 25) == -1)
	{
		perror("[server]Eroare la listen().\n");
		return errno;
	}

	while (1)
	{
		int client, length = sizeof(from);

		printf("Asteptam clienti...\n");
		fflush(stdout);

		client = accept(sd, (struct sockaddr *)&from, &length);

		if (client < 0)
		{
			perror("Eroare la accept().\n");
			continue;
		}
		pthread_t a_thread;
		if (pthread_create(&a_thread, NULL, takeCareOfClient, &client) != 0)
		{
			perror("Eroare la pthread_create().\n");
			close(client);
			continue;
		}
	}
}

void *takeCareOfClient(void *arg)
{
	int client = *((int *)arg);
	
	pthread_detach(pthread_self());// vrem ca thread-ul sa fie independent de restul procesului

	char msg[1024];
	bzero(msg, 1024);
	fflush(stdout);

	/* citirea mesajului */
	if (read(client, msg, 1000) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		close(client); /* inchidem conexiunea cu clientul */
		pthread_exit(NULL);
	}

	printf("[server]Mesajul a fost receptionat:\n%s\n", msg);

	//pregatim raspunsul
	char *msgrasp = (char *)malloc(512);
	char response[40] = "HTTP/1.1 200 OK";
	char server[10] = "Server: C";
	char conType[60] = "Content-Type: text/";
	char connection[40] = "Connection: Closed";
	char expires[40] = "Expires: 0";
	char cacheControl[41] = "Cache-Control: no-store, must-revalidate";

	char info[7][60];//o matrice pentru formarea raspunsului final
	strcpy(info[0], response);
	strcpy(info[2], server);
	strcpy(info[4], connection);
	strcpy(info[5], cacheControl);
	strcpy(info[6], expires);

	//De aici se va incepe prelucrarea raspunsului

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
				fileExtension = strtok(NULL, "."); //split pentru a verifica formatul si a completa header-ul pentru respuns
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

			if (strstr(msgrasp, "html") != NULL)//avem nevoie de numele fisierului si il formam din nou
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

			strcat(msgrasp,getFileContent("404.html"));
			
			//strcat(msgrasp, "File not found");
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
		pthread_exit(NULL);
	}
	else
		printf("[server]Mesajul a fost trasmis cu succes.\n");

	/* am terminat cu acest client, inchidem conexiunea */
	close(client);
	return (NULL);
}

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