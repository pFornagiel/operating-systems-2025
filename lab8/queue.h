#include <stddef.h>

#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_LEN 10
#define MSG_LEN 11

typedef struct queue {
  char arr[QUEUE_LEN][MSG_LEN];
  int head;
  int tail;
} queue;

int is_full(struct queue *Q);
int is_empty(struct queue *Q);
int push(struct queue *Q, char *msg);
int pop(struct queue *Q, char *buff);
int peek(struct queue *Q, char *buff);
size_t len(struct queue *Q);


#endif