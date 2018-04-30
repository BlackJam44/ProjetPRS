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

/*
pour ouvrir le client : ./client1 @IP 9999 nantes.jpg
*/

// fonction de fermeture
void signal_handle(int sig){
	if (sig == SIGINT){
      close(3);
      printf("\nSocket closed\n");
      exit(-1);
	}
}

int main (int argc, char *argv[]) {

  // déclarations
  int COMMUNICATION_PORT;
  int size = sizeof(struct sockaddr);
  int valid= 1;
  char buffer[RCVSIZE-6];
	char buffToSend[RCVSIZE];
  char echo[RCVSIZE];
  struct sockaddr_in adresse;
  struct sockaddr_in adresse2;

  // création socket de connexion
  int connect_sock= socket(AF_INET, SOCK_DGRAM, 0);

  // erreur de traitement
  if (connect_sock ==  INVALID_SOCKET) {
    perror("cannot create socket udp\n");
    return -1;
  }

  // autorise réutilisation de la socket
  setsockopt(connect_sock, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  // paramètrage structure de connexion addr_in
  memset((char*)&adresse, 0, sizeof(adresse));
  adresse.sin_family= AF_INET;
  adresse.sin_port= htons(CONNECTION_PORT);
  adresse.sin_addr.s_addr= htonl(INADDR_ANY);

  // lier structure addr et le descripteur de la socket de connexion
  if (bind(connect_sock, (struct sockaddr*) &adresse, sizeof(adresse)) == -1) {
    perror("Connection bind fail\n");
    close(connect_sock);
    return -1;
  }

  // initialisation connexion avec le client
  CONNECT* init = (CONNECT*)malloc(sizeof(CONNECT));
  init = openClient(connect_sock, (struct sockaddr*) &adresse);
  COMMUNICATION_PORT = atoi(init->port); // Numéro de port spécifique
  printf("Communication port: %d\n", COMMUNICATION_PORT);

	// création socket de communication
	int comm_socket= socket(AF_INET, SOCK_DGRAM, 0);

	// paramètrage structure de communication addr_in
	memset((char*)&adresse2, 0, sizeof(adresse2));
  adresse2.sin_family= AF_INET;
  adresse2.sin_port= htons(COMMUNICATION_PORT);
  adresse2.sin_addr.s_addr= htonl(INADDR_ANY);

  setsockopt(comm_socket, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int)); // autorise réutilisation de la socket

  // lier structure addr et le descripteur de la socket de connexion
  if (bind(comm_socket, (struct sockaddr*) &adresse2, sizeof(struct sockaddr)) == -1) {
    perror("Communication bind fail\n");
    close(comm_socket);
    return -1;
  }

  while (init->result) {
    printf("\nWaiting for data... \n");
    signal(SIGINT, signal_handle);

    // réception du nom de fichier à envoyer
    if( recvfrom(comm_socket, buffer, RCVSIZE, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
      perror("Error: recvfrom()\n");
      close(comm_socket);
      exit(-1);
    }
    else{
			printf("Received: %s\n",buffer);
			wait(1);

			// Ouverture du fichier et fragmentation
			FILE* fp;
			fp = fopen(buffer ,"r");
			if(fp==NULL){
				perror("Error: file does not exist\n");
	      close(comm_socket);
	      exit(-1);
			}

			fseek(fp, 0, SEEK_END); // définit la position du fichier à la fin
			size_t fpsize = ftell(fp); // retourne la position actuelle dans le fichier
			fseek(fp, 0, SEEK_SET);

			int intNum=0;
			char charNum[6];
			int no_segment;

			while(ftell(fp)<fpsize){

				snprintf(charNum, 6, "%d", intNum);
				no_segment=normalizeNumber(charNum);
				memset(buffToSend, '\0', 1024);
				strcpy(buffToSend, no_segment);
				printf("buffToSend avant : %s\n\n", buffToSend);

				if(fread(buffer, fpsize, 1, fp) == -1){
					perror("Error: copying file into buffer\n");
					close(comm_socket);
					exit(-1);
				}
				printf("buffer : %s\n\n", buffer);
				memmove(buffToSend+6, buffer, 1018);
				printf("buffToSend après : %s\n\n", buffToSend);

				if(sendto(comm_socket, buffToSend, 1024, 0, (struct sockaddr*) &adresse2, size) == -1){
					perror("Error: sending file\n");
					close(comm_socket);
					exit(-1);
				}


				intNum++;

				//FRAME* imPart;
				// = fragment(fp, buffer, 0); // fragmentation de la première trame seulement

				/*printf("seq_no : %d \n", imPart->seq_no);
				char* new_seq_no = normalizeNumber(imPart->seq_no);
				imPart->seq_no = new_seq_no;
				printf("new_seq_no : %s \n", imPart->seq_no);

				//test conversion de char* à char* pour normaliser le num de seq

				printf("Size of frame 1 : %d\n", sizeof(imPart));*/
			}
			fclose(fp);

			/* Vérifier que le fichier soit bien découpé, que les trames s'envoient bien, faire attention
			au seq_no et au EOF
			on envoyait depuis tpclient vers tpserveur. faut faire l'inverse ici */
		}

  }

  return 1;
}
