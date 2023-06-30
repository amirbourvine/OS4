//
// Created by amirb on 26/06/2023.
//
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <math.h>
#include <ctime>
#include <cstdlib>
#include <sys/mman.h>

//#include "my_tests/header_2.h"

#define MAX_SIZE (100000000)
#define NUM_ORDERS (11)
#define INITIAL_BLOCK_SIZE (128*1024)
#define INITIAL_BLOCKS_NUM (32)
#define ALIGN (128*1024*32)
#define MIN_BLOCK_SIZE (128)

size_t global_cookie = 0;

typedef struct MallocMetadata {
private:
    size_t cookie;
    size_t size; //does not include meta-data
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;

    void check_cookie() const{
        if(this->cookie != global_cookie){
            exit(0xdeadbeef);
        }
    }

public:
    size_t get_size() const{
        this->check_cookie();
        return this->size;
    }
    void set_size(size_t size){
        this->check_cookie();
        this->size = size;
    }

    bool get_is_free() const{
        this->check_cookie();
        return this->is_free;
    }
    void set_is_free(size_t is_free){
        this->check_cookie();
        this->is_free = is_free;
    }

    MallocMetadata* get_prev() const{
        this->check_cookie();
        return this->prev;
    }
    void set_prev(MallocMetadata* prev){
        this->check_cookie();
        this->prev = prev;
    }

    MallocMetadata* get_next() const{
        this->check_cookie();
        return this->next;
    }
    void set_next(MallocMetadata* next){
        this->check_cookie();
        this->next = next;
    }

    void set_cookie(){
        this->cookie = global_cookie;
    }
} MallocMetadata;

typedef struct FreeBlocksManager {
    MallocMetadata* lists[NUM_ORDERS];

    FreeBlocksManager();

    void insert(MallocMetadata* to_insert);
    MallocMetadata* find(size_t size);//finds best fit for size 'size'
    void remove(MallocMetadata* to_remove);
} FreeBlocksManager;

typedef struct BlocksList {
    MallocMetadata* first = nullptr;
    unsigned int num_free_blocks = 0;//number of allocated blocks in the heap that are currently free
    unsigned int num_allocated_blocks = 0;//overall (free and used) number of allocated blocks in the heap
    unsigned int allocated_bytes = 0; // overall number (free and used) of allocated bytes in the heap, does not include meta-data
    unsigned int freed_bytes = 0; //number of bytes in all allocated blocks in the heap that are currently free, does not include meta-data

    bool is_first = true;
    FreeBlocksManager* free_block_manager;

    BlocksList(){
        srand(time(0));
        global_cookie = rand() % sizeof(SIZE_MAX);
    }

    void insert(MallocMetadata* to_insert);
} BlocksList;

void BlocksList::insert(MallocMetadata *to_insert) {
    MallocMetadata* temp = this->first;
    if(temp == nullptr){
        this->first = to_insert;
        this->first->set_next(nullptr);
        this->first->set_prev(nullptr);

        return;
    }
    while(temp!=nullptr && to_insert > temp){
        temp = temp->get_next();
    }
    if(temp== nullptr){//insert last
        MallocMetadata* last = this->first;
        while(last->get_next() != nullptr){ last = last->get_next(); }
        last->set_next(to_insert);
        to_insert->set_next(nullptr);
        to_insert->set_prev(last);
        return;
    }
    if(temp==this->first){
        this->first = to_insert;
        to_insert->set_next(temp);
        to_insert->set_prev(nullptr);
        temp->set_prev(to_insert);
        return;
    }

    //regular insertion
    MallocMetadata* insert_after = temp->get_prev();
    to_insert->set_next(temp);
    to_insert->set_prev(insert_after);
    temp->set_prev(to_insert);
    insert_after->set_next(to_insert);
}

int size_to_ord(size_t size){
    //gets size of block (w\o meta-data)
    if(size + sizeof(MallocMetadata) < MIN_BLOCK_SIZE){
        return 0;
    }
    return (int)ceil(log2((double)(size + sizeof(MallocMetadata))/128));
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
    intptr_t curr_addr = (intptr_t)sbrk(0);

    size_t tmp1 = ((int)ceil((double)curr_addr/ALIGN));
    size_t tmp2 = tmp1*ALIGN;
    size_t size = tmp2-curr_addr;
    use_sbrk(size);


    void* start = use_sbrk(INITIAL_BLOCK_SIZE*INITIAL_BLOCKS_NUM);

    char* curr;

    MallocMetadata* tmp = (MallocMetadata*)start;

    tmp->set_cookie();
    tmp->set_is_free(true);
    tmp->set_size(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));
    tmp->set_prev(nullptr);

    MallocMetadata* keep = tmp;

    this->lists[NUM_ORDERS-1] = tmp;

    for(int i = 1; i<INITIAL_BLOCKS_NUM; i++){
        curr = (char*)tmp;
        curr += INITIAL_BLOCK_SIZE;
        tmp = (MallocMetadata*)curr;

        tmp->set_cookie();
        tmp->set_is_free(true);
        tmp->set_size(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));
        tmp->set_prev(keep);
        keep->set_next(tmp);

        keep = tmp;
    }

    block_list->num_allocated_blocks = INITIAL_BLOCKS_NUM;
    block_list->allocated_bytes = INITIAL_BLOCKS_NUM*(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));
    block_list->num_free_blocks = INITIAL_BLOCKS_NUM;
    block_list->freed_bytes = INITIAL_BLOCKS_NUM*(INITIAL_BLOCK_SIZE-sizeof(MallocMetadata));

}


