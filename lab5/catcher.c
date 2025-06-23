#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

sig_atomic_t mode = 0;
sig_atomic_t request_count = 0;
pid_t sender_pid = 0;
int running = 1;
int current_mode = 0;

void sigint_handler(int signo) {
  printf("Wciśnięto CTRL+C\n");
}

void sigusr1_handler(int sig, siginfo_t *info, void *ucontext) {
  mode = info->si_value.sival_int;
  sender_pid = info->si_pid;
  request_count++;

  switch(mode){
    case 1:
        printf("Liczba żądań zmiany trybu: %d\n", request_count);
        current_mode = 1;
        break;
    case 2:
        current_mode = 2;
        printf("Licznik: %d\n", request_count);
        sleep(1);
        break;
    case 3:
        current_mode = 3;
        signal(SIGINT, SIG_IGN);
        printf("Tryb 3: ignorowanie Ctrl+C\n");
        break;
    case 4:
        current_mode = 4;
        signal(SIGINT, sigint_handler);
        printf("Tryb 4: reagowanie na Ctrl+C\n");
        break;
    case 5:
        current_mode = 5;
        printf("Tryb 5: kończenie działania\n");
        running = 0;
        break;
    default:
        current_mode = 0;
        printf("Nieznany tryb: %d\n", mode);
  }

  kill(sender_pid, SIGUSR1);
}


int main(){
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigusr1_handler;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGUSR1, &sa, NULL);

  printf("Catcher PID: %d\n", getpid());

  while(running){
    if (current_mode == 2) {
        printf("Licznik: %d\n", request_count);
        sleep(1);
  } else {
    pause();  // czekamy na sygnały
  }

  }

  printf("Catcher zakończył działanie.\n");
  return 0;
}
