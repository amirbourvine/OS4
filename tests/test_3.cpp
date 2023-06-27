//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    int* b1 = (int*)smalloc((128 * 1024));
    print();
    b1[0] = 1;
    sfree(b1);
    print();
}
