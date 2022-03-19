#include <stdio.h>
#include "green.h"
#include <sys/time.h>
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
    int i = 3;
    while (i < x/2) {
        if (x % i == 0) {
          return 0;
        }
        i += 2;
    }
    return 1;
}

void *task(void *arg){
    int n = *(int*)arg;
    int res = 0; 
    for (int i=0; i<n; i++) {
        res += is_prime(i);
        //green_yield();
    }
    green_mutex_lock(&mutex);
    sum += res;
    green_mutex_unlock(&mutex);
}
    


int main() {
    green_mutex_init(&mutex);
    struct timeval start_time;
    struct timeval stop_time;
    for (int number_of_threads = 1; number_of_threads < 21; number_of_threads++) {
        gettimeofday(&start_time, NULL);
        green_t   g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13, g14, g15, g16, g17, g18, g19;
                    /*g20, g21, g22, g23, g24, g25, g26, g27, g28, g29, g30, g31, g32, g33, g34, g35, g36, g37, 
                    g38, g39, g40, g41, g42, g43, g44, g45, g46, g47, g48, g49, g50*/
        green_t threads[] = {
            g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11, g12, g13, g14, g15, g16, g17, g18, g19, 
            /*g20, g21, g22, g23, g24, g25, g26, g27, g28, g29, g30, g31, g32, g33, g34, g35, g36, g37, 
            g38, g39, g40, g41, g42, g43, g44, g45, g46, g47, g48, g49, g50*/
        };
        int args[20];
        for (int i = 0; i<number_of_threads; i++) {
            args[number_of_threads-i-1] = 50000;
        }
        for (int i=0; i<number_of_threads; i++) {
            green_create(&threads[i], task, &args[i]);
        }
        for(int i=0; i<number_of_threads; i++) {
            green_join(&threads[i], NULL);
            //printf("%d\n", i);
        }
        gettimeofday(&stop_time, NULL);
        long seconds = stop_time.tv_sec - start_time.tv_sec;
        long useconds = stop_time.tv_usec - start_time.tv_usec;
        double total = seconds + ((double)useconds)/1000000;
        printf("%d\t%f\n",number_of_threads, total);
    }
    return 0;
}

