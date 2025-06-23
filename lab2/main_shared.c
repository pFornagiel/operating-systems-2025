#include "collatz.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>



int main(){
    srand(time(NULL));
    int numOfTries = 5;
    int numSteps = 50;

    
    for(int i = 0; i< numOfTries; i++){
        int input = rand() % 1000;
        int result[50];
        int count = test_collatz_convergence(input, numSteps, result);
        printf("[");
        for(int j = 0; j< count; j++){
            if(j != 0){
                printf(", ");
            }
            printf("%d", result[j]);
        }
        printf("]\n");
    }
    return 0;
}