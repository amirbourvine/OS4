//
// Created by amirb on 29/06/2023.
//

#include "header_4.h"

int main(){
    print();
    void* b1 = smalloc((1024*1024));
    print();
    void* b2 = srealloc(b1,1024*1024+1);
    print();
    sfree(b2);
    print();
    return 0;
}

