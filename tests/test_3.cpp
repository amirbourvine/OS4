//
// Created by amirb on 26/06/2023.
//
#include "header_2.h"
int main(){
    print();
    void* b1 = smalloc((88));
    std::cout << "HERE1-MAIN\n";
    print();
    sfree(b1);
    print();
}
