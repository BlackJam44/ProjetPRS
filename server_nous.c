#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>

#define BUFSIZE 1500
#define SEQSIZE 6
#define CHASIZE BUFSIZE-SEQSIZE
#define NB_Port_Max 9999
#define NB_Port_Definit 1025

typedef struct {
  	int Descripteur;
  	socklen_t Client_Length;
  	struct sockaddr_in Server_Address;
 	  struct sockaddr_in Client_Address;
  	char Buffer[BUFSIZE];
  	int Opt_Val;
  	int Nb_Bytes;
} ELEM, *ELEMP;

int Port_Temp;

/* message d'erreur */
int Error(char *message) {
	fprintf(stderr,"Error: %s\n",message);
	exit(-1);
}

/* interruption */
void signalHandler() {
	close(3);
	Error("Programme Interrompu..");
}

/* génération d'un port aléatoire */
int Port_Aleatoire() {
	srand(time(NULL));
	return (rand() % (NB_Port_Max-NB_Port_Definit)) + NB_Port_Definit;
}

/* création socket de connexion */
int Parametre_Principal_Server(ELEMP a, int Port) {
  	a->Descripteur = socket(AF_INET,SOCK_DGRAM,0);
  	if (a->Descripteur < 0) {
  		printf("Probleme Ouverture de la Socket Principale\n");
  		return -1;
  	}
  	a->Opt_Val = 1;
  	setsockopt(a->Descripteur,SOL_SOCKET,SO_REUSEADDR,(const void *)&(a->Opt_Val),sizeof(int));
  	bzero((char*) &(a->Server_Address),sizeof(a->Server_Address));
  	a->Server_Address.sin_family = AF_INET;
  	a->Server_Address.sin_addr.s_addr = htonl(INADDR_ANY);
 	a->Server_Address.sin_port = htons(Port);
	a->Client_Length = sizeof(a->Client_Address);
  	if (bind(a->Descripteur, (struct sockaddr *) &(a->Server_Address),sizeof(a->Server_Address)) < 0) {
		close(a->Descripteur);
		printf("Probleme Bind Principale\n");
		return -1;
	}
	return 0;
}

/* création de la socket de communication */
int Parametre_Principal_Fils(ELEMP b)
{
	b->Descripteur = socket(AF_INET,SOCK_DGRAM,0);
	if (b->Descripteur < 0) {
		printf("Probleme Ouverture de la Socket Fils\n");
		return -1;
	}
  	b->Opt_Val = 1;
  	setsockopt(b->Descripteur,SOL_SOCKET,SO_REUSEADDR,(const void *)&(b->Opt_Val),sizeof(int));
  	bzero((char *) &(b->Server_Address),sizeof(b->Server_Address));
  	b->Server_Address.sin_family = AF_INET;
  	b->Server_Address.sin_addr.s_addr = htonl(INADDR_ANY);
	Port_Temp = Port_Aleatoire();
	b->Server_Address.sin_port = htons(Port_Temp);
	b->Client_Length = sizeof(b->Client_Address);
  	if (bind(b->Descripteur, (struct sockaddr *) &(b->Server_Address),sizeof(b->Server_Address)) < 0) {
		close(b->Descripteur);
		printf("Probleme Bind Fils\n");
		return -1;
	}
	getsockname(b->Descripteur, &(b->Server_Address), &(b->Client_Length));
	return 0;
}

/* réception de paquet */
int Reception_Message(ELEMP a) {
	bzero(a->Buffer,BUFSIZE);
    	a->Nb_Bytes = recvfrom(a->Descripteur,a->Buffer,BUFSIZE,0,(struct sockaddr *) &(a->Client_Address),&(a->Client_Length));
    	return a->Nb_Bytes;
}

/* envoi de paquet */
int Envoi_Message(ELEMP a, int Size) {
	a->Nb_Bytes = sendto(a->Descripteur,a->Buffer,Size,0,(struct sockaddr *) &(a->Client_Address),a->Client_Length);
	return a->Nb_Bytes;
}

/* connexion avec le client */
int Connexion_Nouveau_Client(ELEMP a, int Port) {
	bzero(a->Buffer,BUFSIZE);
	snprintf(a->Buffer, BUFSIZE, "SYN-ACK%d", Port);
	if (Envoi_Message(a,11) < 0) {
		printf("Envoi Message Erreur\n");
		return -1;
	}
	printf("Sending %s...\n",a->Buffer);
	if (Reception_Message(a) < 0) {
		printf("Reception Message Erreur\n");
		return -1;
	}
	printf("Received message: %s\n",a->Buffer);
	if (strncmp(a->Buffer,"ACK",BUFSIZE) != 0) {
		printf("Probleme Reception ACK\n");
		return -1;
	}
	else printf("Connexion etablie sur le port %d\n",Port);
	return 0;
}

