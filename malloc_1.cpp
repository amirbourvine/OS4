//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>

#include "header_1.h"
#define MAX_SIZE 100000000

void* smalloc(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;

    void* ptr = sbrk(size);

    if(ptr == (void*)-1)
        return NULL;

    return ptr;
}