#include <arpa/inet.h>
#include <errno.h> // codes d'erreurs socket
#include <fcntl.h> // pour ouverture
#include <netinet/in.h>
#include <unistd.h> // pour fermeture
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "functions.h"


// fonctions d'ouverture de connexion
CONNECT* openServer(int socket, struct sockaddr* addr){ // exécuté côté client
	int size = sizeof(struct sockaddr);
	char msg[4];
	char ans[10];
	char* no_port = (char*)malloc(sizeof(char));

	printf("Initializing connection...\n");
	strcpy(msg, "SYN");

	if(sendto(socket, msg, 4, 0, addr, size) == -1){
		perror("Error: SYN\n");
		close(socket);
		exit(-1);
	}

	if(recvfrom(socket, ans, 8, 0, addr, &size) == -1){
		perror("Error: SYN-ACK reception\n");
		close(socket);
		exit(-1);
	}

	if(recvfrom(socket, no_port, 5, 0, addr, &size) == -1){
		perror("Error: port number reception\n");
		close(socket);
		exit(-1);
	}

	if (strcmp(ans,"SYN-ACK") == 0) {
		strcpy(msg, "ACK");
		if(sendto(socket, msg, 4, 0, addr, size) == -1){
			perror("Error: ACK\n");
			close(socket);
			exit(-1);
		}
	}else{
		perror("Error: SYN-ACK message received\n");
		close(socket);
		exit(-1);
	}
	printf("\tConnection initialized.\n");

	// structure retournée
	CONNECT* res = (CONNECT*)malloc(sizeof(CONNECT));
	res->result = 1;
	res->port = (char*)malloc(sizeof(char));
	strcpy(res->port, no_port);

  return res;
}

CONNECT* openClient(int socket, struct sockaddr* addr){ // exécuté côté serveur
	int size = sizeof(struct sockaddr);
	char msg[12];
	char req1[12];
	char req2[12];
	char* no_port = (char*)malloc(sizeof(char));

	printf("Initializing connection...\n");

	if(recvfrom(socket, req1, 12, 0, addr, &size) == -1){
		perror("Error: SYN reception\n");
		close(socket);
		exit(-1);
	}

	printf("Received message: %s \n", req1);
	if (strcmp(req1,"SYN") == 0) {
		strcpy(msg, "SYN-ACK");
		strcpy(no_port, getPort());
		strcat(msg, no_port);

		if(sendto(socket, msg, 12, 0, addr, size) == -1){
			perror("Error: SYN-ACK port number sending\n");
			close(socket);
			exit(-1);
		}
		printf("Sending %s... \n", msg);

		if(recvfrom(socket, req2, 12, 0, addr, &size)== -1){
			perror("Error: ACK reception\n");
			close(socket);
			exit(-1);
		}
		if(strcmp(req2,"ACK") != 0){
			perror("Error: ACK message received\n");
			close(socket);
			exit(-1);
		}
	} else{
			perror("Error: SYN message received\n");
			close(socket);
			exit(-1);
	}

	// structure retournée
	CONNECT* res = (CONNECT*)malloc(sizeof(CONNECT));
	res->result = 1;
	res->port = (char*)malloc(sizeof(char));
	strcpy(res->port, no_port);

	return res;
}

// génère un numéro de port random entre 5001 et 10000
char* getPort(){
	srand(time(NULL));
	char* res = (char*)malloc(sizeof(char));
	int a = (rand()%(9000-25)) + 1025;
	snprintf(res, 5, "%d", a);
	printf("random port generated : %s\n", res);
	return res;
}

// fragmente les données en frames
FRAME* fragment(FILE* fp, char* filename, int index){
	FRAME* frame = (FRAME*)malloc(sizeof(FRAME));
	int j = index*1018;
	int nb_read = 0;

	snprintf(frame->seq_no, 6, "%d", index); // copie du numéro de séquence
	printf("seq_no : %s\n", frame->seq_no);

	int i=0, cont=1;
	while(cont && (i < RCVSIZE-6)){
		nb_read = fread(frame->data, RCVSIZE-6, 1, fp);
		if(feof(fp)){
			cont = 0;
		}
		j++;
		i++;
	}if(j!= RCVSIZE-6) {
		printf("EOF at position %d\n", j);
	}
	return frame;
}

// normalise un numéro de séquence sur six chiffres
char* normalizeNumber(char* noSeq){
	static char normalized[6+1];
	int s = strlen(noSeq);
	// ajout des zéros initiaux
	switch(s){
		case 0:
			perror("Error: size of noSeq equal to zero\n");
			close(socket);
			exit(-1);
		break;
		case 1:
			strcpy(normalized, "00000");
		break;
		case 2:
			strcpy(normalized, "0000");
		break;
		case 3:
			strcpy(normalized, "000");
		break;
		case 4:
			strcpy(normalized, "00");
		break;
		case 5:
			strcpy(normalized, "0");
		break;
		case 6:
			strcpy(normalized, "");
		break;
	}
	// on complète pour atteindre 6 chiffre plus le '\0'
	strcat(normalized, noSeq);
	normalized[6]='\0';
	return normalized;
}

// concatène deux chaînes de caractères
void memcat(unsigned char* src, unsigned char* dest, int destsize){
    src+=sizeof(src);
    for (int i=0; i<destsize; i++){
       *src++ = *dest++;
   }
}
