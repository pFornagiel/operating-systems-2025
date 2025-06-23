#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <math.h>

#define MAX_CLIENTS 10

struct client_info{
  clientCount_t id;
  mqd_t mqd;
};

int main(){
  if(mq_unlink("/mqServer") == 0){
    fprintf(stdout, "Old message queue /mqServer removed from system.\n");
  }

  // Set mq_attr for message queue
  struct mq_attr attr = {
    .mq_curmsgs = 0,
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_msgsize = sizeof(struct message)
  };

  // Open mq for server
  mqd_t mqdServer;
  if((mqdServer = mq_open("/mqServer", O_RDWR | O_CREAT, 0666, &attr)) == -1){
    perror("Cant open server queue");
    return EXIT_FAILURE;
  }

  

  printf("Server started. Listening... \n");

  // declare client array
  // Client count also works as an id of client
  clientCount_t clientCount = 0;
  struct client_info clientInfo[MAX_CLIENTS];

  // declare message
  struct message msg;

  // Handle messages
  while((mq_receive(mqdServer, (char *)&msg, sizeof(struct message), NULL)) != -1){
    // Handle INIT
    if(strcmp("INIT", msg.buffer) == 0){
      printf("Server: Received INIT\n");

      if(clientCount >= MAX_CLIENTS){
        printf("Cannot add any more clients.\n");
        continue;
      }

      // Open client mq
      char mqClientName[33];
      snprintf(mqClientName, sizeof(mqClientName) ,"/%lu", msg.id);
      
      mqd_t mqdClient;
      if((mqdClient = mq_open(mqClientName, O_RDWR, 0666, &attr)) == -1){
        perror("Cant open client queue");
        continue;
      }
      
      struct message reply = {.id = clientCount};
      strcpy(reply.buffer, "ACK");
      if(mq_send(mqdClient, (char *)&reply, sizeof(struct message), 1) == -1){
        perror("Could not send ACK message to client");
        continue;
      }

      printf("Server: ACK message to client sent.\n");

      // Save client info to array and increment the client count 
      clientInfo[clientCount].id = clientCount;
      clientInfo[clientCount].mqd = mqdClient;

      clientCount++;

    // Handle other messages
    } else {
      printf("Server: Received broadcast\n");
      for(int i = 0; i < clientCount; i++){
        if((int)msg.id != clientInfo[i].id){
          if(mq_send(clientInfo[i].mqd, (char *)&msg, sizeof(struct message), 1) == -1){
            perror("Could not send message");
          }
        }
      }
    }
  }

  return EXIT_FAILURE;
}