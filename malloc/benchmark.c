#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "dlmall.h" 

void bench(int num_blocks, int min_size, int max_size){
    int interations = 1000;
    srand(time(NULL));   // Initialization, should only be called once.
    int *memlist[num_blocks];
    for (int i = 0; i < num_blocks; i++){
        memlist[i] = 0;
    }
    memlist[0] = dalloc(1);
    int num_malloc = 0;
    for (int i=0; i < interations; i++){
        for (int j=0; j<num_blocks; j++){
            if (memlist[j] == 0){
                memlist[j] = dalloc(min_size + (rand() % max_size-min_size));
            }
            if (sanity() == -1){
                return;
            }
        }
        int x = rand()%num_blocks;
        dfree(memlist[x]);
        memlist[x] = 0;
        if (sanity() == -1){
            return;
        }
    }
    for (int j=0; j<num_blocks; j++){
        if (memlist[j] == 0){
            memlist[j] = dalloc(min_size + (rand() % max_size-min_size));
        }
    }
    sanity();
    printf("%d\t", num_blocks);
    //print_arena();
    print_stats();
    return;
}

int main(int argc, char *argv[]) {
    int start = atoi(argv[1]);
    //int end = atoi(argv[2]);
    //for (int i = start; i<end; i++){
    bench(start, 1, 1000);
    //}
    return 1;
}

