//
// Created by amirb on 26/06/2023.
//

#include "header_2.h"

int main(){
    void* b1 = smalloc(10);
    print();
    void* b2 = smalloc(20);
    print();
    std::cout << "***********************" << std::endl;
    void* b3 = smalloc(30);
    print();
    std::cout << "***********************" << std::endl;

}

