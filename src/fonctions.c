#include <arpa/inet.h>
#include <errno.h> // codes socket error
#include <fcntl.h> // fonction open()
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h> // fonction close()

#include "fonctions.h"


CONNECT* openClient(int socket, struct sockaddr* addr){ // exécuté côté serveur
	int size = sizeof(struct sockaddr);
	char msg[12]; 				// "SYN-ACKxxxx" avec xxxx le numero de port
	char req_client[4]; 	// "SYN" ou "ACK"
	char* no_port = (char*)malloc(sizeof(char));

	printf("En attente de connexion...\n");

	if((recvfrom(socket, req_client, 10, 0, addr, &size) == -1)||(strcmp(req_client,"SYN") != 0)){
		perror("Error: SYN reception\n");
		close(socket);
		exit(-1);
	}

	strcpy(msg, "SYN-ACK");
	strcpy(no_port, getPort());
	strcat(msg, no_port);
	if(sendto(socket, msg, 8, 0, addr, size) == -1){
		perror("Error: SYN-ACK<no_port>\n");
		close(socket);
		exit(-1);
	}

	memset(req_client,0,4);
	if((recvfrom(socket, req_client, 4, 0, addr, &size)== -1)||(strcmp(req_client,"ACK") != 0)){
		perror("Error: ACK reception\n");
		close(socket);
		exit(-1);
	}

	printf("\tConnexion ouverte.\n");

	CONNECT* res = (CONNECT*)malloc(sizeof(CONNECT));
	res->result = 1;
	res->port = (char*)malloc(sizeof(char));
	strcpy(res->port, no_port);

	return res;
}


// génère un numéro de port entre 1000 et 9999
char* getPort(){
	srand(time(NULL));
	char* res = (char*)malloc(sizeof(char));
	int a = (rand()%(9000-25)) + 1025;
	snprintf(res, 5, "%d", a);
	printf("Nouveau port : %s\n", res);
	return res;
}


FRAME* fragment(FILE* fp, char* filename, int index){
	FRAME* frame = (FRAME*)malloc(sizeof(FRAME));
	int j = index*1018;
	int nb_read = 0;

	snprintf(frame->seq_no, 6, "%d", index); // sequence number copying

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

char* normalizeNumber(int noSeq){
	char normalized[6] = "000000";
	int n, i;
	switch(noSeq){
		
	}

	for(i=0; i<n; i++){
		normalized[6-n+i]=noSeq[i];
	}
	return normalized;
}
