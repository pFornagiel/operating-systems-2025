#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

sig_atomic_t received = 0;

void handler(int sig) {
  received = 1;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Użycie: %s <PID catcher> <tryb>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  pid_t catcher_pid = atoi(argv[1]);
  int mode = atoi(argv[2]);

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  
  sigaction(SIGUSR1, &sa, NULL);

  // Wysyłanie sygnału z parametrem
  union sigval value;
  value.sival_int = mode;
  if (sigqueue(catcher_pid, SIGUSR1, value) == -1) {
    perror("sigqueue");
    exit(EXIT_FAILURE);
  }

  // Czekanie na potwierdzenie
  sigset_t mask, oldmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &mask, &oldmask);

  while (!received) {
    sigsuspend(&oldmask);
  }

  printf("Potwierdzenie otrzymane, kończenie sendera.\n");
  return 0;
}
