#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "queue.h"

int is_full(struct queue *Q){
  return ((Q->tail + 1)%QUEUE_LEN) == Q->head;
}

int is_empty(struct queue *Q){
  return Q->head == Q->tail;
}

int push(struct queue * Q, char * msg){
  if(strlen(msg) >= MSG_LEN){
    fprintf(stderr, "Message too long to enqueue\n");
    return EXIT_FAILURE;
  }

  if (is_full(Q)) {
    fprintf(stderr, "The end of queue has been reached\n");
    return EXIT_FAILURE;
  }

  if(snprintf(Q->arr[Q->tail], MSG_LEN, "%s", msg) < 0){
    fprintf(stderr, "Could not copy string to queue\n");
    return EXIT_FAILURE;
  }

  Q->tail = (Q->tail+1)%QUEUE_LEN;

  return EXIT_SUCCESS;
}

int pop(struct queue * Q, char *buff){
  if(is_empty(Q)){
    fprintf(stderr, "Nothing on the queue!\n");
    return EXIT_FAILURE;
  }
  snprintf(buff, MSG_LEN, "%s", Q->arr[Q->head]);
  Q->head = (Q->head + 1) % QUEUE_LEN;
  return EXIT_SUCCESS;
}

int peek(struct queue * Q, char * buff){
  if(is_empty(Q)){
    fprintf(stderr, "Nothing on the queue!\n");
    return EXIT_FAILURE;
  }
  snprintf(buff, MSG_LEN, "%s",Q->arr[Q->head]);
  return EXIT_SUCCESS;
}

size_t len(struct queue *Q){
  return (Q->tail - Q->head + QUEUE_LEN) % QUEUE_LEN;
}