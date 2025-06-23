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

void register_client(int client_fd, const message *msg);
void send_client_list(int client_fd, const message *msg);
void broadcast_message(int sender_fd, const message *msg);
void handle_direct_message(int sender_fd, const message *msg);
void remove_client(int client_fd, const message *msg);
void update_alive_status(int client_fd, const message *msg);
void *ping_clients_loop();

int find_client_by_name(const char *name);
int find_client_by_fd(int fd);

typedef struct id_element {
  char name[CLIENT_NAME_LEN + 1];
  struct in_addr ip; // struct with address to network in byte order
  int fd;            // client's socket file descriptor
  time_t last_alive;
  int active;

  time_t ping_sent_time;
  int awaiting_alive_ack;
} id_element;

id_element clients[CLIENT_COUNT];
int client_count = 0;

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

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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

  int listen_status = listen(server_fd, CLIENT_COUNT);
  if (listen_status == -1) {
    perror("Could not start listening");
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
        int client_fd = accept4(server_fd, (struct sockaddr *)&client_addr,
                                &client_len, SOCK_NONBLOCK);
        if (client_fd == -1) {
          perror("Could not accept new client");
          continue;
        }

        // We can reuse the event, because epoll_ctl copies the values anyways
        event.data.fd = client_fd;
        event.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

        cprintf(SERVER_COLOR, "Accepted new client.\n");

      } else {
        // An actual message from client
        // char buffer[BUFFER_SIZE + 1] = {0};

        message msg = {0};

        int count = read(events[i].data.fd, &msg, sizeof(msg));
        if (count <= 0) {
          // Client closed connection or error
          remove_client(events[i].data.fd, &msg);
          close(events[i].data.fd);
          continue;
        }

        if (strncmp(msg.type, "REG", 3) == 0) {
          register_client(events[i].data.fd, &msg);
        } else if (strncmp(msg.type, "LIST", 4) == 0) {
          send_client_list(events[i].data.fd, &msg);
        } else if (strncmp(msg.type, "2ALL", 4) == 0) {
          broadcast_message(events[i].data.fd, &msg);
        } else if (strncmp(msg.type, "2ONE", 4) == 0) {
          handle_direct_message(events[i].data.fd, &msg);
        } else if (strncmp(msg.type, "STOP", 4) == 0) {
          remove_client(events[i].data.fd, &msg);
          close(events[i].data.fd);
        } else if (strncmp(msg.type, "ALIVE_ACK", 9) == 0) {
          update_alive_status(events[i].data.fd, &msg);
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

void register_client(int client_fd, const message *msg) {
  int client_index = find_client_by_name(msg->client_id);
  if (client_index != -1) {
    cprintf(SERVER_COLOR, "Client %s already registered", msg->client_id);
  }

  if (client_count >= CLIENT_COUNT) {
    cprintf(SERVER_COLOR, "Maximum number of clients reached\n");
    close(client_fd);
    return;
  }

  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);

  clients[client_count] = (id_element){.fd = client_fd,
                                       .ip = addr.sin_addr,
                                       .active = 1,
                                       .last_alive = time(NULL),
                                       .awaiting_alive_ack = 0};
  snprintf(clients[client_count].name, CLIENT_NAME_LEN + 1, "%s",
           msg->client_id);

  cprintf(SERVER_COLOR, "Received REG from %s\n", clients[client_count].name);
  cprintf(SERVER_COLOR, "Registered client %s\n", clients[client_count].name);
  client_count++;
  return;
}

void send_client_list(int client_fd, const message *msg) {

  cprintf(SERVER_COLOR, "Received LIST from %s\n",
          msg->client_id);

  message return_msg = {0};
  strcpy(return_msg.client_id, msg->client_id);
  strcpy(return_msg.type, "LIST");

  for (int i = 0; i < CLIENT_COUNT; i++) {
    if (clients[i].active) {
      char entry[CLIENT_NAME_LEN + 3 + 15 + 1];
      snprintf(entry, sizeof(entry), "%s - %s\n", clients[i].name,
               inet_ntoa(clients[i].ip));
      strcat(return_msg.message, entry);
    }
  }
  write(client_fd, &return_msg, sizeof(return_msg));
}

void broadcast_message(int sender_fd, const message *msg) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  cprintf(SERVER_COLOR, "Received 2ALL from %s\n",
           msg->client_id);

  message return_msg;
  strcpy(return_msg.type, "2ALL");
  strcpy(return_msg.client_id, msg->client_id);

  snprintf(return_msg.message, sizeof(return_msg.message),
           "[%02d:%02d:%02d] %s: %s", t->tm_hour, t->tm_min, t->tm_sec,
           msg->client_id, msg->message);

  for (int i = 0; i < CLIENT_COUNT; i++) {
    if (clients[i].active && clients[i].fd != sender_fd) {
      write(clients[i].fd, &return_msg, sizeof(return_msg));
    }
  }
}

void handle_direct_message(int sender_fd, const message *msg) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  cprintf(SERVER_COLOR, "Received 2ONE from %s to %s\n",
          msg->client_id , msg->target_client);

  message return_msg;
  strcpy(return_msg.type, "MSG");
  strcpy(return_msg.target_client, msg->target_client);
  strcpy(return_msg.client_id, msg->client_id);

  snprintf(return_msg.message, sizeof(return_msg.message),
           "[%02d:%02d:%02d] %s: %s", t->tm_hour, t->tm_min, t->tm_sec,
           msg->client_id, msg->message);

  int index = find_client_by_name(msg->target_client);
  if (index == -1) {
    cprintf(SERVER_COLOR, "Could not find client to send message to\n");
  }
  write(clients[index].fd, &return_msg, sizeof(return_msg));
}

void remove_client(int client_fd, const message *msg) {

  int index = find_client_by_fd(client_fd);
  if (index == -1) {
    cprintf(SERVER_COLOR, "Could not find client to remove\n");
  }

  cprintf(SERVER_COLOR, "Received STOP from %s\n", clients[index].name);
  clients[index].active = 0;
  close(client_fd);
}

void update_alive_status(int client_fd, const message *msg) {

  int index = find_client_by_fd(client_fd);
  if (index == -1) {
    cprintf(SERVER_COLOR, "Could not find client update status\n");
  }
  cprintf(SERVER_COLOR, "Received ALIVE_ACK from %s\n", clients[index].name);
  clients[index].awaiting_alive_ack = 0;
  clients[index].last_alive = time(NULL);
}

void *ping_clients_loop() {
  while (1) {
    sleep(20); // check every second

    time_t now = time(NULL);

    for (int i = 0; i < CLIENT_COUNT; i++) {
      if (!clients[i].active)
        continue;

      if (clients[i].awaiting_alive_ack) {
        // Client was pinged and hasn't replied in time
        if (now - clients[i].ping_sent_time > 5) { // 5 seconds timeout
          cprintf(SERVER_COLOR, "Client %s did not respond to ALIVE ping.\n",
                  clients[i].name);
          close(clients[i].fd);
          clients[i].active = 0;
        }
      } else {
        // Send ALIVE message
        message return_msg;
        strcpy(return_msg.type, "ALIVE");
        if (write(clients[i].fd, &return_msg, sizeof(return_msg)) > 0) {
          clients[i].ping_sent_time = now;
          clients[i].awaiting_alive_ack = 1;
        }
      }
    }
  }
}

int find_client_by_fd(int fd) {
  for (int i = 0; i < client_count; i++) {
    if (clients[i].active && clients[i].fd == fd) {
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