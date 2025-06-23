#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
// #include <linux/time.h>

#define INTERVAL_START 0
#define INTERVAL_END 1

float f(double x){
    if(x < INTERVAL_START || x > INTERVAL_END){
        fprintf(stderr, "Function undefined at x %f", x);
        return EXIT_FAILURE;
    }
    return 4/(x*x + 1);
}

double quadrature_at_point(double x, double width){
    return f(x) * width;
}

double quadrature_of_interval(double a, double b, double width){
    double current_x = a;
    double sum = 0;
    while(current_x < b && current_x < INTERVAL_END){
        sum += quadrature_at_point(current_x, width);
        current_x += width;
    }
    return sum;
}



int main(int argc, char *argv[])
{
    if(argc != 3){
        fprintf(stderr, "Usage: %s <quadrature_width> <max_processes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    float width = atof(argv[1]);
    int n = atoi(argv[2]);


    struct timespec start, end;
    for(int i = 1; i <=n ;i++){
        double sectionWidth = (INTERVAL_END - INTERVAL_START) / (double)i;
        // Declare an array of descriptors
        int *result = (int*)malloc((i) * sizeof(int));

        // Begin measuring time
        clock_gettime(CLOCK_MONOTONIC, &start);

        
        
        for(int k = 0; k< i; k++){
            int fd[2];
            pipe(fd);

            // Assign current READ descriptor to corresponding array index
            result[k] = fd[0];

            if(fork() == 0){
                close(fd[0]);
                
                double result = quadrature_of_interval(k * sectionWidth, k*sectionWidth + sectionWidth, width);
                
                if(write(fd[1], &result, sizeof(double)) == -1){
                    fprintf(stderr, "Could not write to pipe");
                    close(fd[1]);
                    exit(EXIT_FAILURE);
                }
                close(fd[1]);
                exit(EXIT_SUCCESS);

            } else {
                close(fd[1]);
            }
        }
        // Wait for all to finish
        while(wait(NULL) > 0){}

        // Read from descriptors saved in the array
        double result_sum = 0;
        for(int j = 0; j < i; j++){
            double partial_sum;
            if(read(result[j], &partial_sum, sizeof(double)) == -1){
                fprintf(stderr, "Could not read from pipe %d", j);
                free(result);
                exit(EXIT_FAILURE);
            }

            result_sum += partial_sum;
        }
        free(result);

        // Stop measuring time
        clock_gettime(CLOCK_MONOTONIC, &end);
        float total_time = (float)(end.tv_sec - start.tv_sec) + 
                 (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("k: %d, result: %f, time: %fs\n", i, result_sum, total_time);
    }
}
