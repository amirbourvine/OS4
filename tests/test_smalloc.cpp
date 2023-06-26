//
// Created by amirb on 26/06/2023.
//

#include "header_2.h"

int main(){
    std::cout << "HERE" << std::endl;
    void* b1 = smalloc(10);
    std::cout << "HERE1" << std::endl;
    print();
    std::cout << "HERE2" << std::endl;
    void* b2 = smalloc(20);
    print();
    void* b3 = smalloc(30);
    print();
}

