#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "cprint.h"
#include "shared.h"

#define SERVER_COLOR "\x1b[31m"

void register_client(message *msg, struct sockaddr_in *addr);
void send_client_list(int server_fd, message *msg, struct sockaddr_in *addr);
void broadcast_message(int server_fd, message *msg,
                       struct sockaddr_in *sender_addr);
void handle_direct_message(int server_fd, message *msg);
void remove_client(struct sockaddr_in *addr);
void update_alive_status(struct sockaddr_in *addr);
void *ping_clients_loop(void *_arg);

int find_client_by_addr(struct sockaddr_in *addr);
int find_client_by_name(const char *name);

typedef struct id_element {
  char name[CLIENT_NAME_LEN + 1];
  struct sockaddr_in addr;
  time_t last_alive;
  int active;
  time_t ping_sent_time;
  int awaiting_alive_ack;
} id_element;

id_element clients[CLIENT_COUNT];
int client_count = 0;
int server_fd;

int main(int argc, char **argv) {

  if (argc != 3) {
    printf("Usage: %s <adress> <port>", argv[0]);
    return EXIT_FAILURE;
  }

  struct in_addr server_ip;

  inet_aton(argv[1], &server_ip);
  uint16_t port = htons(atoi(argv[2]));

  struct sockaddr_in addr_in = {
      .sin_family = AF_INET, .sin_port = port, .sin_addr = server_ip};

  server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd == -1) {
    perror("Could not create the socket");
    return EXIT_FAILURE;
  }

  int bind_status =
      bind(server_fd, (struct sockaddr *)&addr_in, sizeof(addr_in));
  if (bind_status < 0) {
    perror("Could not bind");
    return EXIT_FAILURE;
  }

  // EPOLL FOR HANDLING MULTIPLE USERS AND REQUESTS

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("Could not create epoll_create1");
    exit(EXIT_FAILURE);
  }

  struct epoll_event event = {.events = EPOLLIN, .data.fd = server_fd};
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
  struct epoll_event events[MAX_EVENTS];

  pthread_t ping_thread;
  pthread_create(&ping_thread, NULL, ping_clients_loop, NULL);

  cprintf(INFO_COLOR, "Server listening on port %d...\n", atoi(argv[2]));

  while (1) {
    // Get the number of awaiting events
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; i++) {
      // ACCEPT NEW CLIENTS
      if (events[i].data.fd == server_fd) {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        message msg;

        int msg_len = recvfrom(server_fd, &msg, sizeof(msg), 0,
                               (struct sockaddr *)&client_addr, &client_len);
        if (msg_len <= 0) {
          printf("Could not read the message\n");
          continue;
        }


        if (strncmp(msg.type, "REG", 3) == 0) {
          register_client(&msg, &client_addr);
        } else if (strncmp(msg.type, "LIST", 4) == 0) {
          send_client_list(server_fd, &msg, &client_addr);
        } else if (strncmp(msg.type, "2ALL", 4) == 0) {
          broadcast_message(server_fd, &msg, &client_addr);
        } else if (strncmp(msg.type, "2ONE", 4) == 0) {
          handle_direct_message(server_fd, &msg);
        } else if (strncmp(msg.type, "STOP", 4) == 0) {
          remove_client(&client_addr);
        } else if (strncmp(msg.type, "ALIVE_ACK", 9) == 0) {
          update_alive_status(&client_addr);
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

void register_client(message *msg, struct sockaddr_in *addr) {
  int existing_client = find_client_by_addr(addr);
  if (existing_client != -1) {
    clients[existing_client].active = 1;
    clients[existing_client].last_alive = time(NULL);
    clients[existing_client].awaiting_alive_ack = 0;
    cprintf(SERVER_COLOR, "Re-registered existing client %s\n",
            clients[existing_client].name);
    return;
  }

  if (client_count >= CLIENT_COUNT) {
    cprintf(SERVER_COLOR, "Maximum number of clients reached\n");
    return;
  }

  clients[client_count] = (id_element){.active = 1,
                                       .addr = *addr,
                                       .last_alive = time(NULL),
                                       .awaiting_alive_ack = 0};
  snprintf(clients[client_count].name, CLIENT_NAME_LEN + 1, "%s",
           msg->client_id);

  cprintf(SERVER_COLOR, "Received REG from %s\n", clients[client_count].name);
  cprintf(SERVER_COLOR, "Registered client %s from %s:%d\n",
          clients[client_count].name, inet_ntoa(addr->sin_addr),
          ntohs(addr->sin_port));
  client_count++;
  return;
}

void send_client_list(int server_fd, message *msg, struct sockaddr_in *addr) {
  cprintf(SERVER_COLOR, "Received LIST from %s\n",
          msg->client_id[0] ? msg->client_id : "unknown");

  message return_msg = {0};
  strcpy(return_msg.type, "LIST");
  strcpy(return_msg.client_id, msg->client_id);

  char entry[BUFFER_SIZE];
  for (int i = 0; i < CLIENT_COUNT; i++) {
    if (clients[i].active) {

      snprintf(entry, sizeof(entry), "%s - %s:%d\n", clients[i].name,
               inet_ntoa(clients[i].addr.sin_addr),
               ntohs(clients[i].addr.sin_port));
      strcat(return_msg.message, entry);
    }
  }

  sendto(server_fd, (void *)&return_msg, sizeof(return_msg), 0, addr,
         sizeof(*addr));
}

void broadcast_message(int server_fd, message *msg,
                       struct sockaddr_in *sender_addr) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  cprintf(SERVER_COLOR, "Received 2ALL from %s\n", msg->client_id);

  message broadcast_msg = {0};
  strcpy(broadcast_msg.type, "MSG");
  snprintf(broadcast_msg.message, sizeof(broadcast_msg.message),
           "[%02d:%02d:%02d] %s: %s", t->tm_hour, t->tm_min, t->tm_sec,
           msg->client_id, msg->message);

  in_addr_t sender_ip = sender_addr->sin_addr.s_addr;
  in_port_t sender_port = sender_addr->sin_port;

  for (int i = 0; i < CLIENT_COUNT; i++) {
    if (clients[i].active && !(clients[i].addr.sin_addr.s_addr == sender_ip &&
                               clients[i].addr.sin_port == sender_port)) {

      sendto(server_fd, (void *)&broadcast_msg, sizeof(broadcast_msg), 0,
             (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
    }
  }
}

void handle_direct_message(int server_fd, message *msg) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  cprintf(SERVER_COLOR, "Received 2ONE from %s to %s\n", msg->client_id,
          msg->target_client);

  int target_index = find_client_by_name(msg->target_client);
  if (target_index == -1) {
    cprintf(SERVER_COLOR, "Target client %s not found\n", msg->target_client);
    return;
  }

  message direct_msg = {0};
  strcpy(direct_msg.type, "MSG");
  snprintf(direct_msg.message, sizeof(direct_msg.message),
           "[%02d:%02d:%02d] %s (private): %s", t->tm_hour, t->tm_min,
           t->tm_sec, msg->client_id, msg->message);

  sendto(server_fd, (void *)&direct_msg, sizeof(direct_msg), 0,
         (struct sockaddr *)&clients[target_index].addr,
         sizeof(clients[target_index].addr));
}

void remove_client(struct sockaddr_in *addr) {
  int client_index = find_client_by_addr(addr);
  if (client_index != -1) {
    cprintf(SERVER_COLOR, "Received STOP from %s\n",
            clients[client_index].name);
    clients[client_index].active = 0;
  }
}

void update_alive_status(struct sockaddr_in *addr) {
  int client_index = find_client_by_addr(addr);
  if (client_index != -1) {
    cprintf(SERVER_COLOR, "Received ALIVE_ACK from %s\n",
            clients[client_index].name);
    clients[client_index].awaiting_alive_ack = 0;
    clients[client_index].last_alive = time(NULL);
  }
}

void *ping_clients_loop(void *_arg) {
  while (1) {
    sleep(20); // check every 20 seconds

    time_t now = time(NULL);

    for (int i = 0; i < CLIENT_COUNT; i++) {
      if (!clients[i].active)
        continue;

      if (clients[i].awaiting_alive_ack) {
        // Client was pinged and hasn't replied in time
        if (now - clients[i].ping_sent_time > 5) { // 5 seconds timeout
          cprintf(SERVER_COLOR, "Client %s did not respond to ALIVE ping.\n",
                  clients[i].name);
          clients[i].active = 0;
        }
      } else {
        // Send ALIVE message
        message alive_msg;
        strcpy(alive_msg.type, "ALIVE");
        if (sendto(server_fd, (void *)&alive_msg, sizeof(alive_msg), 0,
                   (struct sockaddr *)&clients[i].addr,
                   sizeof(clients[i].addr)) > 0) {
          clients[i].ping_sent_time = now;
          clients[i].awaiting_alive_ack = 1;
        }
      }
    }
  }
  return NULL;
}

int find_client_by_addr(struct sockaddr_in *addr) {

  in_addr_t search_ip = addr->sin_addr.s_addr;
  in_port_t search_port = addr->sin_port;
  for (int i = 0; i < client_count; i++) {
    if (clients[i].active &&
        clients[i].addr.sin_addr.s_addr == search_ip &&
        clients[i].addr.sin_port == search_port) {
      return i;
    }
  }
  return -1;
}

int find_client_by_name(const char *name) {
  for (int i = 0; i < client_count; i++) {
    if (clients[i].active && strcmp(clients[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}
