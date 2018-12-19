#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {

  //server creates Universal FIFO & waits
  if (mkfifo("Universal", 0777) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Creating Universal FIFO...\n");
  
  int up = open("Universal", O_RDWR);
  if (up == -1){
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }

  //server recieves client's message & removes WKP
  char * buffer = malloc(sizeof(char *) * BUFFER_SIZE);
  if (read(up, buffer, BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Reieved private FIFO: %s, removing Universal FIFO...\n", buffer);
  close(up);
  
  //server connects to client FIFO, sending acknowledgement message
  *to_client = open(buffer, O_RDWR);
  if (*to_client == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Connecting to the private FIFO...\n");
  
  if (write(*to_client, ACK, BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Sending acknowledgement...\n");
  
  close(*to_client);
  free(buffer);
  return up;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  //client creates private FIFO
  if (mkfifo("Unique", 0777) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Creating private FIFO...\n");
  
  int down = open("Unique", O_RDWR);
  if (down == -1){
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }

  //client connects to server & sends the private FIFO & waits
  *to_server = open("Universal", O_RDWR);
  if (write(*to_server, "Unique", BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Connecting to SERVER and sending private FIFO...\n");

  //client receives message, removes private FIFO
  char * ackmessage = malloc(sizeof(char*) * BUFFER_SIZE);
  if (read(down, ackmessage, BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Acknowledgement '%s' recieved...\n", ackmessage);
  
  close(down);

  //client sends responce to server
  if (write(*to_server, "Responce", BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("Sending responce to SERVER");
  
  free(ackmessage);
  return down;
}
