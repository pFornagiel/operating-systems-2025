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
char client_name[CLIENT_NAME_LEN + 1];

void send_stop_and_exit(int signum);
void set_nonblocking(int fd);

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <client_name> <server_ip> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }
  if (strlen(argv[1]) != CLIENT_NAME_LEN) {
    fprintf(stderr, "Clients name must be %d characters long\n",
            CLIENT_NAME_LEN);
    return EXIT_FAILURE;
  }

  strcpy(client_name, argv[1]);
  char *server_ip = argv[2];
  int port = atoi(argv[3]);

  signal(SIGINT, send_stop_and_exit);

  struct sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
  };
  inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

  int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
    perror("socket");
    return EXIT_FAILURE;
  }

  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect");
    return EXIT_FAILURE;
  }
  server_fd = client_fd;

  // Send registration message using message struct
  message reg_msg = {0};
  strcpy(reg_msg.type, "REG");
  strcpy(reg_msg.client_id, client_name);
  write(server_fd, &reg_msg, sizeof(reg_msg));

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
        message recv_msg = {0};
        int len = read(server_fd, &recv_msg, sizeof(recv_msg));
        if (len <= 0) {
          cprintf(INFO_COLOR, "Disconnected from server.\n");
          close(server_fd);
          return EXIT_FAILURE;
        }

        if (strncmp(recv_msg.type, "ALIVE", 5) == 0) {
          message ack_msg = {0};
          strcpy(ack_msg.type, "ALIVE_ACK");
          strcpy(ack_msg.client_id, client_name);
          write(server_fd, &ack_msg, sizeof(ack_msg));
        } else if (strncmp(recv_msg.type, "LIST", 4) == 0) {
          cprintf(CLIENT_COLOR, "Client list:\n%s", recv_msg.message);
        } else if (strncmp(recv_msg.type, "2ALL", 4) == 0) {
          cprintf(CLIENT_COLOR, "%s\n", recv_msg.message);
        } else if (strncmp(recv_msg.type, "MSG", 3) == 0) {
          cprintf(CLIENT_COLOR, "(private): %s\n", recv_msg.message);
        } else {
          cprintf(CLIENT_COLOR, "%s\n", recv_msg.message);
        }

      } else if (fd == STDIN_FILENO) {

        // Input from terminal (STDIN)
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
          continue;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
          buffer[len - 1] = '\0';

        message msg = {0};

        if (strncmp(buffer, "STOP", 4) == 0) {
          send_stop_and_exit(SIGINT);
          exit(EXIT_SUCCESS);
        } else if (strncmp(buffer, "LIST", 4) == 0) {

          strcpy(msg.type, "LIST");
          strcpy(msg.client_id, client_name);
          write(server_fd, &msg, sizeof(msg));

        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
          strcpy(msg.type, "2ALL");
          strcpy(msg.client_id, client_name);
          strcpy(msg.message, buffer + 5);
          write(server_fd, &msg, sizeof(msg));
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
          // Parse "2ONE <target> <message>"
          strcpy(msg.type, "2ONE");

          // 2ONE <client> <message>
          const char *rest = buffer + 5;
          if (strlen(rest) < CLIENT_NAME_LEN + 1) {
            cprintf(CLIENT_COLOR, "Usage: 2ONE <client> <message>\n");
            continue;
          }

          strncpy(msg.target_client, rest, CLIENT_NAME_LEN);
          msg.target_client[CLIENT_NAME_LEN] = '\0';

          if (rest[CLIENT_NAME_LEN] == ' ') {
            strncpy(msg.message, rest + CLIENT_NAME_LEN + 1,
                    sizeof(msg.message) - 1);
          } else {
            cprintf(CLIENT_COLOR, "Usage: 2ONE <client> <message>\n");
            continue;
          }
          strcpy(msg.type, "2ONE");
          strcpy(msg.client_id, client_name);
          write(server_fd, &msg, sizeof(msg));
        } else {
          cprintf(CLIENT_COLOR, "Unknown command");
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

void send_stop_and_exit(int signum) {
  message stop_msg = {0};
  strcpy(stop_msg.type, "STOP");
  strcpy(stop_msg.client_id, client_name);
  write(server_fd, &stop_msg, sizeof(stop_msg));
  close(server_fd);
  cprintf(INFO_COLOR, "\nDisconnected from server.\n");
  exit(EXIT_SUCCESS);
}