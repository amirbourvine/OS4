//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    void* b1 = smalloc(96);
    print();
    sfree(b1);
    print();
}