MallocMetadata *FreeBlocksManager::find(size_t size) {
    int ord_start = size_to_ord(size);
    while(ord_start<=10){
        if(block_list->free_block_manager->lists[ord_start]!= nullptr){
            return block_list->free_block_manager->lists[ord_start];
        }
        ord_start++;
    }
    return nullptr;
}

void FreeBlocksManager::remove(MallocMetadata *to_remove) {
    to_remove->set_is_free(false);
    --block_list->num_free_blocks;
    block_list->freed_bytes-=to_remove->get_size();

    int ord = size_to_ord(to_remove->get_size());

    if(to_remove->get_prev() == nullptr && to_remove->get_next() == nullptr) {//only
        block_list->free_block_manager->lists[ord] = nullptr;
        return;
    }

    if(to_remove->get_prev() == nullptr){//first
        block_list->free_block_manager->lists[ord] = to_remove->get_next();
        to_remove->get_next()->set_prev(nullptr);
        return;
    }
    if(to_remove->get_next() == nullptr){//last
        to_remove->get_prev()->set_next(nullptr);
        return;
    }
    MallocMetadata *before = to_remove->get_prev();
    MallocMetadata *after = to_remove->get_next();
    before->set_next(after);
    after->set_prev(before);
}

void FreeBlocksManager::insert(MallocMetadata *to_insert) {

    to_insert->set_is_free(true);
    ++block_list->num_free_blocks;
    block_list->freed_bytes+=to_insert->get_size();

    int ord = size_to_ord(to_insert->get_size());
    MallocMetadata* first = block_list->free_block_manager->lists[ord];

    MallocMetadata* temp = first;
    if(temp == nullptr){
        block_list->free_block_manager->lists[ord] = to_insert;
        to_insert->set_next(nullptr);
        to_insert->set_prev(nullptr);
        return;
    }
    while(temp!=nullptr && to_insert > temp){
        temp = temp->get_next();
    }
    if(temp == nullptr){//insert last
        MallocMetadata* last = first;
        while(last->get_next() != nullptr){ last = last->get_next(); }
        last->set_next(to_insert);
        to_insert->set_next(nullptr);
        to_insert->set_prev(last);
        return;
    }
    if(temp==first){
        block_list->free_block_manager->lists[ord] = to_insert;
        to_insert->set_next(temp);
        to_insert->set_prev(nullptr);
        temp->set_prev(to_insert);
        return;
    }

    //regular insertion
    MallocMetadata* insert_after = temp->get_prev();
    to_insert->set_next(temp);
    to_insert->set_prev(insert_after);
    temp->set_prev(to_insert);
    insert_after->set_next(to_insert);
}

