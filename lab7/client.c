#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

unsigned long generate_unique_id() {
  pid_t pid = getpid();
  time_t now = time(NULL);
  
  // Unique ID - combination of first 16 bits of timestamp and 16 bits of PID
  return ((unsigned long)now << 16) | (pid & 0xFFFF);
}

int main(){
  struct mq_attr attr = {
    .mq_curmsgs = 0,
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_msgsize = sizeof(struct message)
  };

  unsigned long idSelf = generate_unique_id();
  
  char mqName[33];
  snprintf(mqName, sizeof(mqName), "/%lu", idSelf);

  // If existent - unlink old message queue from system
  if(mq_unlink(mqName) != -1){
    printf("Removed old client message queue %s from the system", mqName);
  }

  // Open own and server queue
  mqd_t mqdSelf;
  if((mqdSelf = mq_open(mqName, O_RDWR | O_CREAT, 0666, &attr)) == -1){
    perror("Cant open own queue");
    return EXIT_FAILURE;
  }

  mqd_t mqdServer;
  if((mqdServer = mq_open("/mqServer", O_RDWR, 0664, &attr)) == -1){
    perror("Cant open server queue");
    return EXIT_FAILURE;
  }

  printf("Client started.\n");

  // Send init message to server
  struct message msgInit = {.id = idSelf};
  strcpy(msgInit.buffer, "INIT");

  if(mq_send(mqdServer, (char *)&msgInit, sizeof(struct message), 1) == -1){
    perror("Could not send INIT message to server");
    return EXIT_FAILURE;
  }

  printf("Client: INIT message to server sent.\n");


  // Get the reply with new id used for communication
  struct message replyInit;
  if(mq_receive(mqdSelf, (char *)&replyInit, sizeof(struct message), NULL) == -1){
    perror("Could not get the ACK signal from the server");
    return EXIT_FAILURE;
  }
  
  clientCount_t idClient;
  if(strcmp("ACK", replyInit.buffer) != 0){
    printf("%s\n", replyInit.buffer);
    printf("Init error - Received message other than ACK.\n");
    return EXIT_FAILURE;
  }

  printf("Client: Received ACK message from server.\n");

  idClient = replyInit.id;


  // Start two processes - one for receiving and other for sending
  int pid = fork();

  if(pid == -1){

    // Panic when could not fork
    perror("Could not fork. Ending communication");
    mq_close(mqdServer);
    mq_close(mqdSelf);
    mq_unlink(mqName);
    return EXIT_FAILURE;
  } else if(pid > 0){

    // Send messages read from standard input
    char buffer[MAX_MSG_LENGTH];
    struct message msgBroadcast = {.id = idClient};
    
    while(fgets(buffer, MAX_MSG_LENGTH, stdin) != NULL){
      strcpy(msgBroadcast.buffer, buffer);
      if(mq_send(mqdServer,(char *)&msgBroadcast, sizeof(struct message), 1 ) == -1) {
        perror("Could not send broadcast message by client");
        continue;
      }
    }

    printf("Client: sending broadcast message to server.\n");

  } else {

    // Receive the messages and print them to standard output
    struct message msgServer;
    __pid_t ownPid = getpid();
    while(mq_receive(mqdSelf,(char *)&msgServer, sizeof(struct message), NULL) != -1){
      fprintf(stdout, "Received by PID=%d:\n%s\n", ownPid, msgServer.buffer);
    }
  }

  return EXIT_SUCCESS;
}