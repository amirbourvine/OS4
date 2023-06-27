//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    void* b1 = smalloc((88));
    print();
    void* b2 = srealloc(b1, 40);
    print();
    sfree(b2);
    print();
}
