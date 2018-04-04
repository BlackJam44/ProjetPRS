#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define RCVSIZE 1024
#define CONNECTION_PORT 9999
#define SOCK_TAB [40]

typedef struct elem_init{
  int result;
  char* port;
} CONNECT;

typedef struct elem_frame{
  char seq_no[6];
  char data[RCVSIZE-6];
} FRAME;

CONNECT* openServer(int socket, struct sockaddr* addr);
CONNECT* openClient(int socket, struct sockaddr* addr);
int createChannel(int no_port);
char* getPort();
FRAME* fragment(FILE* fp, char* filename, int index);
char* normalizeNumber(char* noSeq);

#endif
