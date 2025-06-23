#define _GNU_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "cprint.h"
#include "shared.h"

#define CLIENT_COLOR "\x1b[32m"

int server_fd;
struct sockaddr_in server_addr = {.sin_family = AF_INET};
char client_name[CLIENT_NAME_LEN + 1];

void send_stop_and_exit(int signum);
void send_message_to_server(message *msg);
void parse_and_send_command(const char *command);

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <client_name> <server_ip> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }
  if (strlen(argv[1]) != CLIENT_NAME_LEN) {
    fprintf(stderr, "Clients name must be %d characters long", CLIENT_NAME_LEN);
    return EXIT_FAILURE;
  }

  strcpy(client_name, argv[1]);
  char *server_ip = argv[2];
  int port = atoi(argv[3]);

  signal(SIGINT, send_stop_and_exit);

  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

  server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd == -1) {
    perror("socket");
    return EXIT_FAILURE;
  }

  // Send register message
  message reg_msg;
  strcpy(reg_msg.type, "REG");
  strcpy(reg_msg.client_id, client_name);
  send_message_to_server(&reg_msg);

  // Setup epoll
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    return EXIT_FAILURE;
  }

  struct epoll_event event = {.events = EPOLLIN, .data.fd = server_fd};

  struct epoll_event events[MAX_EVENTS];

  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

  // Add STDIN so that, whenever something is typed to terminal, it shall be
  // sent to client to handle
  event.data.fd = STDIN_FILENO;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);

  cprintf(INFO_COLOR, "Client ready. You can now type commands.\n");

  while (1) {
    char buffer[BUFFER_SIZE] = {0};
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; ++i) {
      int fd = events[i].data.fd;
      if (fd == server_fd) {
        // Handle the messages from the server
        message recv_msg;
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        int len = recvfrom(server_fd, (void *)&recv_msg, sizeof(recv_msg), 0,
                           (struct sockaddr *)&from_addr, &from_len);

        if (len <= 0) {
          cprintf(INFO_COLOR, "Error receiving from server.\n");
          continue;
        }

        if (strncmp(recv_msg.type, "ALIVE", 5) == 0) {
          // Respond to server ping
          message ack_msg;
          strcpy(ack_msg.type, "ALIVE_ACK");
          strcpy(ack_msg.client_id, client_name);
          send_message_to_server(&ack_msg);
        } else if (strncmp(recv_msg.type, "LIST", 4) == 0) {
          // Display client list
          cprintf(CLIENT_COLOR, "Client List:\n%s", recv_msg.message);
        } else if (strncmp(recv_msg.type, "MSG", 3) == 0) {
          // Display message
          cprintf(CLIENT_COLOR, "%s\n", recv_msg.message);
        }

      } else if (fd == STDIN_FILENO) {

        // Input from terminal (STDIN)
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
          continue;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
          buffer[len - 1] = '\0';
        }

        parse_and_send_command(buffer);
      }
    }
  }

  return EXIT_SUCCESS;
}

void send_stop_and_exit(int signum) {
  message msg = {0};
  strcpy(msg.type, "STOP");
  strcpy(msg.client_id, client_name);
  sendto(server_fd, (void*)&msg, sizeof(msg), 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
  cprintf(INFO_COLOR, "\nDisconnected from server.\n");
  exit(EXIT_FAILURE);
}

void send_message_to_server(message *msg) {
  sendto(server_fd, (void *)msg, sizeof(*msg), 0,
         (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void parse_and_send_command(const char *command) {
  message msg;
  strcpy(msg.client_id, client_name);

  if (strncmp(command, "STOP", 4) == 0) {
    send_stop_and_exit(SIGINT);
  } else if (strncmp(command, "LIST", 4) == 0) {
    strcpy(msg.type, "LIST");
    send_message_to_server(&msg);
  } else if (strncmp(command, "2ALL ", 5) == 0) {
    strcpy(msg.type, "2ALL ");
    strncpy(msg.message, command + 5, sizeof(msg.message) - 1);
    send_message_to_server(&msg);
  } else if (strncmp(command, "2ONE ", 5) == 0) {
    strcpy(msg.type, "2ONE");

    // 2ONE <client> <message>
    const char *rest = command + 5;
    if (strlen(rest) < CLIENT_NAME_LEN + 1) {
      cprintf(CLIENT_COLOR, "Usage: 2ONE <client> <message>\n");
      return;
    }

    strncpy(msg.target_client, rest, CLIENT_NAME_LEN);
    msg.target_client[CLIENT_NAME_LEN] = '\0';

    if (rest[CLIENT_NAME_LEN] == ' ') {
      strncpy(msg.message, rest + CLIENT_NAME_LEN + 1, sizeof(msg.message) - 1);
      send_message_to_server(&msg);
    } else {
      cprintf(CLIENT_COLOR, "Usage: 2ONE <client> <message>\n");
    }

  } else {
    cprintf(CLIENT_COLOR, "Unknown command.\n");
  }
}