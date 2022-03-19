#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define HEAD (sizeof(struct head))
#define MIN(size) (((size) > (8)?(size):(8)))
#define LIMIT(size) (MIN(0) + HEAD + size)
#define MAGIC(memory) ((struct head*)memory - 1)
#define HIDE(block) (void*)((struct head*)block + 1)
#define ALIGN 8
#define ARENA (64*1024)


struct head {
    uint16_t bfree; // bytes, the status of block before
    uint16_t bsize; // bytes, the size of block before
    uint16_t free; // 2 bytes, the status of the current block
    uint16_t size; // 2 bytes, the size (max 2^16 i.e. 64 Kibyte)
    struct head *next ; // 8 bytes pointer
    struct head *prev ; // 8 bytes pointer
};

struct head *after(struct head *block){
    return (struct head*)((char*)block + block->size + HEAD);
}

/* returns memory address to block before */
struct head *before(struct head *block){
    return (struct head*)((char*)block - block->bsize - HEAD);
}

struct head *arena = NULL;
struct head *arena_pointer;
struct head *new() {
    if(arena != NULL) {
        printf("one arena already allocated\n" );
        return NULL;
    }
    // using mmap, but could have used sbrk
    struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    arena_pointer = new;
    if (new == MAP_FAILED) {
        printf("mmap failed: error %d\n" , errno);
        return NULL;
    }
    /* make room for head and dummy */
    uint size = ARENA - 2*HEAD;
    new->bfree = FALSE;
    new->bsize = 0;
    new->free= TRUE;
    new->size = size;
    //new->prev = NULL;
    //new->next = NULL;
    struct head *sentinel = after(new);
   
    /* only touch the status fields */
    sentinel->bfree = new->free;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0;
    /* this is the only arena we have */
    arena = (struct head*) new;
    return new ;
}

struct head *split(struct head *block, int size) {
    // block is the block to split, size is the requested size of new block
    int rsize = block->size - HEAD - size;
    block->size = rsize;
    struct head *splt = HIDE(block) + rsize;
    splt->bsize = rsize;
    splt->bfree = block->free;
    splt->size = size;
    splt->free = TRUE;
    struct head *aft = after(splt);
    aft->bsize = size;
    return splt;
}

struct head *farray[4];  // 64 256 512
int find_index(struct head *block){
    if (block->size <= 64){ //&& farray[0] != NULL) {
        return 0;
    } else if (block->size <= 256) {
        return 1;
    } else if (block->size <= 512) {
        return 2;
    } else {
        return 3;
    }
}


void detach (struct head *block) {
    if(block->next != NULL){
        block->next->prev = block->prev;
    }
    if(block->prev != NULL){
        block->prev->next = block->next;
    } else {
        struct head *flist = farray[find_index(block)];
        farray[find_index(block)] = block->next;
    }
}

void insert(struct head *block){
    struct head *flist = farray[find_index(block)];
    block->next = flist;
    block->prev = NULL;
    if (flist != NULL){
        flist->prev = block;
    }
    farray[find_index(block)] = block;

}

int adjust(size_t size){
    int minsize = MIN(size);
    if(minsize % ALIGN == 0){
        return minsize;
    } else {
        int adj = minsize % ALIGN;
        return (minsize + ALIGN - adj);
    }
}

long search_counter = 0;
long maclloc_counter = 0;
struct head *find(size_t size){
    if (arena == NULL){
        farray[3] = new();
    }
    struct head *index;
    if (size <= 64 && farray[0] != NULL){
        index = farray[0];
    } else if (size <= 256 && farray[1] != NULL){ 
        index = farray[1];
    } else if (size <= 512 && farray[2] != NULL){ 
        index = farray[2];
    } else {
        index = farray[3];
    }
    if (index != NULL){
        maclloc_counter += 1;
    }
    while (index != NULL){  // iterate to end of list
        search_counter += 1; // stat
        if (index->size >= size) {  // block found
            detach(index);
            struct head *new_block;
            if (LIMIT(size) < index->size){  // try to split
                new_block = split(index, size);
                insert(index);
            } else {
                new_block = index;
            }
            new_block->free = FALSE;
            after(new_block)->bfree = FALSE;
            return new_block;
        } else {
            index = index->next;
        }
    }
    return NULL;
}


void *dalloc(size_t request) {
    if (request <= 0) {
        return NULL;
    }
    int size = adjust(request);
    struct head *taken = find(size);
    if (taken == NULL) {
        return NULL;
    } else {
        return HIDE(taken);
    }
}


struct head *merge(struct head *block){
    struct head *aft = after(block);
    struct head *bef = before(block);
    if(bef->free){ // merge up
        detach(bef);
        bef->size = bef->size + block->size + HEAD;
        aft->bsize = bef->size;
        block = bef;
    }

    if(aft->free){
        detach(aft);
        block->size = block->size + aft->size + HEAD;
        aft = after(block);
        aft->bsize = block->size;
    }
    return block;
}

void dfree(void *memory){
    if (memory != NULL) {
        struct head *block = (struct head*) MAGIC(memory);
        block  = merge(block);
        struct head *aft = after(block);
        block->free=TRUE;
        aft->bfree=TRUE;
        insert(block);
    }
    return;
}

