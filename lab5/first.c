#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <bits/sigaction.h>
#include <bits/types/sigset_t.h>
// #include <asm-generic/signal.h>

#define NONE "none"
#define IGNORE "ingore"
#define HANDLER "handler"
#define MASK "mask"

void handler(int signum) {
    printf("Otrzymano sygnał %d SIGUSR1\n", signum);
}

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Użycie: %s <typ> \n", argv[0]);
        return EXIT_FAILURE;
    }

    sigset_t newmask, oldmask, pending;

    char* type = argv[1];

    if(strcmp(IGNORE, type) == 0){
        signal(SIGUSR1, SIG_IGN);
        printf("Ignorowanie\n");
    } else if(strcmp(HANDLER, type) == 0){
        signal(SIGUSR1, handler);
        printf("Handler \n");
    } else if(strcmp(MASK, type) == 0){
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        printf("Maskowanie\n");
    } else if(strcmp(NONE, type) == 0){
        printf("Nie zmieniono zachowania\n");
    } else {
        printf("Nieprawidłowa opcja\n");
        return EXIT_FAILURE;
    }

    raise(SIGUSR1);

    if(strcmp(MASK, type) == 0){
        sigpending(&pending);
        if(sigismember(&pending, SIGUSR1)){
            printf("SIGUSR1 znajduje się w oczekujących sygnałach.\n");
        }else{
            printf("SIGUSR1 nie jest w oczekujących sygnałach.\n");
        }
        // mask reset
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
        
    }

    return EXIT_SUCCESS;
}