void print()  {
    MallocMetadata* temp;
    if(block_list->free_block_manager== nullptr){
        std::cout << "free_block_manager is uninitialized!" << std::endl;
        return;
    }
    std::cout << "***********************" << std::endl;
    for(int i = 0; i<NUM_ORDERS; i++) {
        temp = block_list->free_block_manager->lists[i];
        bool flag = (temp!= nullptr);
        if(flag){
            std::cout << "# i: " << i << " #" << std::endl;
        }
        while (temp != nullptr) {
            std::cout << "  SIZE: " << temp->get_size();
            temp = temp->get_next();
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
    block_list->free_block_manager -> remove(init);
    while((((tmp->get_size() - sizeof(MallocMetadata))/2)>=(MIN_BLOCK_SIZE- sizeof(MallocMetadata)))&&((tmp->get_size() - sizeof(MallocMetadata))/2 >= size)){
        size_t new_size = (tmp->get_size() - sizeof(MallocMetadata))/2;
        tmp->set_size(new_size);
        curr = (char*)tmp;
        curr += new_size + sizeof(MallocMetadata);
        new_curr = (MallocMetadata*)curr;
        new_curr->set_cookie();
        new_curr->set_size(new_size);
        ++block_list->num_allocated_blocks;
        block_list->allocated_bytes-=sizeof(MallocMetadata);
        block_list->free_block_manager->insert(new_curr);
    }
    return tmp;
}


void* smalloc(size_t size){
    if(block_list->is_first){
        block_list->is_first = false;
        block_list->free_block_manager = new FreeBlocksManager();
    }
    if(size==0 or size>MAX_SIZE)
        return NULL;
    MallocMetadata* keep = block_list->free_block_manager->find(size);
    if(keep!= nullptr){ //found a block
        keep = break_block_down(keep, size);
        return (keep+1);
    }
    else{ // did not find a block
        if(size>(INITIAL_BLOCK_SIZE- sizeof(MallocMetadata))){
            keep = (MallocMetadata*)mmap(NULL, size+ sizeof(MallocMetadata),
                                         PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE,-1, 0);
            keep->set_cookie();
            keep->set_is_free(false);
            keep->set_size(size);

            ++block_list->num_allocated_blocks;
            block_list->allocated_bytes += size;

            return (keep+1);
        }
        else {
            perror("BLOCK NOT FOUND");
        }
    }
    return nullptr;
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

    if(!block->get_is_free()) {

        if(block->get_size()<=(INITIAL_BLOCK_SIZE- sizeof(MallocMetadata))) { // allocated with sbrk
            //Free Buddies
            while (size_to_ord(block->get_size()) <= NUM_ORDERS - 1) {
                block_list->free_block_manager->insert(block);
                MallocMetadata *buddy = (MallocMetadata *) (((intptr_t) block) ^
                                                            (block->get_size() + sizeof(MallocMetadata)));
                if (!buddy->get_is_free()) {
                    break;
                }

                //Buddy is also free
                if (NUM_ORDERS - 1 != size_to_ord(block->get_size())) {
                    block_list->free_block_manager->remove(block);
                    block_list->free_block_manager->remove(buddy);
                    if((intptr_t)block > (intptr_t)buddy){
                        block = buddy;
                    }
                    block->set_size(block->get_size() * 2 + sizeof(MallocMetadata));

                    --block_list->num_allocated_blocks;
                    block_list->allocated_bytes += sizeof(MallocMetadata);
                }
                else{
                    break;
                }
            }
        }
        else{ // used mmap
            size_t size_tmp = block->get_size();
            munmap((void*)block, size_tmp + sizeof(MallocMetadata));
            --block_list->num_allocated_blocks;
            block_list->allocated_bytes -= size_tmp;
            return;
        }
    }
}

void* srealloc(void* oldp, size_t size) {
    if(oldp == NULL){
        return smalloc(size);
    }

    MallocMetadata *block = (MallocMetadata*)(oldp);
    block -= 1;

    if(block->get_is_free()){
        return smalloc(size);
    }

    if(block->get_size()>(INITIAL_BLOCK_SIZE- sizeof(MallocMetadata))){//mmaped block
        if(size==block->get_size())
            return oldp;

        void* allocated_block = smalloc(size);

        size_t min_size = size >  block->get_size() ? block->get_size() : size;
        memmove(allocated_block, oldp, min_size);

        sfree(oldp);

        return allocated_block;
    }
    else{//not an mmaped block
        //option a
        if(block->get_size() >= size){
            if(block->get_is_free()){
                block->set_is_free(false);
                --block_list->num_free_blocks;
                block_list->freed_bytes -= block->get_size();
            }

            return oldp;
        }

        //option b
        //check
        MallocMetadata *tmp = block;
        bool found = false;
        size_t curr_size = tmp->get_size();
        while (size_to_ord(curr_size) <= NUM_ORDERS - 1) {
            if(curr_size>=size){
                found = true;
                break;
            }

            MallocMetadata *buddy = (MallocMetadata *) (((intptr_t) tmp) ^
                                                        (curr_size + sizeof(MallocMetadata)));
            if (!buddy->get_is_free()) {
                break;
            }

            //Buddy is also free
            if (NUM_ORDERS - 1 != size_to_ord(curr_size)) {
                if((intptr_t)tmp > (intptr_t)buddy){
                    tmp = buddy;
                }
                curr_size = curr_size * 2 + sizeof(MallocMetadata);
            }
            else{
                break;
            }
        }
        if(!found){
            //option c
            void* allocated_block = smalloc(size);
            if(allocated_block == NULL)
                return NULL;
            memmove(allocated_block, oldp, block->get_size());
            sfree(oldp);
            return allocated_block;
        }
        else{//use the found block
            int max_ord = size_to_ord(curr_size);
            MallocMetadata *keep_block = block;

            while (size_to_ord(block->get_size()) <= max_ord) {
                if (max_ord != size_to_ord(block->get_size())) {
                    block_list->free_block_manager->insert(block);
                    MallocMetadata *buddy = (MallocMetadata *) (((intptr_t) block) ^
                            (block->get_size() + sizeof(MallocMetadata)));

                    //Buddy is also free

                    block_list->free_block_manager->remove(block);
                    block_list->free_block_manager->remove(buddy);
                    if ((intptr_t) block > (intptr_t) buddy) {
                        block = buddy;
                    }
                    block->set_size(block->get_size() * 2 + sizeof(MallocMetadata));

                    --block_list->num_allocated_blocks;
                    block_list->allocated_bytes += sizeof(MallocMetadata);
                } else {
                    break;
                }
            }
            block += 1;
            memmove(block, oldp, keep_block->get_size());
            //do not need to free oldp cause its a part of the block now
            return block;
        }
    }
}












