#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include "myalloc.h"
static void *_arena_start = NULL;
static node_t *free_mem = NULL;
static size_t current_arena_size = 0;
static size_t memory_size = 0;
static size_t init_size = 0;
int statusno;
int returnVal;
// #define ERR_OUT_OF_MEMORY  (-1)
// #define ERR_BAD_ARGUMENTS  (-2)
// #define ERR_SYSCALL_FAILED (-3)
// #define ERR_CALL_FAILED    (-4)
// #define ERR_UNINITIALIZED   (-5)

int myinit(size_t size){
    if (_arena_start != NULL || (int) size > MAX_ARENA_SIZE){
        returnVal = ERR_OUT_OF_MEMORY;
        statusno = ERR_OUT_OF_MEMORY;
    }
    else if ((int) size < 0){
        statusno = ERR_BAD_ARGUMENTS;
        returnVal = ERR_BAD_ARGUMENTS;
    }
    
    else{
        // Page Alignment
        _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if (_arena_start == NULL){
            statusno = ERR_SYSCALL_FAILED;
            returnVal = ERR_SYSCALL_FAILED;
        }
        int multiple = size/4096.0 > size/4096? (size/4096 + 1) : size/4096;
        free_mem = (node_t*) _arena_start;
        free_mem->is_free = 1;
        free_mem->size = size;
        free_mem->bwd = NULL;
        free_mem->fwd = NULL;
        statusno = 0; // Clear previous status
        current_arena_size = (multiple)*4096;
        init_size = size;
        returnVal = current_arena_size;
    }
    return returnVal;
}

int mydestroy(){
    if (_arena_start == NULL){
        returnVal = ERR_UNINITIALIZED;
        statusno = ERR_UNINITIALIZED;
    }
    else{
        munmap(_arena_start, returnVal);
        statusno = 0;
        returnVal = 0;
        current_arena_size = 0;
        memory_size = 0;
        init_size = 0;
        _arena_start = NULL;
        free_mem = NULL;
    }
    return returnVal;
}


void* myalloc(size_t size){
    // New chunks are made
    void *returnPtr = NULL;
    if (_arena_start == NULL){
        statusno = ERR_UNINITIALIZED;
    }
    else if (size < 0 || size > init_size){
        statusno = ERR_BAD_ARGUMENTS;
    }
    else if (memory_size + size > init_size || size + sizeof(node_t) > init_size){
        statusno = ERR_OUT_OF_MEMORY;
    }
    else{
        size_t unused = free_mem->size;
        memory_size += size + sizeof(node_t);
        statusno = 0;
        free_mem->is_free = 0;
        free_mem->size = size;
        free_mem->fwd = NULL;
        if(size < unused){
            node_t *newChunk = (node_t*) ((void*) free_mem + sizeof(node_t) + size);
            free_mem->fwd = newChunk;
            newChunk->bwd = free_mem; 
            newChunk->size = init_size - memory_size - sizeof(node_t);
            newChunk->is_free = 1;
            newChunk->fwd = NULL;
            returnPtr = (void*)free_mem + sizeof(node_t);
            free_mem = newChunk;
            return returnPtr;
        }
        return (void*)free_mem + sizeof(node_t);
    }
    return returnPtr;
}

void myfree(void *ptr){
    if (ptr == NULL) {
        statusno = ERR_BAD_ARGUMENTS;
    }
    else{
        // No coalescing added for backward or forward headers
        node_t *chunk = (node_t *)(ptr - sizeof(node_t)); 
        chunk->is_free = 1;
    }
}
