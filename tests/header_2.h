//
// Created by Elad on 26/06/2023.
//

#ifndef OS4_HEADER_2_H
#define OS4_HEADER_2_H
#include <unistd.h>

//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>
#include <iostream>

#include "header_2.h"
#define MAX_SIZE 100000000

void print();

void* smalloc(size_t size);

size_t _num_free_blocks();

size_t _num_free_bytes();

size_t _num_allocated_blocks();

size_t _num_allocated_bytes();

size_t _num_meta_data_bytes();

size_t _size_meta_data();

void* scalloc(size_t num, size_t size);

void sfree(void* p);

void* srealloc(void* oldp, size_t size);


#endif //OS4_HEADER_2_H
