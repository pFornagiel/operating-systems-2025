#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <asm-generic/fcntl.h>

float f(double x){
    return 4/(x*x + 1);
}

double quadrature_at_point(double x, double width){
    return f(x) * width;
}

double quadrature_of_interval(double a, double b, double width){
    double current_x = a;
    double sum = 0;
    while(current_x < b){
        sum += quadrature_at_point(current_x, width);
        current_x += width;
        printf("%f" ,current_x);
    }
    return sum;
}

int main(){
    double interval[3];
    mkfifo("/tmp/integral_result", 0666);
    mkfifo("/tmp/integral_calc", 0666);
    
    int fd_read = open("/tmp/integral_calc", O_RDONLY);
    read(fd_read, interval, sizeof(double) * 3);
    close(fd_read);
    unlink("/tmp/integral_calc");
    
    double result = quadrature_of_interval(interval[1], interval[2], interval[0]);
    
    int fd_write = open("/tmp/integral_result", O_WRONLY);
    write(fd_write, &result, sizeof(double));
    
    return EXIT_SUCCESS;
}