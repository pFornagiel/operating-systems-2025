#ifndef COMMON_H
#define COMMON_H

#define MAX_MSG_LENGTH 1000
typedef int clientCount_t;
typedef struct message{
  unsigned long id;
  char buffer[MAX_MSG_LENGTH];
} message;

#endif
