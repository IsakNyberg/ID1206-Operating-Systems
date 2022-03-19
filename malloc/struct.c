struct head {       // 24 bytes
    uint16_t bfree; // 2 bytes, the status of block before
    uint16_t bsize; // 2 bytes, the size of block before
    uint16_t free;  // 2 bytes, the status of the current block
    uint16_t size;  // 2 bytes, the size of the current block
    struct head *next; // 8 bytes pointer to next block in list
    struct head *prev; // 8 bytes pointer to prev block in list
};
