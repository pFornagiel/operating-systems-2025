#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Użycie: %s <liczba_procesów> \n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        fprintf(stderr, "Liczba procesów musi być liczbą naturalną.\n");
        return EXIT_FAILURE;
    }

    printf("PID rodzica (call w rodzicu): %d\n", (int)getpid());

    for(int i = 0;i < n;i++){
        pid_t pid = fork();
        if(pid < 0) {
            fprintf(stderr, "fork nieudany");
            return EXIT_FAILURE;
        }
        if(pid == 0) {
            printf("PID rodzica: %d, PID dziecka: %d\n", getppid(), getpid());
            return EXIT_SUCCESS; // Proces potomny kończy działanie
        }
    }

    while(wait(NULL) > 0);
    
    return EXIT_SUCCESS;
}
