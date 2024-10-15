#include "myalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>

void *_arena_start = NULL;

int size_m = 0;

int myinit(size_t size){
    size_m = size;
    _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 );
    return _arena_start;
}


int mydestroy(){
    return munmap(_arena_start, size_m);
}


void* myalloc(size_t size){

}

void myfree(void *ptr){

}
