//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <math.h>

#include "tests/header_2.h"

#define MAX_SIZE (100000000)
#define NUM_ORDERS (11)
#define INITIAL_BLOCK_SIZE (128*1024)
#define INITIAL_BLOCKS_NUM (32)
#define ALIGN (128*1024*32)
#define MIN_BLOCK_SIZE (128)

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


    FreeBlocksManager();

    void insert(MallocMetadata* to_insert);
    MallocMetadata* find(size_t size);//finds best fit for size 'size'
    void remove(MallocMetadata* to_remove);
} FreeBlocksManager;

int size_to_ord(size_t size){
    //gets size of block (w\o meta-data)
    return (int)ceil(log2((size + sizeof(MallocMetadata))/128));
}

void* use_sbrk(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;

    void* ptr = sbrk(size);

    if(ptr == (void*)-1)
        return NULL;

    return ptr;
}

BlocksList* block_list = new BlocksList();

FreeBlocksManager::FreeBlocksManager() {
    for(int i =0;i<NUM_ORDERS-1; i++){
        this->lists[i] = nullptr;
    }

    //first sbrk() to align
    intptr_t curr_addr = (intptr_t)use_sbrk(0);
    size_t size = ((int)ceil(curr_addr/ALIGN))*ALIGN-curr_addr;
    use_sbrk(size);


    void* start = use_sbrk(INITIAL_BLOCK_SIZE*INITIAL_BLOCKS_NUM);
    char* curr;

    MallocMetadata* tmp = (MallocMetadata*)start;
    tmp->is_free = true;
    tmp->size = INITIAL_BLOCK_SIZE-sizeof(MallocMetadata);
    tmp->prev = nullptr;

    MallocMetadata* keep = tmp;

    this->lists[NUM_ORDERS-1] = tmp;

    for(int i = 1; i<INITIAL_BLOCKS_NUM; i++){
        curr = (char*)tmp;
        curr += INITIAL_BLOCK_SIZE;
        tmp = (MallocMetadata*)curr;

        tmp->is_free = true;
        tmp->size = INITIAL_BLOCK_SIZE-sizeof(MallocMetadata);
        tmp->prev = keep;
        keep->next = tmp;

        keep = tmp;
    }


    block_list->num_allocated_blocks = INITIAL_BLOCKS_NUM;
    block_list->allocated_bytes = INITIAL_BLOCKS_NUM*(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));
    block_list->num_free_blocks = INITIAL_BLOCKS_NUM;
    block_list->freed_bytes = INITIAL_BLOCKS_NUM*(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));

}

FreeBlocksManager* free_block_manager = new FreeBlocksManager();

MallocMetadata *FreeBlocksManager::find(size_t size) {
    int ord_start = size_to_ord(size);

    while(ord_start<=10){
        if(free_block_manager->lists[ord_start]!= nullptr){
            return free_block_manager->lists[ord_start];
        }
        ord_start++;
    }
    return nullptr;
}

void FreeBlocksManager::remove(MallocMetadata *to_remove) {

    to_remove->is_free = false;
    --block_list->num_free_blocks;
    block_list->freed_bytes-=to_remove->size;

    int ord = size_to_ord(to_remove->size);

    if(to_remove->prev == nullptr){//first
        free_block_manager->lists[ord] = to_remove->next;
        to_remove->next->prev = nullptr;
        return;
    }
    if(to_remove->next == nullptr){//last
        to_remove->prev->next = nullptr;
        return;
    }
    MallocMetadata *before = to_remove->prev;
    MallocMetadata *after = to_remove->next;
    before->next = after;
    after->prev = before;
}

void FreeBlocksManager::insert(MallocMetadata *to_insert) {

    to_insert->is_free = true;
    ++block_list->num_free_blocks;
    block_list->freed_bytes+=to_insert->size;

    int ord = size_to_ord(to_insert->size);
    std::cout << "ord: "<< ord << std::endl;
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
    MallocMetadata* temp;
    std::cout << "***********************" << std::endl;
    for(int i = 0; i<NUM_ORDERS; i++) {
        temp = free_block_manager->lists[i];
        bool flag = (temp!= nullptr);
        if(flag){
            std::cout << "# i: " << i << " #" << std::endl;
        }
        while (temp != nullptr) {
            std::cout << "  SIZE: " << temp->size;
            temp = temp->next;
        }
        if(flag){
            std::cout << std::endl;
        }
    }
    std::cout << "# num_free_blocks: " << block_list->num_free_blocks << " #" << std::endl;
    std::cout << "# num_allocated_blocks: " << block_list->num_allocated_blocks << " #" << std::endl;
    std::cout << "# allocated_bytes: " << block_list->allocated_bytes << " #" << std::endl;
    std::cout << "# freed_bytes: " << block_list->freed_bytes << " #" << std::endl;
    std::cout << "***********************" << std::endl;
}


MallocMetadata* break_block_down(MallocMetadata* init, size_t size){
    MallocMetadata* tmp = init;
    MallocMetadata* new_curr;
    char* curr;
    free_block_manager -> remove(init);
    std::cout << "init->size: "<< init->size << std::endl;
    while((((tmp->size - sizeof(MallocMetadata))/2)>=MIN_BLOCK_SIZE)&&((tmp->size - sizeof(MallocMetadata))/2 >= size)){
        std::cout << "HERE-GOOD" << std::endl;
        tmp->size = (tmp->size - sizeof(MallocMetadata))/2;
        curr = (char*)tmp;
        curr += (tmp->size - sizeof(MallocMetadata))/2;
        new_curr = (MallocMetadata*)curr;
        new_curr->size = (tmp->size - sizeof(MallocMetadata))/2;
        std::cout << "new_curr->size: "<< new_curr->size << std::endl;
        free_block_manager->insert(new_curr);
    }
    return tmp;
}


void* smalloc(size_t size){
    if(size==0 or size>MAX_SIZE)
        return NULL;
    MallocMetadata* keep = free_block_manager->find(size);
    if(keep!= nullptr){ //found a block
        keep = break_block_down(keep, size);
        return (keep+1);
    }
    else{ // did not find a block
        perror("BLOCK NOT FOUND");
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












