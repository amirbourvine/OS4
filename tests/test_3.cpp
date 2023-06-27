//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    void* b1 = smalloc((1024*128+100));
    print();
    srealloc(b1, 1024*128+100);
    print();
    sfree(b1);
    print();
}