void print_stats(){
    if (arena == NULL){
        printf("arena is null\n");
    } else {
        struct head *curr = arena;
        struct head *nxt = after(curr);
        int sum = 0;
        int counter = 0;
        int f_counter = 0;
        int f_sum = 0;
        int allocated_counter = 0;
        int allocated_sum= 0;
        while (curr->free == TRUE || curr->size != 0){
            if (curr->free){
                f_counter += 1;
                f_sum += curr->size;
            } else {
                allocated_counter += 1;
                allocated_sum += curr->size;
            }
            curr = nxt;
            nxt = after(curr);
            sum += curr->size;
            counter += 1;
        }
        //printf("%d\t", counter);
        //printf("%d\t", sum);
        //printf("%lu\t", counter*HEAD);
        //printf("%lu\t", 1000*sum/(counter*HEAD+sum+1));
        //printf("%d\t", allocated_counter);
        //printf("%d\t", allocated_sum);
        //printf("%lu\t", allocated_counter*HEAD);
        //printf("%.3f\t", ((float)(100*allocated_sum))/(allocated_counter*(HEAD-16)+allocated_sum+1));
        printf("%f", ((float)search_counter)/maclloc_counter);
        //printf(" search counter: %ld\t", search_counter);
        //printf(" malloc counter: %ld\t", maclloc_counter);
        //printf("%d\t", f_sum);
        //printf("%lu\t", f_counter*HEAD);
        //printf("%lu\t", 1000*f_sum/(f_counter*HEAD+f_sum+1));
        printf("\n");
        /*
        printf("Number of blocks: %d \n", counter);
        printf("\tBytes: %d \n", sum);
        printf("\tHeadr: %lu \n", counter*HEAD);
        printf("\teffic: %lu \n", 1000*sum/(counter*HEAD+sum));
        printf("Allocated %d \n", allocated_counter);
        printf("\tBytes: %d \n", allocated_sum);
        printf("\tHeadr: %lu \n", allocated_counter*HEAD);
        printf("\teffic: %lu \n", 1000*allocated_sum/(allocated_counter*HEAD+allocated_sum));
        printf("free %d \n", f_counter);
        printf("\tBytes: %d \n", f_sum);
        printf("\tHeadr: %lu \n", f_counter*HEAD);
        printf("\teffic: %lu \n", 1000*f_sum/(f_counter*HEAD+f_sum));*/
    }
}

void print_arena(int arg){
    if (arena == NULL){
        printf("arena is null\n");
    } else if ( arg != 2) {
        struct head *curr = arena;
        struct head *nxt = after(curr);
        int sum = 0;
        int counter = 0;
        int f_counter = 0;
        int allocated_counter = 0;
        while (curr->free == TRUE || curr->size != 0){
            if (curr->free){
                printf("|+\t");
                f_counter += 1;
            } else {
                printf("|- \t");
                allocated_counter += 1;
            }
            printf("%p\t", curr);
            printf("nxt: %11p\t", curr->next);
            printf("prv: %11p\t", curr->prev);
            printf("size: %4d\t", curr->size);
            printf("flist: %d\n", find_index(curr));
            curr = nxt;
            nxt = after(curr);
            sum += curr->size;
            counter += 1;
        }
        printf("number of blocks %d \n", counter);
        printf("\ttaken %d \n", allocated_counter);
        printf("\tfree %d \n", f_counter);
        printf("Area used: %d \n", sum);
        printf("Efficiency: %lu \n", 100*sum/(sum + HEAD*counter));
        printf("Leftover Area: %lu\n\n", ARENA-(sum + HEAD*counter));
    }

    for (int i = 0; i<4; i++){
        struct head *flist = farray[i];
        printf("flist ------- %d ------\n", i);
        if (flist == NULL){
            printf("flist is null %d %p\n\n", i, flist);
        } else if (arg != 1) {
            struct head *curr = flist;
            struct head *nxt = curr->next;
            int sum = 0;
            int counter = 0;
            printf("flist->");
            while (curr != NULL){
                printf("%d->", curr->size);
                counter += 1;
                sum += curr->size;
                curr = nxt;
                if (curr != NULL){
                    nxt = curr->next;
                }
            }
            printf("NULL\nnumber of free blocks %d \n", counter);
            printf("Area free: %d \n", sum);
            if (counter != 0){
                printf("average area_free: %d \n", sum/counter);
                printf("Efficiency free: %lu \n\n", 100*sum/(sum + HEAD*counter));
            } else{
                printf("average area_free: 0\n");
                printf("Efficiency free: 0\n\n");
            }
        }
    }
}

int sanity(){
    if (arena == NULL){
        printf("arena is null\n");
    } else {
        // check after sanity
        struct head *curr = arena;
        struct head *nxt = after(curr);
        while (nxt->free == TRUE || nxt->size != 0){
            if (curr->size != nxt->bsize){
                printf("curr size does not match nxt bsize");
                return -1;
            } 
            if (curr->free != nxt->bfree){
                printf("curr free does not match nxt bfree");
                return -1;
            }
            curr = nxt;
            nxt = after(nxt);
        }
    }
    for (int i = 0; i<4; i++){
        struct head *flist = farray[i];
        if (flist != NULL){
            struct head *curr = flist;
            struct head *nxt = curr->next;
            if (curr->prev != NULL){
                printf("flist element prev is not null %d curr size: %d prev size: %d", i, curr->size, curr->prev->size);
                return -1;
            }
            int counter = 0;
            while (nxt != NULL){
                if (nxt->prev != curr){
                    printf("nxt prev does not match curr");
                    return -1;
                }
                if (counter > 10000){
                    printf("possible loop found in flist");
                    return -1;
                }
                curr = nxt;
                nxt = curr->next;
                counter += 1;
            }
        }
    }
    return 0;
}
