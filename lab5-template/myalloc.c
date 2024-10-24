#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include "myalloc.h"
void *_arena_start = NULL;
node_t *header;
int size_M = 0;

int myinit(size_t size){
    if ((int) size < 0){
        size_M = ERR_BAD_ARGUMENTS;
    }
    else{
        int multiple = size/4096.0 > size/4096? (size/4096.0 + 1) : size/4096.0;
        size_M = (multiple)*4096;
        _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 );
    }

    return size_M;
}


int mydestroy(){
    if (_arena_start == NULL){
        size_M = ERR_UNINITIALIZED;
    }
    else{
        munmap(_arena_start, size_M);
        size_M = 0;
        _arena_start = NULL;
    }
    return size_M;
    
}


// void* myalloc(size_t size){
    
// }

// void myfree(void *ptr){

// }
