//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){

    void* ptr1 = smalloc(16);  // Allocate 16 bytes

    void* ptr2 = smalloc(32);  // Allocate 32 bytes

//    // Overflow the first allocation
//    char* overflowPtr = reinterpret_cast<char*>(ptr1);
//    for (int i = 0; i < 2000; i++) {
//        overflowPtr[i] = 'A';
//    }


    // Allocate more memory
    void* ptr3 = smalloc(64);  // Allocate 64 bytes


    // Free the allocations
    sfree(ptr1);
    sfree(ptr2);
    sfree(ptr3);

}
