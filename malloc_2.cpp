//
// Created by amirb on 26/06/2023.
//

typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

typedef struct BlocksList {
    MallocMetadata* first;
    unsigned int num_free_blocks;
    unsigned int num_allocated_blocks;
    unsigned int allocated_bytes; //does not include meta-data
    unsigned int freed_bytes; //does not include meta-data
} BlocksList;