/* écriture numéro de séquence */
int Ecrire_Sequence(ELEMP b, int i, char* a) {
	if (i < 10) {
		snprintf(b->Buffer, BUFSIZE, "00000%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	else if (i < 100) {
		snprintf(b->Buffer, BUFSIZE, "0000%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	else if (i < 1000) {
		snprintf(b->Buffer, BUFSIZE, "000%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	else if (i < 10000) {
		snprintf(b->Buffer, BUFSIZE, "00%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	else if (i < 100000) {
		snprintf(b->Buffer, BUFSIZE, "0%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	else {
		snprintf(b->Buffer, BUFSIZE, "%d",i);
		memcpy(b->Buffer+SEQSIZE,a,CHASIZE);
	}
	return 0;
}

/* envoi du fichier */
int Envoyer_Fichier(ELEMP b, FILE* Fichier) {
	fd_set rdfs;
	struct timeval Temps;
	Temps.tv_sec = 0;
	Temps.tv_usec = 2000;

	char Chaine[CHASIZE], Seq[SEQSIZE];
	int NumSeq = 1, Bytes_Read, Total_Bytes = 0, Boucle = 0;
	while ((Bytes_Read = fread(Chaine, 1, CHASIZE-1, Fichier)) > 0) {
		do {
			bzero(b->Buffer,BUFSIZE);
			Ecrire_Sequence(b,NumSeq,Chaine);
			do {
					if (Envoi_Message(b,(Bytes_Read+SEQSIZE)) < 0) {
						printf("Envoi Message Erreur Num Seq %d\n",NumSeq);
						return -1;
					}
					printf("Sent %d Bytes of Data Seq %06d...\n",b->Nb_Bytes,NumSeq);
					FD_ZERO(&rdfs);
					FD_SET(b->Descripteur,&rdfs);
					Boucle++;
					if (Boucle > 1) {
						Temps.tv_sec = 0;
						Temps.tv_usec = 1000;
					}
			} while (select(b->Descripteur + 1, &rdfs, NULL, NULL, &Temps) == 0);
			Boucle = 0;
			if (Reception_Message(b) < 0) {
				printf("Reception Message Erreur Num Seq %d\n",NumSeq);
				return -1;
			}
			printf("Received message: %s\n",b->Buffer);
			sscanf(b->Buffer,"ACK%s",Seq);
		} while (atol(Seq) != NumSeq);
		NumSeq++;
		Total_Bytes += Bytes_Read;
		bzero(Chaine,CHASIZE);
	}
	snprintf(b->Buffer, BUFSIZE, "FIN");
	printf("Sending %s...\n",b->Buffer);
	if (Envoi_Message(b,4) < 0) {
		printf("Envoi Message Erreur Fin\n");
		return -1;
	}
	printf("Total bytes sent %d\n",Total_Bytes);
	return 0;
}

int main(int argc, char *argv[]) {
	ELEM a, b;
  if (argc != 2)
    Error("Usage: ./server1 <Numero_Port>");
	if (Parametre_Principal_Server(&a,atoi(argv[1])) != 0)
		Error("L initialisation des parametres du serveur se sont deroulees avec des erreurs !");
	while (1)
	{
		if (Reception_Message(&a) < 0)
			Error("Reception Message Erreur SYN");
		if (strncmp(a.Buffer,"SYN",BUFSIZE) == 0)
		{
			printf("Received message: %s\n",a.Buffer);
			if (Parametre_Principal_Fils(&b) != 0)
				Error("L'initialisation des parametres du fils se sont deroulees avec des erreurs !");
			if (Connexion_Nouveau_Client(&a,Port_Temp) != 0)
				Error("Probleme de connexion d un nouveau client !");
			if (Reception_Message(&b) < 0)
				Error("Reception Message Erreur Fichier");
			printf("Received message: %s\n",b.Buffer);
			FILE* Fichier;
			if ((Fichier = fopen(b.Buffer,"rb")) == NULL)
				Error("Opening Failed File !\n");
			if (Envoyer_Fichier(&b,Fichier) != 0)
				Error("Probleme Envoie Fichier");
			else printf("Fichier Bien envoyé !\n");
			fclose(Fichier);
			close(b.Descripteur);
			exit(0);
		}
	}
	close(a.Descripteur);
	return 0;
}
