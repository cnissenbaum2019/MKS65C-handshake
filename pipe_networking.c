#include "pipe_networking.h"

static void sighandler(int signo) {
  printf("[SERVER] Killing Server\n");
  remove("Universal");
  exit(0);
}

// FOR 2ND PART ONLY
// void process(char * input) {
//   while(*input) {
//     (*input)++;
//     input++;
//   }
// }

/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake_helper(int *to_client) {
  
  remove("Universal");

  //server creates Universal FIFO & waits
  if (mkfifo("Universal", 0777) == -1) {
    printf("[SERVER] Error: %s\n", strerror(errno));
    exit(1);
  }
  

  int up = open("Universal", O_RDWR);
  if (up == -1){
    printf("[SERVER] Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[SERVER] Created Universal FIFO\n");
  
  //server recieves client's message & removes WKP
  char * buffer = malloc(sizeof(char *) * BUFFER_SIZE);
  if (read(up, buffer, BUFFER_SIZE) == -1) {
    printf("[SERVER] Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[SERVER] Recieved private FIFO: %s, removing Universal FIFO\n", buffer);

  
  if(!fork()) { //CHILD PROCESS
    printf("[SERVER] Child Process created\n");
    
    *to_client = open(buffer, O_RDWR);
    if (*to_client == -1) {
      printf("[SERVER] Error: %s\n", strerror(errno));
      exit(1);
    }
    printf("[SERVER] Connected to Client FIFO\n");
    
    if(write(*to_client, ACK, BUFFER_SIZE) == -1) {
      printf("[SERVER] Error: %s\n", strerror(errno));
      exit(1);
    }
    printf("[SERVER] ACK sent\n");
    
    if(read(up, buffer, BUFFER_SIZE) == -1) {
      printf("[SERVER] Error: %s", strerror(errno));
      exit(1);
    }
    printf("[SERVER] Recieved message from Client: %s", buffer);
    
    close(up);
    close(*to_client);
    free(buffer);
    exit(0);
    
  } else { // PARENT PROCESS
    remove("Universal");
    printf("[SERVER] Removed Universal FIFO\n");
  }
  
  return 0;
}

//SERVER loops and can hold many clients (hopefully)
int server_handshake(int *to_client) {
  while(1) {
    signal(SIGINT, sighandler);
    
    server_handshake_helper(to_client);
  }
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  
  remove("Unique");

  //client creates private FIFO
  if (mkfifo("Unique", 0777) == -1) {
    printf("[CLIENT] Error: %s\n", strerror(errno));
    exit(1);
  }

  int down = open("Unique", O_RDWR);
  if (down == -1){
    printf("[CLIENT] Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[CLIENT] Created private FIFO\n");

  //client connects to server & sends the private FIFO & waits
  *to_server = open("Universal", O_RDWR);
  if (write(*to_server, "Unique", BUFFER_SIZE) == -1) {
    printf("[CLIENT] Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[CLIENT] Connected to SERVER\n");
  printf("[CLIENT] Sent private FIFO\n");

  //client receives message, removes private FIFO
  char * buffer = malloc(sizeof(char*) * BUFFER_SIZE);
  if (read(down, buffer, BUFFER_SIZE) == -1) {
    printf("[CLIENT] Error: %s\n", strerror(errno));
    exit(1);
  }
  printf("[CLIENT] Acknowledgement '%s' recieved\n", buffer);
  
  remove("Unique");
  printf("[CLIENT] Removed Private FIFO\n");
  
  //prompt user for input
  printf("[CLIENT] What would you like to send?\n");
  
  //re-use buffer
  fgets(buffer, BUFFER_SIZE, stdin);

  //send input to server
  if (write(*to_server, buffer, BUFFER_SIZE) == -1) {
    printf("[CLIENT] Error: %s\n", strerror(errno));
    exit(1);
  }
  
  printf("[CLIENT] Sent '%s' to server\n", buffer);

  free(buffer);
  close(down);
  return down;
}
