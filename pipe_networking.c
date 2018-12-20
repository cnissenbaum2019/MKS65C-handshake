#include "pipe_networking.h"

static void sighandler(int signo) {
  printf("Resetting Server\n");
  char * args[3] = {"rm", "Universal", NULL};
  if(!fork()) {
    if(!fork()) {
      args[1] = "Unique";
      execvp(args[0], args);
    }
    execvp(args[0], args);
  } else {
    printf("Terminated the Communications\n");
  }
  exit(0);
}

void process(char * input) {
  while(*input) {
    (*input)++;
    input++;
  }
}

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
  printf("[SERVER] Creating Universal FIFO...\n");

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
  printf("[SERVER] Reieved private FIFO: %s, removing Universal FIFO...\n", buffer);

  //server connects to client FIFO, sending acknowledgement message
  *to_client = open(buffer, O_RDWR);
  if (*to_client == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[SERVER] Connecting to the private FIFO...\n");

  if (write(*to_client, ACK, BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[SERVER] Sending acknowledgement...\n");

  //repeat until client exits
  //get data from the client
  while(1) {
        
    printf("[SERVER] Awaiting message from Client\n");
    if (read(up, buffer, BUFFER_SIZE) == -1) {
      printf("Error: %s\n", strerror(errno));
      exit(1);
    }
    printf("[SERVER] Recieved '%s'\n", buffer);
    
    //"Process" the data
    process(buffer);
    
    printf("[SERVER] Sending '%s' back\n", buffer);
    //Send new data back
    write(*to_client, buffer, BUFFER_SIZE);
  }
  
  
  close(up);
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
  printf("[CLIENT] Creating private FIFO...\n");

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
  printf("[CLIENT] Connecting to SERVER and sending private FIFO...\n");

  //client receives message, removes private FIFO
  char * buffer = malloc(sizeof(char*) * BUFFER_SIZE);
  if (read(down, buffer, BUFFER_SIZE) == -1) {
    printf("Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[CLIENT] Acknowledgement '%s' recieved...\n", buffer);
  
  //repeat until ^C
  while (1) {
    //check for exit
    signal(SIGINT, sighandler);

    //prompt user for input
    printf("[CLIENT] What would you like to send?\n");
    
    //re-use buffer
    fgets(buffer, BUFFER_SIZE, stdin);

    printf("[CLIENT] Sending '%s' to server\n", buffer);
    //send input to server
    if (write(*to_server, buffer, BUFFER_SIZE) == -1) {
      printf("Error: %s\n", strerror(errno));
      exit(1);
    }

    //get responce from server
    if (read(down, buffer, BUFFER_SIZE) == -1) {
      printf("Error: %s\n", strerror(errno));
      exit(1);
    }

    //display it to user
    printf("[CLIENT] This is what the server sent back: %s\n", buffer);
    
  }

  free(buffer);
  close(down);
  return down;
}
