#include <stdio.h>
#include "green.h"

void *test(void *arg) {
    int i = *(int*)arg;
    int loop = 4;
    while(loop > 0) {
        //printf("thread %d: %d\n", i, loop);
        loop -= 1;
        green_yield();
    }
    printf("thread %d: done\n", i);
}

void *foo(void *arg) {
    int i = *(int*)arg;
    int loop = 40;
    while(loop > 0) {
        //printf("thread %d: %d\n", i, loop);
        loop -= 1;
        green_yield();
    }
    printf("thread %d: done\n", i);
}

int main() {
    green_t g0, g1;
    int a0 = 0;
    int a1 = 1;

    //printf("main: 1\n");
    green_create(&g0, test, &a0);
    //printf("main: 2\n");
    green_create(&g1, test, &a1);
    //printf("main: 3\n");
    green_join(&g0, NULL);
    //printf("main: 4\n");
    green_join(&g1, NULL);
    //printf("main: 5\n\n\n");
    return 0;
}

