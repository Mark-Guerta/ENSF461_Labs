#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include "myalloc.h"
void *_arena_start = NULL;
int size_M = 0;
int num_of_chunks = 0;
int memUsage = 0;
int statusno;
node_t *head = NULL;
int myinit(size_t size){
    if ((int) size < 0){
        size_M = ERR_BAD_ARGUMENTS;
    }
    else{
        int multiple = size/4096.0 > size/4096? (size/4096 + 1) : size/4096;
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


void* myalloc(size_t size){
    void *ptr = NULL;
    if (_arena_start == NULL)
        statusno = ERR_UNINITIALIZED;
    else if ((int) size == size_M)
        statusno = ERR_OUT_OF_MEMORY;
    else{
        node_t *header = (node_t *) _arena_start;
        if (num_of_chunks > 0){
            if (size_M < memUsage + size){
                statusno = ERR_OUT_OF_MEMORY;
            }
            // Implement adding new headers
            num_of_chunks++;
        }
        else{
            header->size = size;
            header->is_free = 0;
            header->fwd = NULL;
            header->bwd = NULL;
            memUsage += size + sizeof(header);
            num_of_chunks++;
            ptr = _arena_start + sizeof(header);
        }
    }
    return ptr;
}

// void myfree(void *ptr){

// }
