#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 0;

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Użycie: %s <nazwa_katalogu> \n", argv[0]);
        return EXIT_FAILURE;
    }

    char* dir = argv[1];

    printf("Nazwa głównego programu: %s\n", argv[0]);
    printf("PID głównego programu: %d\n", (getpid()));

    int local = 0;

    int pid = fork();
    if(pid < 0) {
        fprintf(stderr, "fork nieudany");
        return EXIT_FAILURE;
    } 
    if(pid == 0){
        printf("Child proccess\n");
        global += 1;
        local += 1;
        printf("Child PID = %d, Parent PID = %d\n", getpid(), getppid());
        printf("Child's local = %d, Child's global = %d\n", local, global);
        execl("/bin/ls", "ls", dir, (char *)NULL);
        // If program continues to execute, execl ended with error
        fprintf(stderr, "execl failed");
        exit(EXIT_FAILURE);
    }
    if(pid > 0){
        printf("Parent proccess\n");
        printf("Parent PID = %d, Child PID = %d\n", getpid(), pid);
        int status; 
        wait(&status); 
        printf("Child exit code: %d\n", WEXITSTATUS(status));
        printf("Parent's local = %d, Parent's global = %d\n", local, global);
    }
    
    return EXIT_SUCCESS;
}
