void bench(int num_blocks){
    int min_size = 1;
    int max_size = 1000;
    int *malloc_list[num_blocks];

    int interations = 1000;
    for (int i=0; i < interations; i++){
        // use malloc for each index in list
        for (int j=0; j<num_blocks; j++){
            if (memlist[j] == 0)  // index is not allocated
                memlist[j] = dalloc(random(min_size, max_size)); // malloc random size
        }
        // free a random block from the list
        int x = random(0, num_blocks);
        dfree(memlist[x]);
        memlist[x] = 0;
    }
    return;
}
