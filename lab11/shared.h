#ifndef SHARED_H
#define SHARED_H

#define CLIENT_NAME_LEN 3
#define CLIENT_COUNT 10
#define MAX_EVENTS CLIENT_COUNT
#define BUFFER_SIZE 1024

#define INFO_COLOR "\x1b[33m"

typedef struct message {
  char type[10];
  char message[BUFFER_SIZE];
  char client_id[CLIENT_NAME_LEN + 1];
  char target_client[CLIENT_NAME_LEN + 1];
} message;

#endif