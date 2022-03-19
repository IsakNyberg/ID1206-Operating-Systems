#include <stdio.h>
#include "green.h"
#include <time.h>

volatile double time_sum = 0;
volatile int sum = 0;
struct green_cond_t cond;
struct green_mutex_t mutex;

void A(int x) {
    while (x > 0) {
        x--;
    }
}


int is_prime(int x) {
    int is_prime = 1;
    int i = 3;
    while (i < x/2) {
        if (x % i == 0) {
          is_prime = 0;
          return 0;
        }
        i += 2;
    }
    return 1;
}

void *task(void *arg){
    int n = *(int*)arg;
    int res = 0;
    clock_t t;
    t = clock();
    for (int i=0; i<n; i++) {
        if (is_prime(i) && n%100 == 1)
            printf("%d\t%f\n", i, ((double)clock() - t)/CLOCKS_PER_SEC);
        green_yield();   
    }
}
    



int main() {
    int number_of_threads = 10;
    green_t g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13, g14, g15, g16, g17, g18, g19;
    green_t threads[] = {
        g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10,
        g11, g12, g13, g14, g15, g16, g17, g18, g19
    };
    int args[20];
    for (int i = 0; i<number_of_threads; i++) {
        args[number_of_threads-i-1] = 100000+i;
    }
    green_cond_init(&cond);
    green_mutex_init(&mutex);
    
    for (int i=0; i<number_of_threads; i++) {
        green_create(&threads[i], task, &args[i]);
    }
    for(int i=0; i<number_of_threads; i++) {
        green_join(&threads[i], NULL);
    }
    //printf("%d\t%f\n",number_of_threads, time_sum);
    return 0;
}

