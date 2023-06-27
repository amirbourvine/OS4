//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    void* b1 = smalloc((1024*128+100));
    print();
    void* b2 = srealloc(b1, 1024*128+200);
    print();
    sfree(b2);
    print();
}
