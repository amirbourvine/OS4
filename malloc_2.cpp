//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>
#include <cstddef>

#define MAX_SIZE 100000000

typedef struct MallocMetadata {
    size_t size; //does not include meta-data
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

typedef struct BlocksList {
    MallocMetadata* first = nullptr;
    unsigned int num_free_blocks = 0;
    unsigned int num_allocated_blocks = 0;
    unsigned int allocated_bytes = 0; //does not include meta-data
    unsigned int freed_bytes = 0; //does not include meta-data

    void insert(MallocMetadata* node);
} BlocksList;

void BlocksList::insert(MallocMetadata *node) {
    MallocMetadata* temp = this->first;
    while(temp!=nullptr){
        
        temp = temp->next;
    }
}

BlocksList* block_list;

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


//todo: copy good implementation
void* use_sbrk(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;
    void* ptr = sbrk(size);
    if(ptr == (void*)-1)
        return NULL;
    return ptr;
}

void* smalloc(size_t size){
    MallocMetadata* keep = find_block(size);
    if(keep!= nullptr){ //found a block
        keep->is_free = false;
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
            //todo: insert to blockList
            return (temp_res+1);
        }
    }
}









