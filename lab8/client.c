#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "queue.h"
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>


int random_int(int min, int max) {
  return min + (rand() % (max - min + 1));
}

// fixed-size memory block syntax
char * get_random_word(char (*word)[MSG_LEN]){  
  for(int i = 0; i< MSG_LEN-1;i++){
    (*word)[i] = random_int('a', 'z');
  }
  (*word)[MSG_LEN-1] = '\0';
  return *word;
}

// #define RAND_MAX 10
int main(){
  srand(time(NULL) ^ getpid());   
  int r = random_int(1,10);
  char word[MSG_LEN];
  get_random_word(&word);

  sem_t *sem_fill;
  if((sem_fill = sem_open("semaphore_filled", O_RDWR)) == SEM_FAILED){
    perror("Could not open semaphore");
    return EXIT_FAILURE;
  }
  sem_t *sem_empty;
  if((sem_empty = sem_open("semaphore_empty", O_RDWR)) == SEM_FAILED){
    perror("Could not open semaphore");
    return EXIT_FAILURE;
  }

  int fd = shm_open("shared_mem", O_RDWR, 0444);
  if(fd == -1){
    perror("shm_open");
    return EXIT_FAILURE;
  }

  struct queue * Q;
  if((Q = mmap(NULL, sizeof(struct queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
    perror("Could not map the shared memory");
    close(fd);
    return EXIT_FAILURE;
  }

  while(1){
    
    if(sem_trywait(sem_empty) != -1) {
      printf("Sending message to a queue, pid=%d\n", getpid());
      printf("MESSAGE: %s\n", word);
      push(Q, word);
      sem_post(sem_fill);
    } else if(errno == EAGAIN) {
      printf("Cannot send message to a queue! pid=%d\n", getpid());
    } else {
      perror("Could not access the queue");
    }
    sleep(r);
  }

  munmap(Q, sizeof(struct queue));
  close(fd);
  return 0;

}