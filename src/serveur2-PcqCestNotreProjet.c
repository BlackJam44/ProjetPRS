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

// fermeture : "FIN" : le client ne demande qu'un seul fichier par session

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
	printf("Nouveau port de communication : %d\n", PORT_COMMUNICATION);

	// socket de communication sur le nouveau port
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


  //while (init->result) {
    printf("\nEn attente de demande de la part du client... \n");

    // data reception
    if( recvfrom(comm_socket, buffer, RCVSIZE, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
      perror("Error: recvfrom()\n");
      close(comm_socket);
      exit(-1);
    }
    printf("Fichier requis par le client : %s\n",buffer);

    //-----------------------------------------------------------------------------------------------

/*
		while(1){
			FRAME* frame= (FRAME*)malloc(sizeof(FRAME));

			if(recvfrom(comm_socket, frame->seq_no, 6, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
				perror("Error: file recvfrom(seq_no)\n");
	      close(comm_socket);
	      exit(-1);
			} else printf("\nFrame %s \n", frame->seq_no);

			// detects the end of received file
			if(strcmp(frame->seq_no, "EOF") == 0){
				break;
			}

			if(recvfrom(comm_socket, frame->data, RCVSIZE-6, 0, (struct sockaddr*) &adresse2, &size) == -1 ){
				perror("Error: file recvfrom(data)\n");
	      close(comm_socket);
	      exit(-1);
			}else{
				printf("Content received:\n%s\n", frame->data);
				// writing in the file
				int nb_written = fwrite(frame->data, RCVSIZE-6, 1, fp);
				printf("written : %d \n", nb_written);
			}

		}
	}


    // data echo
    strcpy(echo, buffer);
    if(sendto(comm_socket, echo, RCVSIZE, 0, (struct sockaddr*) &adresse2, size) == -1){
      perror("Error: sendto()\n");
      close(comm_socket);
      exit(-1);
    }
    printf("Echo sent.\n");

		// stop process
    if (strcmp(buffer,"stop\n") == 0) {
      init->result = 0;
			close(comm_socket);
			printf("Connection closed.\n");
    }

    memset(buffer,0,RCVSIZE);
    memset(echo,0,RCVSIZE);
  }*/

  memset(buffer,0,RCVSIZE);

	close(connect_sock);
  close(comm_socket);

  return 0;
}
