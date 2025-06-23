#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define INTERVAL_START 0
#define INTERVAL_END 1

float f(double x)
{
    if (x < INTERVAL_START || x > INTERVAL_END)
  {
    fprintf(stderr, "Function undefined at x %f", x);
    return EXIT_FAILURE;
  }
  return 4 / (x * x + 1);
}

double quadrature_at_point(double x, double width)
{
  return f(x) * width;
}

struct quadrature_args {
  double a;
  double b;
  double width;
};

void * quadrature_of_interval(void* args_ptr)
{
  struct quadrature_args* args = (struct quadrature_args* )args_ptr;
  double a = args->a, b = args->b, width = args->width;
  double current_x = a;
  double sum = 0;
  while (current_x < b && current_x < INTERVAL_END)
  {
    sum += quadrature_at_point(current_x, width);
    current_x += width;
  }

  double* result = malloc(sizeof(double));
  *result = sum;
  return result;

}

int main(int argc, char **argv){
  if (argc != 3){
    printf("Usage: <quadrature_width> <number_of_threads>");
    return EXIT_FAILURE;
  }

  double quadrature_width = atof(argv[1]);
  int k = atoi(argv[2])+1;

  struct timespec start, end;

  for (int i = 1; i < k; i++){
    pthread_t *tid = (pthread_t*) malloc(sizeof(pthread_t) * i);
    if(tid == NULL){
      perror("Could not allocate tid array");
      return EXIT_FAILURE;
    }

    double result = 0;
    double section_width = (INTERVAL_END - INTERVAL_START) / (double)i;
    struct quadrature_args * args_array = (struct quadrature_args *) malloc(sizeof(struct quadrature_args) * i);
    if(args_array == NULL){
      perror("Could not allocate args_array");
      free(tid);
      return EXIT_FAILURE;
    }

    // Begin time measurement
    clock_gettime(CLOCK_MONOTONIC, &start);
    // Strating threads
    for (int j = 0; j < i; j++){
      // args cannot be a static array (double args[3]), because it will live on the stack
      // The program has to allocate it on the heap for each iteration on j, and then free it afterwards
      args_array[j] = (struct quadrature_args){.a = section_width * j, .b= section_width * (j+1), .width = quadrature_width};

      pthread_create(&tid[j], NULL, quadrature_of_interval, (void*) &args_array[j]);
    }
    // Joining and adding
    for (int j = 0; j < i; j++){
      void * retval_ptr;
      pthread_join(tid[j], &retval_ptr);
      if(retval_ptr == NULL){
        perror("Could not read value from thread - aborting.");
        free(tid);
        free(args_array);
        return EXIT_FAILURE;
      }
      double retval = *(double*)retval_ptr;
      free(retval_ptr);
      result += retval;
    }

    // End time measuremnt
    clock_gettime(CLOCK_MONOTONIC, &end);
    float total_time = (float)(end.tv_sec - start.tv_sec) +
                 (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Result for k=%d: %f, time=%f\n", i, result, total_time);

    free(args_array);
    free(tid);
  }
}
