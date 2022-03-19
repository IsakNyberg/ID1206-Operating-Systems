struct head *farray[4]; // 0-64     65-256      257-512     513+
int find_index(struct head *block){
    if (block->size <= 64){
        return 0;
    } else if (block->size <= 256){
        return 1;
    } else if (block->size <= 512){
        return 2;
    } else {
        return 3;
    }
}