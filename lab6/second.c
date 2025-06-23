#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <asm-generic/fcntl.h>


int main(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, "Usage: %s <quadrature_width> <start_interval> <end_interval>\n", argv[0]);
        return EXIT_FAILURE;
    }
    

    double interval[] = {atof(argv[1]), atof(argv[2]), atof(argv[3])};
    
    // mkfifo("/tmp/integral_calc", 0666);
    int fd_write = open("/tmp/integral_calc", O_WRONLY);
    if(write(fd_write, interval, sizeof(double) *  3) == -1){
        perror( "Could not write to pipe");
        close(fd_write);
        return EXIT_FAILURE;
    };
    close(fd_write);

    sleep(1);

    int fd_read = open("/tmp/integral_result", O_RDONLY);
    if(fd_read == -1){
      perror("Could not read");
      return EXIT_FAILURE;
    }
    double result;
    if(read(fd_read, &result, sizeof(double)) == -1){
        perror("Could not read from pipe");
        close(fd_read);
        return EXIT_FAILURE;
    }
    unlink("/tmp/integral_result");
    printf("Result: %f", result);

    return EXIT_SUCCESS;
}
