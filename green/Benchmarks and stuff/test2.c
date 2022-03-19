#include <stdio.h>
#include "green.h"

int flag = 0;
struct green_cond_t cond;

int A(int x) {
    x = 1<<x;
    while(x > 0){
        x--;
    }

}

void *test(void *arg){
    int i = *(int*)arg;
    int loop = 10;
    while(loop > 0){
        printf("thread %d: %d\n", i, loop);
        A(21);
        loop--;
    }
}
void *foo(void *arg) {
    int i = *(int*)arg;
    int loop = 10;
    while(loop > 0){
        printf("thread %d: %d\n", i, loop);
        A(24);
        loop--;
    }
}

void *test(void *arg){
    int id = *(int*)arg;
    int loop = 10;
    while(loop > 0){
        green_cond_signal(&cond);
        //green_mutex_lock(&mutex);
        flag++;
        printf("%d, %d, %d\n", id, loop, flag);
        //green_mutex_unlock(&mutex);
        green_cond_wait(&cond, &mutex);
        A(1<<29+id);
        loop--;
    }
    green_mutex_unlock(&mutex);
    green_cond_signal(&cond);
}


int main() {
    green_t g0;
    green_t g1;
    int a0 = 0;
    int a1 = 1;
    green_cond_init(&cond);
    green_create(&g0, foo, &a0);
    green_create(&g1, test, &a1);
    green_join(&g0, NULL);
    green_join(&g1, NULL);
    printf("main: Done \n");
    return 0;
}

