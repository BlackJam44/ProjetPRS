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


int main (int argc, char *argv[]) {
  int PORT_COMMUNICATION;

  // structure addr_in creation
  struct sockaddr_in adresse;
  int size = sizeof(struct sockaddr);
  int valid= 1;
  char buffer[RCVSIZE];

  // creation of connection socket
  int connect_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (connect_sock ==  INVALID_SOCKET) {
    perror("Error: socket initialization\n");
    return -1;
  }

  // enables the reuse of the socket
  setsockopt(connect_sock, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  memset((char*)&adresse, 0, sizeof(adresse));
  adresse.sin_family= AF_INET;
  adresse.sin_port= htons(PORT_CONNEXION);
  adresse.sin_addr.s_addr= htonl(INADDR_ANY);

  if (bind(connect_sock, (struct sockaddr*) &adresse, sizeof(adresse)) == -1) {
    perror("Connection bind fail\n");
    close(connect_sock);
    return -1;
  }

  // Initialization of connection with client
  CONNECT* init = (CONNECT*)malloc(sizeof(CONNECT));
  init = openClient(connect_sock, (struct sockaddr*) &adresse);
  PORT_COMMUNICATION = atoi(init->port);
	printf("New communication port: %d\n", PORT_COMMUNICATION);

	// Communication socket on new port
	int comm_socket= socket(AF_INET, SOCK_DGRAM, 0);
  if (connect_sock ==  INVALID_SOCKET) {
    perror("Error: socket initialization\n");
    return -1;
  }

  setsockopt(comm_socket, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

	struct sockaddr_in adresse2;
	memset((char*)&adresse2, 0, sizeof(adresse2));
  adresse2.sin_family= AF_INET;
  adresse2.sin_port= htons(PORT_COMMUNICATION);
  adresse2.sin_addr.s_addr= htonl(INADDR_ANY);

  if (bind(comm_socket, (struct sockaddr*) &adresse2, sizeof(struct sockaddr)) == -1) {
    perror("Communication bind fail\n");
    close(comm_socket);
    return -1;
  }


  printf("\nWaiting for a client request... \n");
  fd_set readfds;
  struct timeval timeout;

  FD_ZERO(&readfds);
	FD_SET(comm_socket, &readfds);

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

  int desc = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
  if(desc == 0) {
    printf("No request received. Aborting connection.\n");
    char msg[4];
    strcpy(msg, "FIN");
    if(sendto(comm_socket, msg, 4, 0, (struct sockaddr*) &adresse2, size) == -1){
      perror("Error: FIN message\n");
      close(comm_socket);
      exit(-1);
    } else {
        printf("Sending \"FIN\" message...\n\n");
        memset(buffer,0,RCVSIZE);
        close(connect_sock);
        close(comm_socket);
        printf("\tConnection closed.\n");
        exit(-1);
      }
  }


  // data reception
  if(recvfrom(comm_socket, buffer, RCVSIZE, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
    perror("Error: recvfrom()\n");
    close(comm_socket);
    exit(-1);
  }
  printf("Client requires file: %s\n", buffer);

  FILE* fichier_client = fopen(buffer, "rb");   // ouverture du fichier BINAIRE en LECTURE
  if(!fichier_client){
    perror("Error: file opening\n");
    exit(-1);
  } else{
    printf("Opening and reading file...\n");
    int no_seq = 0;     // numero de sequence (ACK)

    fseek(fichier_client, 0, SEEK_END); // initialisation d'un pointeur dans le fichier
    size_t fpsize = ftell(fichier_client);
    fseek(fichier_client, 0, SEEK_SET);

    while(ftell(fichier_client) < fpsize) {
      // preparation du numero de sequence
      no_seq++;
      char nb[6];
    	snprintf(nb, 6, "%d", no_seq);
      char* ack_frame = (char*)malloc(6*sizeof(char));
      ack_frame = normalizeNumber(nb);

      // preparation des donnees
      char* data_frame = (char*)malloc((RCVSIZE-6)*sizeof(char));

      int readfile = fread(data_frame, RCVSIZE-6, 1, fichier_client);
      strcat(ack_frame, data_frame);

      printf("Sequence sent: %s\n\nPointer position : %d\nFile size : %d\n", ack_frame, ftell(fichier_client), fpsize);

      if(ftell(fichier_client) >= fpsize) {
        if(sendto(comm_socket, ack_frame, fpsize-((no_seq-1)*1018)+6, 0, (struct sockaddr*) &adresse2, size) == -1){
      		perror("Error: frame data\n");
      		close(comm_socket);
      		exit(-1);
      	}
      } else {
        if(sendto(comm_socket, ack_frame, RCVSIZE, 0, (struct sockaddr*) &adresse2, size) == -1){
      		perror("Error: frame data\n");
      		close(comm_socket);
      		exit(-1);
      	}
      }

      // Verifier si une retransmission est necessaire
      if(FD_ISSET(comm_socket, &readfds) != 0) FD_CLR(comm_socket, &readfds);
    	else FD_ZERO(&readfds);
    	FD_SET(comm_socket, &readfds);
      desc = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);

      if(desc == 0) {
        printf("Timeout. Retransmitting...\n");
        if(sendto(comm_socket, ack_frame, RCVSIZE, 0, (struct sockaddr*) &adresse2, size) == -1){
          perror("Error: frame data\n");
          close(comm_socket);
          exit(-1);
        }
      }

      if(recvfrom(comm_socket, buffer, 10, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
        perror("Error: recvfrom()\n");
        close(comm_socket);
        exit(-1);
      }

      printf("Received message: %s\n\n", buffer);
      strcpy(ack_frame, "");
    }

    fclose(fichier_client);

    char msg[4];
    strcpy(msg, "FIN");
    if(sendto(comm_socket, msg, 4, 0, (struct sockaddr*) &adresse2, size) == -1){
      perror("Error: FIN message\n");
      close(comm_socket);
      exit(-1);
    } else printf("Sending \"FIN\" message...\n\n");
  }


  memset(buffer,0,RCVSIZE);

	close(connect_sock);
  close(comm_socket);
  printf("\tConnection closed successfully.\n");

  return 0;
}
