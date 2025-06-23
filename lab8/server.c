#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "queue.h"


int main(int argc, char ** argv){
  if(argc < 2){
    printf("Usage: <number_of_clients>");
    return EXIT_FAILURE;
  }

  int N = atoi(argv[1]);

  printf("Cleaning up existing semaphores...\n");
  sem_unlink("semaphore_filled");
  sem_unlink("semaphore_empty");

  // Create shared memory for queue
  int fd;
  if((fd = shm_open("shared_mem", O_RDWR | O_CREAT, 0664)) == -1){
    perror("Could not create a shared memory block");
    return EXIT_FAILURE;
  }

  if(ftruncate(fd, sizeof(struct queue)) == -1){
    perror("Could not reserve space for queue");
    return EXIT_FAILURE;
  }

  struct queue * Q;
  if((Q = mmap(NULL, sizeof(struct queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
    perror("Could not map the shared memory");
    close(fd);
    return EXIT_FAILURE;
  }

  memset(Q, 0, sizeof(struct queue));

  sem_t *sem_fill;
  if((sem_fill = sem_open("semaphore_filled", O_RDWR | O_CREAT, 0664, 0)) == SEM_FAILED){
    perror("Could not create semaphore");
    return EXIT_FAILURE;
  }
  sem_t *sem_empty;
  if((sem_empty = sem_open("semaphore_empty", O_RDWR | O_CREAT, 0664, QUEUE_LEN)) == SEM_FAILED){
    perror("Could not create semaphore");
    return EXIT_FAILURE;
  }
  
  for(int i = 0; i < N; i++){
    int pid;
    if((pid = fork()) == -1){
      perror("Could not fork. Conitnuing...");
      continue;
    } else if(pid == 0){
      execl("./client", "client", NULL);
      usleep(1000*100);
    }
  }
  
}