#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>


// RTLD_LAZY is a flag used with dlopen() when loading a shared library. It stands for "resolve symbols lazily". This means that function symbols (like my_library_function) are not resolved immediately when the library is loaded but instead are resolved only when they are used (first called).
#ifdef DYNAMICVARIANT
    #define LIB_NAME "./libcollatz.so"
    #define FUNC_NAME "test_collatz_convergence"
#else 
    #error "DYNAMICVARIANT not defined"
#endif

int main(){
    void *handle = dlopen(LIB_NAME, RTLD_LAZY);
    if(!handle){
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // looks for the symbol "test_collatz_convergence" inside the shared library
    // If the function is found, returns a memory address where the function is located
    // dlsym() always returns void* (generic pointer to data),
    void *sym = dlsym(handle, FUNC_NAME);
    // function pointer declaration
    int (*test_collatz_convergence)(int , int , int *);
    // &test_collatz_convergence gets the address of function pointer variable
    // (void **) casts that address to a void** (pointer to void*)
    // assigns the void* sym to the function pointer variable
    *(void **)(&test_collatz_convergence) = sym;

    if(dlerror() != NULL){
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    int numOfTries = 5;
    int numSteps = 50;
    
    for(int i = 0; i< numOfTries; i++){
        int input = rand() % 1000;
        int result[50];
        int count = (*test_collatz_convergence)(input, numSteps, result);
        printf("[");
        for(int j = 0; j< count; j++){
            if(j != 0){
                printf(", ");
            }
            printf("%d", result[j]);
        }
        printf("]\n");
    }
    dlclose(handle);
    return 0;
}