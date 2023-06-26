//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <math.h>

#include "tests/header_2.h"
#define MAX_SIZE 100000000
#define NUM_ORDERS 11

typedef struct MallocMetadata {
    size_t size; //does not include meta-data
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

typedef struct BlocksList {
    MallocMetadata* first = nullptr;
    unsigned int num_free_blocks = 0;//number of allocated blocks in the heap that are currently free
    unsigned int num_allocated_blocks = 0;//overall (free and used) number of allocated blocks in the heap
    unsigned int allocated_bytes = 0; // overall number (free and used) of allocated bytes in the heap, does not include meta-data
    unsigned int freed_bytes = 0; //number of bytes in all allocated blocks in the heap that are currently free, does not include meta-data

    void insert(MallocMetadata* to_insert);
} BlocksList;

void BlocksList::insert(MallocMetadata *to_insert) {
    MallocMetadata* temp = this->first;
    if(temp == nullptr){
        this->first = to_insert;
        this->first->next = nullptr;
        this->first->prev = nullptr;

        return;
    }
    while(temp!=nullptr && to_insert > temp){
        temp = temp->next;
    }
    if(temp== nullptr){//insert last
        MallocMetadata* last = this->first;
        while(last->next!= nullptr){ last = last->next; }
        last->next = to_insert;
        to_insert->next = nullptr;
        to_insert->prev = last;
        return;
    }
    if(temp==this->first){
        this->first = to_insert;
        to_insert->next = temp;
        to_insert->prev = nullptr;
        temp->prev = to_insert;
        return;
    }

    //regular insertion
    MallocMetadata* insert_after = temp->prev;
    to_insert->next = temp;
    to_insert->prev = insert_after;
    temp->prev = to_insert;
    insert_after->next = to_insert;
}

typedef struct FreeBlocksManager {
    MallocMetadata* lists[NUM_ORDERS];
    //todo: implement constructor

    void insert(MallocMetadata* to_insert);
    MallocMetadata* find(size_t size);//finds best fit for size 'size'
    void remove(MallocMetadata* to_remove);
} FreeBlocksManager;

int size_to_ord(size_t size){
    return (int)ceil(log2((size+ sizeof(MallocMetadata))/128));
}

FreeBlocksManager::

BlocksList* block_list = new BlocksList();
FreeBlocksManager* free_block_manager = new FreeBlocksManager();

void FreeBlocksManager::insert(MallocMetadata *to_insert) {
    int ord = size_to_ord(to_insert->size);
    MallocMetadata* first = free_block_manager->lists[ord];

    MallocMetadata* temp = first;
    if(temp == nullptr){
        free_block_manager->lists[ord] = to_insert;
        to_insert->next = nullptr;
        to_insert->prev = nullptr;
        return;
    }
    while(temp!=nullptr && to_insert > temp){
        temp = temp->next;
    }
    if(temp == nullptr){//insert last
        MallocMetadata* last = first;
        while(last->next!= nullptr){ last = last->next; }
        last->next = to_insert;
        to_insert->next = nullptr;
        to_insert->prev = last;
        return;
    }
    if(temp==first){
        free_block_manager->lists[ord] = to_insert;
        to_insert->next = temp;
        to_insert->prev = nullptr;
        temp->prev = to_insert;
        return;
    }

    //regular insertion
    MallocMetadata* insert_after = temp->prev;
    to_insert->next = temp;
    to_insert->prev = insert_after;
    temp->prev = to_insert;
    insert_after->next = to_insert;
}

void print()  {
    MallocMetadata* temp = block_list->first;
    std::cout << "***********************" << std::endl;
    while(temp!=nullptr){
        std::cout << "# SIZE: " << temp->size << " #" << std::endl;
        temp = temp->next;
    }
    std::cout << "# num_free_blocks: " << block_list->num_free_blocks << " #" << std::endl;
    std::cout << "# num_allocated_blocks: " << block_list->num_allocated_blocks << " #" << std::endl;
    std::cout << "# allocated_bytes: " << block_list->allocated_bytes << " #" << std::endl;
    std::cout << "# freed_bytes: " << block_list->freed_bytes << " #" << std::endl;
    std::cout << "***********************" << std::endl;
}

MallocMetadata* find_block(size_t size){
    MallocMetadata* keep = nullptr;
    MallocMetadata* temp = block_list->first;
    while(temp!=nullptr){
        if((temp->is_free) && (temp->size>=size)){
            keep = temp;
            break;
        }
        temp = temp->next;
    }
    return keep;
}


void* use_sbrk(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;

    void* ptr = sbrk(size);

    if(ptr == (void*)-1)
        return NULL;

    return ptr;
}

void* smalloc(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;
    MallocMetadata* keep = find_block(size);
    if(keep!= nullptr){ //found a block
        keep->is_free = false;
        block_list->num_free_blocks -= 1;
        block_list->freed_bytes -= keep->size; //does not include meta-data
        return (keep+1);
    }
    else{ // did not find a block
        void* new_block = use_sbrk(size + sizeof(MallocMetadata));
        if(new_block==NULL) {
            return NULL;
        }
        else{
            MallocMetadata* temp_res = (MallocMetadata*)new_block;
            temp_res->is_free = false;
            temp_res->size = size;
            block_list->insert(temp_res);
            block_list->num_allocated_blocks += 1;
            block_list->allocated_bytes += temp_res->size; //does not include meta-data

            return (temp_res+1);
        }
    }
}

size_t _num_free_blocks(){
    return block_list->num_free_blocks;
}

size_t _num_free_bytes(){
    return block_list->freed_bytes;
}

size_t _num_allocated_blocks(){
    return block_list->num_allocated_blocks;
}

size_t _num_allocated_bytes() {
    return block_list->allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return (block_list->num_allocated_blocks*sizeof(MallocMetadata));
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}


void* scalloc(size_t num, size_t size){
    void* allocated_block = smalloc(num * size);

    if(allocated_block == NULL)
        return NULL;

    memset (allocated_block, 0, num * size);

    return allocated_block;
}

void sfree(void* p){
    if(p == NULL)
        return;

    MallocMetadata* block = (MallocMetadata*)(p);
    block -= 1;

    if(!block->is_free) {
        //Free Buddies
        while(size_to_ord(block->size) <= NUM_ORDERS - 1){
            free_block_manager->insert(block);

            MallocMetadata* buddy = (MallocMetadata*)(((size_t)block) ^ (block->size + sizeof(MallocMetadata)));
            if(!buddy->is_free){
                break;
            }

            //Buddy is also free
            free_block_manager->remove(block);
            free_block_manager->remove(buddy);

            block->size = block->size * 2 + sizeof(MallocMetadata);
        }
    }
}

void* srealloc(void* oldp, size_t size) {
    if(oldp == NULL){
        return smalloc(size);
    }

    MallocMetadata *block = (MallocMetadata*)(oldp);
    block -= 1;

    if(block->size >= size){
        if(block->is_free){
            block->is_free = false;
            --block_list->num_free_blocks;
            block_list->freed_bytes -= block->size;
        }

        return oldp;
    }

    void* allocated_block = smalloc(size);

    if(allocated_block == NULL)
        return NULL;

    memmove(allocated_block, oldp, size);
    sfree(oldp);

    return allocated_block;
}












